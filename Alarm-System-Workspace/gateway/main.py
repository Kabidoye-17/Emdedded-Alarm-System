"""
MQTT-UART Gateway

Handles both command transmission (MQTT → UART) and telemetry reception (UART → MQTT).
"""

import json
import threading
from datetime import datetime, timezone
from mqtt.mqtt_subscriber import MQTTSubscriber
from mqtt.mqtt_publisher import MQTTPublisher
from uart.uart_bridge import UARTBridge
from uart.uart_frame_parser import UARTFrameParser
from config.config import topics, commands, protocol as protocol_config
import time
import serial

class MQTTUARTGateway:
    """Gateway bridging MQTT (cloud) and UART (embedded device) for commands and telemetry"""

    def __init__(self):
        # Single UART instance for both TX and RX
        self.uart = UARTBridge()

        # MQTT subscriber for commands (existing)
        self.mqtt_subscriber = MQTTSubscriber()

        # MQTT publisher for telemetry (new)
        self.mqtt_publisher = MQTTPublisher()

        # Frame parser for incoming UART data (pass serial port for ACK)
        self.frame_parser = UARTFrameParser(self.on_update_frame_received, self.uart.ser)

        # Thread for UART RX
        self.uart_rx_thread = None
        self.running = False

    def on_command_received(self, payload):
        """Handle incoming MQTT commands and forward to UART"""
        try:
            data = json.loads(payload)
            command = data.get(commands.mqtt_command_payload_key, "")

            if command in commands.valid_uart_commands:
                print(f"Command received: {command}")
                self.uart.send(command)
            else:
                print(f"ERROR: Invalid command received: {command}")
        except json.JSONDecodeError as e:
            print(f"ERROR: Failed to parse MQTT payload: {e}")

    def on_update_frame_received(self, data):
        """Handle valid update frame from board"""
        try:
            # Decode pipe-delimited string from board
            # Format: FROM_MOTION|WARN_TYPE|ALARM_STATE
            # Examples: "1|HIGH|WARN" (motion event) or "0||DISARMED" (command event)
            message = data.decode(protocol_config.encoding)
            parts = message.split('|')

            if len(parts) != 3:
                print(f"ERROR: Invalid cloud update format: {message}")
                return

            # Parse fields
            from_motion = int(parts[0])
            warn_type = parts[1] if parts[1] else None  # Empty string -> None for command events
            alarm_state = parts[2]

            # Build update object
            update = {
                "from_motion": from_motion,
                "alarm_state": alarm_state,
                "warn_type": warn_type,
                "timestamp": datetime.now(timezone.utc).isoformat()
            }


            # Publish to MQTT as JSON
            self.mqtt_publisher.publish(topics.update, update)
        except (UnicodeDecodeError, ValueError, IndexError) as e:
            print(f"ERROR: Failed to parse cloud update data: {e}")

    def uart_rx_loop(self):
        """Thread loop with automatic reconnection on disconnect"""
        print("UART RX thread started")
        retry_delay = 1.0  # Initial retry delay in seconds
        max_retry_delay = 5.0  # Cap retry delay at 5 seconds

        while self.running:
            try:
                # Check connection state before attempting read
                if not self.uart.is_connected():
                    print(f"UART disconnected. Reconnecting in {retry_delay:.1f}s...")
                    time.sleep(retry_delay)

                    if self.uart.reconnect():
                        print("✓ UART reconnected successfully")
                        retry_delay = 1.0  # Reset backoff on success
                        # Rebuild parser to drop stale state and bind new serial handle
                        self.frame_parser = UARTFrameParser(self.on_update_frame_received, self.uart.ser)
                        # Flush any buffered junk from device reboot
                        try:
                            self.uart.ser.reset_input_buffer()
                            self.uart.ser.reset_output_buffer()
                        except Exception:
                            pass
                    else:
                        # Exponential backoff (double delay up to max)
                        retry_delay = min(retry_delay * 2, max_retry_delay)
                    continue

                # Normal read operation
                if self.uart.ser.in_waiting > 0:
                    byte = self.uart.ser.read(1)[0]
                    self.frame_parser.process_byte(byte)
                    retry_delay = 1.0  # Reset backoff on successful read
                else:
                    # Small sleep to prevent busy-waiting
                    time.sleep(0.001)

            except (serial.SerialException, OSError, PermissionError) as e:
                # Port became invalid (ClearCommError, device unplugged, etc.)
                print(f"✗ UART error (will reconnect): {type(e).__name__}")
                self.uart.disconnect()
                # Will attempt reconnection on next iteration

            except Exception as e:
                # Unexpected error - log but don't assume disconnect
                print(f"⚠ Unexpected error in UART RX loop: {e}")
                time.sleep(0.1)

    def start(self):
        """Start the bidirectional bridge"""
        try:
            # Connect MQTT publisher
            self.mqtt_publisher.connect()

            # Subscribe to command topic
            self.mqtt_subscriber.subscribe(topics.command, self.on_command_received)

            # Start UART RX thread
            self.running = True
            self.uart_rx_thread = threading.Thread(target=self.uart_rx_loop, daemon=True)
            self.uart_rx_thread.start()

            # Start MQTT subscriber (blocking)
            print("MQTT-UART Gateway running...")
            self.mqtt_subscriber.start()
        finally:
            # Cleanup
            self.running = False
            if self.uart_rx_thread:
                self.uart_rx_thread.join(timeout=1.0)
            self.uart.close()
            self.mqtt_publisher.disconnect()

def main():
    gateway = MQTTUARTGateway()
    gateway.start()

if __name__ == "__main__":
    main()

import json
from mqtt_subscriber import MQTTSubscriber
from uart_bridge import UARTBridge
from config.config import topics, commands

class MQTTToUARTBridge:
    """Bridge that forwards MQTT messages to UART"""

    def __init__(self):
        self.uart = UARTBridge()
        self.subscriber = MQTTSubscriber()

    def on_command_received(self, payload):
        """Handle incoming MQTT messages and forward to UART"""
        try:
            # Parse JSON payload
            data = json.loads(payload)
            command = data.get(commands.mqtt_command_payload_key, "")

            # Map cloud command to UART command
            if command in commands.valid_uart_commands:
                print(f"Command received: {command}")
                self.uart.send(command)
            else:
                print(f"ERROR: Invalid command received: {command}")
        except json.JSONDecodeError as e:
            print(f"ERROR: Failed to parse MQTT payload: {e}")

    def start(self):
        """Start the MQTT-UART bridge"""
        try:
            # Subscribe to command topic
            self.subscriber.subscribe(topics.command, self.on_command_received)

            # Start the MQTT client (blocking)
            self.subscriber.start()
        finally:
            # Cleanup
            self.uart.close()

def main():
    bridge = MQTTToUARTBridge()
    bridge.start()

if __name__ == "__main__":
    main()
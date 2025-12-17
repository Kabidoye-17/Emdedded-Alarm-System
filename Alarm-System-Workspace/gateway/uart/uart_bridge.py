import serial
import time
from config.config import uart as uart_config
from uart.port_detector import MAX32655PortDetector
from uart.uart_frame_builder import FrameBuilder


class UARTBridge:
    """Bridge for sending data over UART using STX/ETX binary framing protocol"""

    def __init__(self):
        # Auto-detect port (always try auto-detection first)
        detected_port = MAX32655PortDetector.detect()
        if detected_port:
            self.port = detected_port
        else:
            # No port found and no fallback configured
            print("✗ No board detected and no port configured in config.json")
            self.port = None

        self.baudrate = uart_config.baudrate
        self.ser = None
        self.connected = False
        self._connect()

    def _connect(self):
        """Connect to serial port with error handling"""
        try:
            # Close any existing connection first
            if self.ser and self.ser.is_open:
                self.ser.close()

            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            time.sleep(0.1)
            self.connected = True
            print(f"✓ UART connected on {self.port} at {self.baudrate} baud")
            return True
        except (serial.SerialException, OSError) as e:
            print(f"✗ Failed to connect to {self.port}: {e}")
            self.connected = False
            self.ser = None
            return False

    def reconnect(self):
        """Attempt to reconnect to serial port

        Re-detects the port in case the board was plugged into a different USB port.

        Returns:
            bool: True if reconnection successful, False otherwise
        """
        # Re-detect port in case it changed
        detected_port = MAX32655PortDetector.detect()
        if detected_port:
            self.port = detected_port

        return self._connect()

    def is_connected(self):
        """Check if serial port is connected and valid

        Returns:
            bool: True if connected and port is open
        """
        return self.connected and self.ser and self.ser.is_open

    def disconnect(self):
        """Mark connection as disconnected and close port"""
        self.connected = False
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except:
                pass  # Ignore errors during disconnect

    def send(self, command):
        """Send a command over UART using binary protocol

        Args:
            command: String command ("ARM", "DISARM", or "RESOLVE")

        Returns:
            bool: True if send successful, False otherwise
        """
        try:
            if not self.is_connected():
                print("✗ Cannot send: UART not connected")
                return False

            frame = FrameBuilder.build_frame(command)
            self.ser.write(frame)
            print(f"✓ Sent to UART: {command} (frame: {frame.hex()})")
            return True

        except (serial.SerialException, OSError) as e:
            print(f"✗ Send failed: {e}")
            self.disconnect()
            return False

    def close(self):
        """Close UART connection"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("UART port closed")

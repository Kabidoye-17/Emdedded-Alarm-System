import serial
import time
from config.config import uart as uart_config

class UARTBridge:
    """Bridge for sending data over UART"""

    def __init__(self):
        self.port = uart_config.port
        self.baudrate = uart_config.baudrate
        self.ser = None
        self._connect()

    def _connect(self):
        """Initialize UART connection"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            time.sleep(0.1)
            print(f"UART initialized on {self.port} at {self.baudrate} baud")
        except serial.SerialException as e:
            print(f"ERROR: Failed to open UART port {self.port}: {e}")
            raise

    def send(self, message):
        """
        Send a message over UART

        Args:
            message: String message to send
        """
        if self.ser and self.ser.is_open:
            self.ser.write(message.encode('utf-8'))
            print(f"Sent to UART: {message}")
        else:
            print("ERROR: UART port is not open")

    def close(self):
        """Close UART connection"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("UART port closed")

    def __enter__(self):
        """Context manager entry"""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.close()

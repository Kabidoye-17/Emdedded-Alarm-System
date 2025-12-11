import serial
import time
from config.config import uart as uart_config, protocol as protocol_config
from crc16 import CRC16

class UARTBridge:
    """Bridge for sending data over UART using STX/ETX binary framing protocol"""

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

    def build_frame(self, command):
        """
        Build a binary frame with STX/ETX framing and CRC-16.

        Frame format: [STX][length][data][crc_low][crc_high][ETX]

        Args:
            command: String command ("ARM", "DISARM", or "RESOLVE")

        Returns:
            bytes object containing complete frame
        """
        # Convert command to bytes
        data = command.encode(protocol_config.encoding)
        length = len(data)

        # Build CRC payload: length + data
        crc_payload = bytes([length]) + data

        # Calculate CRC using CRC16 module
        crc = CRC16.calculate(crc_payload)
        crc_low = crc & 0xFF
        crc_high = (crc >> 8) & 0xFF

        # Build complete frame using protocol config
        frame = bytes([protocol_config.stx, length]) + data + bytes([crc_low, crc_high, protocol_config.etx])

        return frame

    def send(self, command):
        """
        Send a command over UART using binary protocol.

        Args:
            command: String command ("ARM", "DISARM", or "RESOLVE")
        """
        if self.ser and self.ser.is_open:
            frame = self.build_frame(command)
            self.ser.write(frame)
            print(f"Sent to UART: {command} (frame: {frame.hex()})")
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

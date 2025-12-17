"""
UART Frame Builder Module

Handles construction of binary frames with STX/ETX framing and CRC-16 checksums.
"""

from config.config import protocol as protocol_config
from crc16 import CRC16


class FrameBuilder:
    """Builds binary frames for UART transmission using STX/ETX protocol"""

    @staticmethod
    def build_frame(command):
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

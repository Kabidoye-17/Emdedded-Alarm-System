"""
UART Frame Parser

State machine parser for incoming UART frames with STX/ETX framing.
Frame format: [STX][length][data...][crc_low][crc_high][ETX]
"""

import serial
from crc16 import CRC16
from config.config import protocol as protocol_config

class UARTFrameParser:
    """
    State machine parser for incoming UART frames with STX/ETX framing.

    Frame format: [STX][length][data...][crc_low][crc_high][ETX]
    """

    # Parser states
    STATE_WAIT_STX = 0
    STATE_READ_LENGTH = 1
    STATE_READ_DATA = 2
    STATE_READ_CRC_LOW = 3
    STATE_READ_CRC_HIGH = 4
    STATE_WAIT_ETX = 5

    def __init__(self, on_frame_received, serial_port=None):
        """
        Initialize parser.

        Args:
            on_frame_received: Callback function(data: bytes) called when valid frame received
            serial_port: Serial port object for sending ACK (optional)
        """
        self.on_frame_received = on_frame_received
        self.serial_port = serial_port
        self.state = self.STATE_WAIT_STX
        self.data_buffer = bytearray()
        self.data_length = 0
        self.data_index = 0
        self.received_crc = 0

    def process_byte(self, byte):
        """Process single byte from UART"""
        if self.state == self.STATE_WAIT_STX:
            if byte == protocol_config.stx:
                self.state = self.STATE_READ_LENGTH
                self.data_buffer = bytearray() # initialize buffer to store incoming data

        elif self.state == self.STATE_READ_LENGTH:
            self.data_length = byte
            self.data_index = 0
            # length must be greater than 0 and within max limit
            if self.data_index < self.data_length <= protocol_config.max_data_length:
                self.state = self.STATE_READ_DATA
            else:
                # Invalid length
                # Reset to wait for next frame
                self.state = self.STATE_WAIT_STX

        elif self.state == self.STATE_READ_DATA:
            self.data_buffer.append(byte)
            self.data_index += 1
            if self.data_index >= self.data_length:
                self.state = self.STATE_READ_CRC_LOW

        elif self.state == self.STATE_READ_CRC_LOW:
            self.received_crc = byte  # Low byte
            self.state = self.STATE_READ_CRC_HIGH

        elif self.state == self.STATE_READ_CRC_HIGH:
            # Combine high byte with low byte: (high_byte << 8) | low_byte
            self.received_crc |= (byte << 8) 
            self.state = self.STATE_WAIT_ETX

        elif self.state == self.STATE_WAIT_ETX:
            if byte == protocol_config.etx:
                # Validate CRC
                crc_payload = bytes([self.data_length]) + bytes(self.data_buffer)
                calculated_crc = CRC16.calculate(crc_payload)

                if calculated_crc == self.received_crc:
                    # Valid frame - invoke callback
                    self.on_frame_received(bytes(self.data_buffer))

                    # Send ACK byte back to board
                    self.send_ack()

            # Always reset to wait for next frame
            self.state = self.STATE_WAIT_STX

    def send_ack(self):
        """Send ACK byte back to board"""
        if self.serial_port:
            try:
                self.serial_port.write(bytes([protocol_config.ack]))
            except (serial.SerialException, OSError):
                # Port disconnected - ACK will fail silently
                # Reconnection will be handled by RX loop
                pass
            except Exception as e:
                print(f"Failed to send ACK: {e}")

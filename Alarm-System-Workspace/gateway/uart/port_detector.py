"""
Port Detector Module

Handles automatic detection of MAX32655 development board via USB VID/PID.
"""

import serial.tools.list_ports
from typing import Optional


class MAX32655PortDetector:
    """Detects MAX32655 board via USB VID/PID"""

    # MAX32655 USB VID/PID (Vendor ID / Product ID)
    # ARM DAPLink VID: 0x0D28 (common for ARM-based development boards)
    # CMSIS-DAP PID: 0x0204 (standard debug interface)
    VID = 0x0D28
    PID = 0x0204

    @classmethod
    def detect(cls) -> Optional[str]:
        """Auto-detect MAX32655 board COM port

        Searches for MAX32655 board by USB VID/PID. Falls back to None if not found.

        Returns:
            str: COM port device name (e.g., 'COM12') or None if not found
        """
        print("Searching for MAX32655 board...")

        for port in serial.tools.list_ports.comports():
            # Check if this port matches the MAX32655 VID/PID
            if port.vid == cls.VID and port.pid == cls.PID:
                print(f"✓ Found MAX32655 on {port.device}")
                return port.device

        print("⚠ MAX32655 board not found via auto-detect")
        return None

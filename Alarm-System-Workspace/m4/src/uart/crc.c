#include "crc.h"

/**
 * @brief Update CRC-16 checksum by processing one byte
 *
 * This function implements table-driven CRC-16 calculation. It updates an
 * existing CRC value by incorporating a new data byte. The CRC is computed
 * incrementally - call this function once per byte in sequence.
 *
 * How it works:
 * 1. XOR the new byte with the low 8 bits of the current CRC
 * 2. Use the result as an index into the precomputed lookup table
 * 3. XOR the table value with the high 8 bits of the current CRC
 *
 * This table-driven approach is ~8x faster than bit-by-bit calculation,
 * making it suitable for use in interrupt handlers.
 *
 * @param crc Current CRC value (use CRCINIT for first byte)
 * @param byte Next data byte to include in checksum
 * @return Updated CRC value (feed this back for next byte)
 *
 * @note Call sequence for a 3-byte message [0x01, 0x02, 0x03]:
 *       uint16_t crc = CRCINIT;
 *       crc = crc_iterate(crc, 0x01);
 *       crc = crc_iterate(crc, 0x02);
 *       crc = crc_iterate(crc, 0x03);
 *       // crc now contains the final checksum
 */
uint16_t crc_iterate(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ crc_fcstab[(crc ^ byte) & 0xff];
}

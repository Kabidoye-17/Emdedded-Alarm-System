#ifndef ADXL343_H
#define ADXL343_H

#include <stdint.h>
#include <stdbool.h>
#include "mxc_device.h"
#include "spi.h"

#define SPI_SPEED 1000000

#define SPI MXC_SPI1

// ADXL343 register map (see adxl343 datasheet)
#define ADXL343_REG_DEVID 0x00
#define ADXL343_REG_BW_RATE 0x2C
#define ADXL343_REG_POWER_CTL 0x2D
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0 0x32

#define ADXL343_DEVID_VALUE 0xE5
#define ADXL343_ODR_100_HZ 0x0A
#define ADXL343_POWER_MEASURE (1 << 3)
#define ADXL343_DATA_FULL_RES (1 << 3)
#define ADXL343_DATA_RANGE_2G 0x00

#define ADXL343_SPI_READ 0x80
#define ADXL343_SPI_MB 0x40
#define ADXL343_SPI_SS_INDEX 1 // Use SS1 on the Feather header by default
#define ADXL343_SPI_MAX_TRANSFER 8 // Command byte + 6 data bytes, rounded up

int adxl343_spi_init(const mxc_spi_pins_t *pins);
int adxl343_probe(void);
int adxl343_init(void);
int adxl343_read_xyz(int16_t *x, int16_t *y, int16_t *z);
void adxl343_set_ss(int ss);

int adxl343_write_reg(uint8_t reg, uint8_t value);
int adxl343_read_regs(uint8_t start_reg, uint8_t *values, uint32_t len);


#endif // ADXL343_H

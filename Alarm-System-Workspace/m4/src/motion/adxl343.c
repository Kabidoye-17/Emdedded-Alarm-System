#include "adxl343.h"
#include "mxc_delay.h"
#include <string.h>

/*
 * ============================================================================
 * ADXL343 SPI Driver (Implementation)
 * ============================================================================
 * This file contains the SPI communication logic and high-level helper
 * functions for configuring and reading data from the ADXL343 accelerometer.
 * It is written to work with the MAX32655 SPI peripheral.
 */


/***** SPI config *****/
static int g_adxl343_ss = 1;


/***** SPI transfer helper *****/
/*
 * Performs a single blocking SPI transaction.
 * This function is the lowest-level SPI routine used by the driver.
 *
 * tx   -> buffer containing bytes to transmit
 * rx   -> buffer where received bytes will be stored
 * len  -> number of bytes to transfer
 */
static int adxl343_spi_xfer(uint8_t *tx, uint8_t *rx, uint32_t len)
{
    // SPI transaction request structure
    mxc_spi_req_t req = {0};

    // Select SPI peripheral (SPI1)
    req.spi = SPI;

    // Transmit and receive buffers
    req.txData = tx;
    req.rxData = rx;

    // Number of bytes to transmit and receive
    req.txLen = len;
    req.rxLen = len;

    // Which slave-select line to use
    req.ssIdx = g_adxl343_ss;

    // Release slave-select line after transfer completes
    req.ssDeassert = 1;

    // Transfer counters (managed by driver)
    req.txCnt = 0;
    req.rxCnt = 0;

    // Blocking transfer, no callback function
    req.completeCB = NULL;

    // Execute SPI transaction
    return MXC_SPI_MasterTransaction(&req);
}


/***** SPI initialization *****/
/*
 * Initializes the SPI peripheral and configures it for the ADXL343.
 * This must be called before any sensor communication.
 *
 * pins -> structure defining which SPI pins are enabled
 */
int adxl343_spi_init(const mxc_spi_pins_t *pins)
{
    // Count how many slave-select pins are enabled
    uint32_t num_slaves = pins->ss0 + pins->ss1 + pins->ss2;

    // At least one SS line must be enabled
    if (num_slaves == 0) {
        return E_BAD_PARAM;
    }

    // Initialize SPI in master mode
    int ret = MXC_SPI_Init(SPI, 1, 0, num_slaves, 0, SPI_SPEED, *pins);
    if (ret != E_NO_ERROR) return ret;

    // Configure SPI for 8-bit data frames
    ret = MXC_SPI_SetDataSize(SPI, 8);
    if (ret != E_NO_ERROR) return ret;

    // Standard single-wire SPI
    ret = MXC_SPI_SetWidth(SPI, SPI_WIDTH_STANDARD);
    if (ret != E_NO_ERROR) return ret;

    // SPI Mode 3: CPOL = 1, CPHA = 1 (required by ADXL343)
    return MXC_SPI_SetMode(SPI, SPI_MODE_3);
}


/***** Register write (multi-byte) *****/
/*
 * Writes one or more consecutive registers starting at start_reg.
 *
 * start_reg -> first register address
 * values    -> pointer to data bytes to write
 * len       -> number of registers to write
 */
static int adxl343_write_regs(uint8_t start_reg,
                              const uint8_t *values,
                              uint32_t len)
{
    // Total bytes = command byte + data bytes
    uint32_t total = len + 1;

    // Prevent buffer overflow
    if (total > ADXL343_SPI_MAX_TRANSFER) {
        return E_BAD_PARAM;
    }

    // Temporary SPI buffers
    uint8_t tx[ADXL343_SPI_MAX_TRANSFER] = {0};
    uint8_t rx[ADXL343_SPI_MAX_TRANSFER] = {0};

    // Build SPI command byte
    // Multi-byte bit is set only if writing more than one register
    tx[0] = start_reg | ((len > 1) ? ADXL343_SPI_MB : 0);

    // Copy register values into transmit buffer
    memcpy(&tx[1], values, len);

    // Send SPI transaction
    return adxl343_spi_xfer(tx, rx, total);
}

int adxl343_write_reg(uint8_t reg, uint8_t value)
{
    return adxl343_write_regs(reg, &value, 1);
}

int adxl343_read_regs(uint8_t start_reg, uint8_t *values, uint32_t len)
{
    // 1 command byte + requested data bytes
    uint32_t total = len + 1;

    // Prevent oversized SPI transfers
    if (total > ADXL343_SPI_MAX_TRANSFER)
        return E_BAD_PARAM;

    // SPI transmit and receive buffers
    uint8_t tx[ADXL343_SPI_MAX_TRANSFER] = {0};
    uint8_t rx[ADXL343_SPI_MAX_TRANSFER] = {0};

    // Build read command (address + read bit + optional multi-byte bit)
    tx[0] = start_reg | ADXL343_SPI_READ |
            ((len > 1) ? ADXL343_SPI_MB : 0);

    // Perform blocking SPI transaction
    int ret = adxl343_spi_xfer(tx, rx, total);

    // Copy received register data (skip command byte)
    if (ret == E_NO_ERROR)
        memcpy(values, &rx[1], len);

    return ret;
}


/***** Sensor probe *****/
/*
 * Confirms that the connected device is an ADXL343 by checking
 * the device ID register.
 */
int adxl343_probe(void)
{
    uint8_t devid = 0;

    // Read device ID register
    int ret = adxl343_read_regs(ADXL343_REG_DEVID, &devid, 1);
    if (ret != E_NO_ERROR) {
        return ret;
    }

    // Verify device ID matches expected value
    return (devid == ADXL343_DEVID_VALUE) ?
           E_NO_ERROR : E_BAD_STATE;
}


/***** Set slave select *****/
void adxl343_set_ss(int ss) {
    g_adxl343_ss = ss;
}


/***** Sensor initialization *****/
/*
 * Fully initializes the ADXL343 sensor:
 *  - Verifies device presence
 *  - Configures data format
 *  - Sets output data rate
 *  - Enables measurement mode
 */
int adxl343_init(void)
{
    int ret;

    // Confirm sensor is present
    ret = adxl343_probe();
    if (ret != E_NO_ERROR) {
        return ret;
    }

    // Configure full-resolution mode with ±2g range
    uint8_t data_format =
        ADXL343_DATA_FULL_RES | ADXL343_DATA_RANGE_2G;

    ret = adxl343_write_reg(ADXL343_REG_DATA_FORMAT, data_format);
    if (ret != E_NO_ERROR) return ret;

    // Set output data rate to 100 Hz
    ret = adxl343_write_reg(ADXL343_REG_BW_RATE, ADXL343_ODR_100_HZ);
    if (ret != E_NO_ERROR) return ret;

    // Enable measurement mode
    return adxl343_write_reg(ADXL343_REG_POWER_CTL,
                             ADXL343_POWER_MEASURE);
}



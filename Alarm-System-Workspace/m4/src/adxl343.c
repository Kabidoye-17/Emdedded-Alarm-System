#include "adxl343.h"
#include "mxc_delay.h"
#include <string.h>


/***** SPI config *****/
static int g_adxl343_ss = 1;

/***** SPI transfer helper *****/
static int adxl343_spi_xfer(uint8_t *tx, uint8_t *rx, uint32_t len)
{
    mxc_spi_req_t req = {0};
    req.spi = SPI;
    req.txData = tx;
    req.rxData = rx;
    req.txLen = len;
    req.rxLen = len;
    req.ssIdx = g_adxl343_ss;
    req.ssDeassert = 1;
    req.txCnt = req.rxCnt = 0;
    req.completeCB = NULL;
    return MXC_SPI_MasterTransaction(&req);
}

/***** SPI init *****/
int adxl343_spi_init(const mxc_spi_pins_t *pins)
{
    uint32_t num_slaves = pins->ss0 + pins->ss1 + pins->ss2;
    if (num_slaves == 0) return E_BAD_PARAM;

    int ret = MXC_SPI_Init(SPI, 1, 0, num_slaves, 0, SPI_SPEED, *pins);
    if (ret != E_NO_ERROR) return ret;
    if ((ret = MXC_SPI_SetDataSize(SPI, 8)) != E_NO_ERROR) return ret;
    if ((ret = MXC_SPI_SetWidth(SPI, SPI_WIDTH_STANDARD)) != E_NO_ERROR) return ret;
    return MXC_SPI_SetMode(SPI, SPI_MODE_3); // CPOL=1, CPHA=1
}

/***** Read/write registers *****/
static int adxl343_write_regs(uint8_t start_reg, const uint8_t *values, uint32_t len)
{
    uint32_t total = len + 1;
    if (total > ADXL343_SPI_MAX_TRANSFER) return E_BAD_PARAM;

    uint8_t tx[ADXL343_SPI_MAX_TRANSFER] = {0};
    uint8_t rx[ADXL343_SPI_MAX_TRANSFER] = {0};
    tx[0] = start_reg | ((len > 1) ? ADXL343_SPI_MB : 0);
    memcpy(&tx[1], values, len);
    return adxl343_spi_xfer(tx, rx, total);
}

int adxl343_write_reg(uint8_t reg, uint8_t value)
{
    return adxl343_write_regs(reg, &value, 1);
}

int adxl343_read_regs(uint8_t start_reg, uint8_t *values, uint32_t len)
{
    uint32_t total = len + 1;
    if (total > ADXL343_SPI_MAX_TRANSFER) return E_BAD_PARAM;

    uint8_t tx[ADXL343_SPI_MAX_TRANSFER] = {0};
    uint8_t rx[ADXL343_SPI_MAX_TRANSFER] = {0};
    tx[0] = start_reg | ADXL343_SPI_READ | ((len > 1) ? ADXL343_SPI_MB : 0);

    int ret = adxl343_spi_xfer(tx, rx, total);
    if (ret == E_NO_ERROR) memcpy(values, &rx[1], len);
    return ret;
}

/***** Probe sensor *****/
int adxl343_probe(void)
{
    uint8_t devid = 0;
    int ret = adxl343_read_regs(ADXL343_REG_DEVID, &devid, 1);
    if (ret != E_NO_ERROR) return ret;
    return (devid == ADXL343_DEVID_VALUE) ? E_NO_ERROR : E_BAD_STATE;
}

/***** Set slave select *****/
void adxl343_set_ss(int ss) {
    g_adxl343_ss = ss;
}

/***** Init sensor *****/
int adxl343_init(void)
{
    int ret = adxl343_probe();
    if (ret != E_NO_ERROR) return ret;

    uint8_t data_format = ADXL343_DATA_FULL_RES | ADXL343_DATA_RANGE_2G;
    if ((ret = adxl343_write_reg(ADXL343_REG_DATA_FORMAT, data_format)) != E_NO_ERROR) return ret;
    if ((ret = adxl343_write_reg(ADXL343_REG_BW_RATE, ADXL343_ODR_100_HZ)) != E_NO_ERROR) return ret;
    return adxl343_write_reg(ADXL343_REG_POWER_CTL, ADXL343_POWER_MEASURE);
}

/***** Read XYZ *****/
// this fuction reads the X, Y, Z acceleration values from the ADXL343, was used to configure
// motion detection thresholds programmatically, no longer needed but kept for reference
int adxl343_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t raw[6] = {0};
    int ret = adxl343_read_regs(ADXL343_REG_DATAX0, raw, 6);
    if (ret != E_NO_ERROR) return ret;

    *x = (int16_t)((raw[1] << 8) | raw[0]);
    *y = (int16_t)((raw[3] << 8) | raw[2]);
    *z = (int16_t)((raw[5] << 8) | raw[4]);
    return E_NO_ERROR;
}


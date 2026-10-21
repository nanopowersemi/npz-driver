/**
 * @file npz.c
 *
 * @brief Source file for the nPZero Driver.
 *
 * This file contains the implementation of the nPZero driver for controlling and managing
 * the nPZero G1S power-saving IC.
 * It provides functions to interact with the nPZero G1S IC on register level,
 * enabling extensive configuration and features, such as periodic power switching of peripherals for value comparison.
 *
 * This driver is designed to work with the nPZero G1S IC from Nanopower Semiconductor.
 * It utilizes the I2C communication protocol for interfacing with the host MCU.
 *
 * @note This driver assumes proper initialization of the hardware abstraction layer for I2C is present in the npz_hal.c file.
 *
 * @see npz.h for the corresponding header file providing function prototypes and definitions.
 */

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "../Inc/npz.h"
#include "../Inc/npz_registers.h"

/*****************************************************************************
 * Data
 *****************************************************************************/

/* Per-peripheral register addresses, indexed by (npz_psw_e - PSW_LP1). */
static const uint8_t m_reg_cfgp[4]     = {REG_CFGP1, REG_CFGP2, REG_CFGP3, REG_CFGP4};
static const uint8_t m_reg_modp[4]     = {REG_MODP1, REG_MODP2, REG_MODP3, REG_MODP4};
static const uint8_t m_reg_perp_l[4]   = {REG_PERP1_L, REG_PERP2_L, REG_PERP3_L, REG_PERP4_L};
static const uint8_t m_reg_perp_h[4]   = {REG_PERP1_H, REG_PERP2_H, REG_PERP3_H, REG_PERP4_H};
static const uint8_t m_reg_ncmdp[4]    = {REG_NCMDP1, REG_NCMDP2, REG_NCMDP3, REG_NCMDP4};
static const uint8_t m_reg_addrp[4]    = {REG_ADDRP1, REG_ADDRP2, REG_ADDRP3, REG_ADDRP4};
static const uint8_t m_reg_rregp[4]    = {REG_RREGP1, REG_RREGP2, REG_RREGP3, REG_RREGP4};
static const uint8_t m_reg_throvp_l[4] = {REG_THROVP1_L, REG_THROVP2_L, REG_THROVP3_L, REG_THROVP4_L};
static const uint8_t m_reg_throvp_h[4] = {REG_THROVP1_H, REG_THROVP2_H, REG_THROVP3_H, REG_THROVP4_H};
static const uint8_t m_reg_thrunp_l[4] = {REG_THRUNP1_L, REG_THRUNP2_L, REG_THRUNP3_L, REG_THRUNP4_L};
static const uint8_t m_reg_thrunp_h[4] = {REG_THRUNP1_H, REG_THRUNP2_H, REG_THRUNP3_H, REG_THRUNP4_H};
static const uint8_t m_reg_twtp[4]     = {REG_TWTP1, REG_TWTP2, REG_TWTP3, REG_TWTP4};
static const uint8_t m_reg_tcfgp[4]    = {REG_TCFGP1, REG_TCFGP2, REG_TCFGP3, REG_TCFGP4};
static const uint8_t m_reg_valp_l[4]   = {REG_VALP1_L, REG_VALP2_L, REG_VALP3_L, REG_VALP4_L};
static const uint8_t m_reg_valp_h[4]   = {REG_VALP1_H, REG_VALP2_H, REG_VALP3_H, REG_VALP4_H};

/*****************************************************************************
 * Private Methods
 *****************************************************************************/

/**
 * @brief Converts a low power switch id to an array index.
 *
 * Every per-peripheral function must go through this check. Without it, an invalid switch value would leave the
 * register address at 0x00 (IDLE_RST), and a write could put the device into idle (0xFF) or soft reset it (0xA5).
 */
static bool psw_to_index(const npz_psw_e sw, uint8_t *index)
{
    switch (sw)
    {
        case PSW_LP1:
        case PSW_LP2:
        case PSW_LP3:
        case PSW_LP4:
            *index = (uint8_t)(sw - PSW_LP1);
            return true;
        default:
            return false;
    }
}

static npz_status_e write_reg(const uint8_t reg, const uint8_t value)
{
    uint8_t transmitData[2] = {reg, value};

    return npz_hal_write(NPZ_I2C_ADDRESS, transmitData, sizeof(transmitData), I2C_TRANSMISSION_TIMEOUT_MS);
}

static npz_status_e read_reg(const uint8_t reg, void *data)
{
    if (data == NULL)
    {
        return INVALID_PARAM;
    }

    return npz_hal_read(NPZ_I2C_ADDRESS, reg, (uint8_t *) data, 1, I2C_TRANSMISSION_TIMEOUT_MS);
}

static npz_status_e write_reg16(const uint8_t reg_l, const uint8_t reg_h, const uint8_t value_l, const uint8_t value_h)
{
    if (write_reg(reg_l, value_l) != OK)
    {
        return ERR;
    }

    return write_reg(reg_h, value_h);
}

static npz_status_e read_reg16(const uint8_t reg_l, const uint8_t reg_h, uint8_t *value_l, uint8_t *value_h)
{
    npz_status_e success;

    if (value_l == NULL || value_h == NULL)
    {
        return INVALID_PARAM;
    }

    success = read_reg(reg_l, value_l);

    if (success == OK)
    {
        success = read_reg(reg_h, value_h);
    }

    return success;
}

/* Per-peripheral 8-bit register write/read through the switch-to-index lookup. */
static npz_status_e write_preg(const npz_psw_e sw, const uint8_t regs[4], const uint8_t value)
{
    uint8_t i;

    if (!psw_to_index(sw, &i))
    {
        return INVALID_PARAM;
    }

    return write_reg(regs[i], value);
}

static npz_status_e read_preg(const npz_psw_e sw, const uint8_t regs[4], void *data)
{
    uint8_t i;

    if (!psw_to_index(sw, &i))
    {
        return INVALID_PARAM;
    }

    return read_reg(regs[i], data);
}

/*****************************************************************************
 * Public Methods
 *****************************************************************************/

npz_status_e npz_write_IDLE_RST(uint8_t idle_rst_value)
{
    return write_reg(REG_IDLE_RST, idle_rst_value);
}

npz_status_e npz_read_IDLE_RST(uint8_t *idle_rst_value)
{
    return read_reg(REG_IDLE_RST, idle_rst_value);
}

npz_status_e npz_read_ID(uint8_t *id)
{
    return read_reg(REG_ID, id);
}

npz_status_e npz_read_STA1(npz_register_sta1_s *sta1)
{
    return read_reg(REG_STA1, sta1);
}

npz_status_e npz_read_STA2(npz_register_sta2_s *sta2)
{
    return read_reg(REG_STA2, sta2);
}

npz_status_e npz_write_PSWCTL(const npz_register_pswctl_s pswctl)
{
    uint8_t value = 0;

    value |= pswctl.pswint_p1;
    value |= pswctl.pswint_p2 << 1;
    value |= pswctl.pswint_p3 << 2;
    value |= pswctl.pswint_p4 << 3;
    value |= pswctl.pswh_mode << 4;
    value |= pswctl.psw_en_vn << 6;
    /* Bit 7 (psw_vn_on) is read only. */

    return write_reg(REG_PSWCTL, value);
}

npz_status_e npz_read_PSWCTL(npz_register_pswctl_s *pswctl)
{
    return read_reg(REG_PSWCTL, pswctl);
}

npz_status_e npz_write_SYSCFG1(const npz_register_syscfg1_s syscfg1)
{
    uint8_t value = 0;

    value |= syscfg1.wup1;
    value |= syscfg1.wup2 << 1;
    value |= syscfg1.wup3 << 2;
    value |= syscfg1.wup4 << 3;
    value |= syscfg1.adc_int_wakeup_enable << 4;
    value |= syscfg1.adc_ext_wakeup_enable << 5;
    value |= syscfg1.wake_up_any_or_all << 6;

    return write_reg(REG_SYSCFG1, value);
}

npz_status_e npz_read_SYSCFG1(npz_register_syscfg1_s *syscfg1)
{
    return read_reg(REG_SYSCFG1, syscfg1);
}

npz_status_e npz_write_SYSCFG2(const npz_register_syscfg2_s syscfg2)
{
    uint8_t value = 0;

    value |= syscfg2.sclk_div_en;
    value |= syscfg2.sclk_div_sel << 1;
    value |= syscfg2.sclk_sel << 3;
    value |= syscfg2.adc_ext_on << 4;
    value |= syscfg2.adc_clk_sel << 5;

    return write_reg(REG_SYSCFG2, value);
}

npz_status_e npz_read_SYSCFG2(npz_register_syscfg2_s *syscfg2)
{
    return read_reg(REG_SYSCFG2, syscfg2);
}

npz_status_e npz_write_SYSCFG3(const npz_register_syscfg3_s syscfg3)
{
    uint8_t value = 0;

    value |= syscfg3.io_str;
    value |= syscfg3.i2c_pup_en << 1;
    value |= syscfg3.i2c_pup_auto << 2;
    value |= syscfg3.spi_auto << 3;
    value |= syscfg3.xo_clkout_div << 4;
    /* Bit 7 (sclk_sel_status) is read only. */

    return write_reg(REG_SYSCFG3, value);
}

npz_status_e npz_read_SYSCFG3(npz_register_syscfg3_s *syscfg3)
{
    return read_reg(REG_SYSCFG3, syscfg3);
}

npz_status_e npz_write_TOUT(const npz_register_tout_s tout)
{
    return write_reg16(REG_TOUT_L, REG_TOUT_H, tout.tout_l, tout.tout_h);
}

npz_status_e npz_read_TOUT(npz_register_tout_s *tout)
{
    if (tout == NULL)
    {
        return INVALID_PARAM;
    }

    return read_reg16(REG_TOUT_L, REG_TOUT_H, &tout->tout_l, &tout->tout_h);
}

npz_status_e npz_write_INTCFG(const npz_register_intcfg_s intcfg)
{
    uint8_t value = 0;

    value |= intcfg.pu_int1;
    value |= intcfg.pu_s_int1 << 1;
    value |= intcfg.pu_int2 << 2;
    value |= intcfg.pu_s_int2 << 3;
    value |= intcfg.pu_int3 << 4;
    value |= intcfg.pu_s_int3 << 5;
    value |= intcfg.pu_int4 << 6;
    value |= intcfg.pu_s_int4 << 7;

    return write_reg(REG_INTCFG, value);
}

npz_status_e npz_read_INTCFG(npz_register_intcfg_s *intcfg)
{
    return read_reg(REG_INTCFG, intcfg);
}

npz_status_e npz_write_THROVA1(const npz_register_throva1_s throva1)
{
    return write_reg(REG_THROVA1, throva1.throva);
}

npz_status_e npz_read_THROVA1(npz_register_throva1_s *throva1)
{
    return read_reg(REG_THROVA1, throva1);
}

npz_status_e npz_write_THROVA2(const npz_register_throva2_s throva2)
{
    return write_reg(REG_THROVA2, throva2.throva);
}

npz_status_e npz_read_THROVA2(npz_register_throva2_s *throva2)
{
    return read_reg(REG_THROVA2, throva2);
}

npz_status_e npz_write_THRUNA1(const npz_register_thruna1_s thruna1)
{
    return write_reg(REG_THRUNA1, thruna1.thruna);
}

npz_status_e npz_read_THRUNA1(npz_register_thruna1_s *thruna1)
{
    return read_reg(REG_THRUNA1, thruna1);
}

npz_status_e npz_write_THRUNA2(const npz_register_thruna2_s thruna2)
{
    return write_reg(REG_THRUNA2, thruna2.thruna);
}

npz_status_e npz_read_THRUNA2(npz_register_thruna2_s *thruna2)
{
    return read_reg(REG_THRUNA2, thruna2);
}

npz_status_e npz_read_ADC_CORE(npz_register_adc_core_s *adc_core)
{
    return read_reg(REG_ADC_CORE, adc_core);
}

npz_status_e npz_read_ADC_EXT(npz_register_adc_ext_s *adc_ext)
{
    return read_reg(REG_ADC_EXT, adc_ext);
}

npz_status_e npz_write_SRAM_block(const uint8_t sram_reg, const uint8_t *data, const uint8_t len)
{
    npz_register_sram_s sram_image;
    uint8_t offset;
    npz_status_e ret;

    if (data == NULL || len == 0 || sram_reg < REG_SRAM_START ||
        (uint16_t) (sram_reg - REG_SRAM_START) + len > SRAM_REG_SIZE)
    {
        return INVALID_PARAM;
    }

    /* Read-modify-write of the complete SRAM image. */
    ret = npz_read_SRAM(REG_SRAM_START, &sram_image);
    if (ret != OK)
    {
        return ret;
    }

    offset = (uint8_t) (sram_reg - REG_SRAM_START);
    for (uint8_t n = 0; n < len; n++)
    {
        sram_image.value[offset + n] = data[n];
    }

    /* Write all 128 bytes back. The write order (k is a permutation of 0..127) is kept exactly as in the original
     * driver; it is assumed to be a device requirement. */
    for (uint8_t i = 0; i < SRAM_REG_SIZE; i++)
    {
        uint8_t k = (uint8_t) ((((~i) & 0x07) << 3) + ((i & 0x38) >> 3) + (i & 0x40));

        ret = write_reg((uint8_t) (REG_SRAM_START + k), sram_image.value[k]);
        if (ret != OK)
        {
            return ret;
        }
    }

    return OK;
}

npz_status_e npz_write_SRAM(const uint8_t sram_reg, const uint8_t sram)
{
    return npz_write_SRAM_block(sram_reg, &sram, 1);
}

npz_status_e npz_read_SRAM(const uint8_t sram_reg, npz_register_sram_s *sram)
{
    if (sram == NULL || sram_reg != REG_SRAM_START)
    {
        return INVALID_PARAM;
    }

    return npz_hal_read(NPZ_I2C_ADDRESS, REG_SRAM_START, sram->value, SRAM_REG_SIZE, I2C_TRANSMISSION_TIMEOUT_MS);
}

npz_status_e npz_write_CFGP(const npz_psw_e sw, const npz_register_cfgp_s cfgp)
{
    uint8_t value = 0;

    value |= cfgp.pwmod;
    value |= cfgp.tmod << 2;
    value |= cfgp.pswmod << 4;
    value |= cfgp.intmod << 6;

    return write_preg(sw, m_reg_cfgp, value);
}

npz_status_e npz_read_CFGP(const npz_psw_e sw, npz_register_cfgp_s *cfgp)
{
    return read_preg(sw, m_reg_cfgp, cfgp);
}

npz_status_e npz_write_MODP(const npz_psw_e sw, const npz_register_modp_s modp)
{
    uint8_t value = 0;

    value |= modp.cmod;
    value |= modp.dtype << 1;
    value |= modp.seqrw << 3;
    value |= modp.wunak << 4;
    value |= modp.swprreg << 5;
    value |= modp.spimod << 6;

    return write_preg(sw, m_reg_modp, value);
}

npz_status_e npz_read_MODP(const npz_psw_e sw, npz_register_modp_s *modp)
{
    return read_preg(sw, m_reg_modp, modp);
}

npz_status_e npz_write_PERP(const npz_psw_e sw, const npz_register_perp_s perp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i) || (perp.perp_l == 0 && perp.perp_h == 0))
    {
        return INVALID_PARAM;
    }

    return write_reg16(m_reg_perp_l[i], m_reg_perp_h[i], perp.perp_l, perp.perp_h);
}

npz_status_e npz_read_PERP(const npz_psw_e sw, npz_register_perp_s *perp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i) || perp == NULL)
    {
        return INVALID_PARAM;
    }

    return read_reg16(m_reg_perp_l[i], m_reg_perp_h[i], &perp->perp_l, &perp->perp_h);
}

npz_status_e npz_write_NCMDP(const npz_psw_e sw, const npz_register_ncmdp_s ncmdp)
{
    return write_preg(sw, m_reg_ncmdp, ncmdp.ncmdp);
}

npz_status_e npz_read_NCMDP(const npz_psw_e sw, npz_register_ncmdp_s *ncmdp)
{
    return read_preg(sw, m_reg_ncmdp, ncmdp);
}

npz_status_e npz_write_ADDRP(const npz_psw_e sw, const npz_register_addrp_s addrp)
{
    uint8_t value = 0;

    value |= addrp.addrp;
    value |= addrp.spi_en << 7;

    return write_preg(sw, m_reg_addrp, value);
}

npz_status_e npz_read_ADDRP(const npz_psw_e sw, npz_register_addrp_s *addrp)
{
    return read_preg(sw, m_reg_addrp, addrp);
}

npz_status_e npz_write_RREGP(const npz_psw_e sw, const npz_register_rregp_s rregp)
{
    return write_preg(sw, m_reg_rregp, rregp.rregp);
}

npz_status_e npz_read_RREGP(const npz_psw_e sw, npz_register_rregp_s *rregp)
{
    return read_preg(sw, m_reg_rregp, rregp);
}

npz_status_e npz_write_THROVP(const npz_psw_e sw, const npz_register_throvp_s throvp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i))
    {
        return INVALID_PARAM;
    }

    return write_reg16(m_reg_throvp_l[i], m_reg_throvp_h[i], throvp.throvp_l, throvp.throvp_h);
}

npz_status_e npz_read_THROVP(const npz_psw_e sw, npz_register_throvp_s *throvp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i) || throvp == NULL)
    {
        return INVALID_PARAM;
    }

    return read_reg16(m_reg_throvp_l[i], m_reg_throvp_h[i], &throvp->throvp_l, &throvp->throvp_h);
}

npz_status_e npz_write_THRUNP(const npz_psw_e sw, const npz_register_thrunp_s thrunp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i))
    {
        return INVALID_PARAM;
    }

    return write_reg16(m_reg_thrunp_l[i], m_reg_thrunp_h[i], thrunp.thrunp_l, thrunp.thrunp_h);
}

npz_status_e npz_read_THRUNP(const npz_psw_e sw, npz_register_thrunp_s *thrunp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i) || thrunp == NULL)
    {
        return INVALID_PARAM;
    }

    return read_reg16(m_reg_thrunp_l[i], m_reg_thrunp_h[i], &thrunp->thrunp_l, &thrunp->thrunp_h);
}

npz_status_e npz_write_TWTP(const npz_psw_e sw, const npz_register_twtp_s twtp)
{
    return write_preg(sw, m_reg_twtp, twtp.twtp);
}

npz_status_e npz_read_TWTP(const npz_psw_e sw, npz_register_twtp_s *twtp)
{
    return read_preg(sw, m_reg_twtp, twtp);
}

npz_status_e npz_write_TCFGP(const npz_psw_e sw, const npz_register_tcfgp_s tcfgp)
{
    uint8_t value = 0;

    value |= tcfgp.twt_en;
    value |= tcfgp.twt_ext << 1;
    value |= tcfgp.tinit_en << 2;
    value |= tcfgp.tinit_ext << 3;
    value |= tcfgp.i2cret << 4;

    return write_preg(sw, m_reg_tcfgp, value);
}

npz_status_e npz_read_TCFGP(const npz_psw_e sw, npz_register_tcfgp_s *tcfgp)
{
    return read_preg(sw, m_reg_tcfgp, tcfgp);
}

npz_status_e npz_read_VALP(const npz_psw_e sw, npz_register_valp_s *valp)
{
    uint8_t i;

    if (!psw_to_index(sw, &i) || valp == NULL)
    {
        return INVALID_PARAM;
    }

    return read_reg16(m_reg_valp_l[i], m_reg_valp_h[i], &valp->valp_l, &valp->valp_h);
}

npz_status_e npz_read_register(uint8_t register_address, void *buffer, size_t size)
{
    if (buffer == NULL || size == 0 || size > UINT16_MAX)
    {
        return INVALID_PARAM;
    }

    return npz_hal_read(NPZ_I2C_ADDRESS, register_address, (uint8_t *) buffer, (uint16_t) size,
        I2C_TRANSMISSION_TIMEOUT_MS);
}

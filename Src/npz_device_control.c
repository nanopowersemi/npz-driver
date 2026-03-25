/**
 * @file npz_device_control.c
 * @brief Implementation of functions to read and handle status registers (STA1 and STA2), put the nPZero
 * device in idle mode, perform a software reset, and set a wake-up timeout.
 *
 * The registers in STA1 and STA2 can be read to determine the wake-up cause.
 */

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include <stdio.h>
#include "../Inc/npz_device_control.h"

/*****************************************************************************
 * Defines
 *****************************************************************************/

#define SRAM_START 0x80
#define SRAM_SIZE  128

/*****************************************************************************
 * Data
 *****************************************************************************/

static struct m_adc_ext_code_level_map
{
    uint8_t code;
    uint16_t millivolts;
}

adc_ext_code_map[] = {{0x00, 600}, {0x08, 700}, {0x0F, 800}, {0x15, 900}, {0x19, 1000},
    {0x1C, 1100}, {0x1F, 1200}, {0x22, 1300}, {0x23, 1400}, {0x24, 1500}, {0x26, 1600},
    {0x28, 1700}, {0x29, 1800}, {0x2A, 1900}, {0x2B, 2000}, {0x2C, 2100}, {0x2D, 2200},
    {0x2E, 2300}, {0x2F, 2400}, {0x30, 3600}, {0x31, 2700}, {0x32, 2900}, {0x33, 3100},
    {0x34, 3400}};

static struct m_adc_core_code_level_map
{
    uint8_t code;
    uint16_t millivolts;
} adc_core_code_map[] = {{0x24, 1500}, {0x26, 1600}, {0x28, 1700}, {0x29, 1800}, {0x2A, 1900},
    {0x2B, 2000}, {0x2C, 2100}, {0x2D, 2200}, {0x2E, 2300}, {0x2F, 2400}, {0x30, 2600},
    {0x31, 2800}, {0x32, 3000}, {0x33, 3200}, {0x34, 3400}};

/** Struct that holds the Internal ADC configuration including threshold values. */
typedef struct
{
    npz_register_throva1_s throva1; /**< Struct Internal ADC (VBAT)Threshold Over Value. */
    npz_register_thruna1_s thruna1; /**< Struct Internal ADC (VBAT)Threshold Under Value. */
} int_adc_channel_config_s;

/** Struct that holds the External ADC configuration including threshold values. */
typedef struct
{
    npz_register_throva2_s throva2; /**< Struct External ADC (ADC_IN) Threshold Over Value. */
    npz_register_thruna2_s thruna2; /**< Struct External ADC (ADC_IN) Threshold Under Value. */
} ext_adc_channel_config_s;

/** Struct that holds all registers that can be configured for each peripheral. */
typedef struct
{
    npz_register_cfgp_s cfgp;   /**< Struct Peripheral Config register. */
    npz_register_modp_s modp;   /**< Struct Peripheral Mode register. */
    npz_register_perp_s perp;   /**< Struct Peripheral Polling register. */
    npz_register_ncmdp_s ncmdp; /**< Struct Peripheral Number of Commands register. */
    npz_register_sram_s sram_p;
    npz_register_addrp_s addrp;   /**< Struct Peripheral Address register. */
    npz_register_rregp_s rregp;   /**< Struct Peripheral Address of I2C register. */
    npz_register_throvp_s throvp; /**< Struct Peripheral Threshold Over Value. */
    npz_register_thrunp_s thrunp; /**< Struct Peripheral Threshold Under Value. */
    npz_register_twtp_s twtp;     /**< Peripheral Wait Time before reading value. */
    npz_register_tcfgp_s tcfgp;   /**< Peripheral Wait Time configuration. */
} peripheral_config_s;

// Array to hold indices of configured peripherals
static int m_configured_indices[4] = {-1, -1, -1, -1}; /**< Initialize with invalid indices. */
static int m_configured_count = 0;                     /**< Count of configured peripherals. */
static int m_sram_count = 0;                           /**< Count of Bytes writed to SRAM. */

/*****************************************************************************
 * Private Methods
 *****************************************************************************/

/**
 * @brief Sets global time out until host wakes up.
 */
static bool set_global_timeout(npz_device_config_s * device_config)
{
    npz_register_tout_s time_out_config = {0};

    if (device_config->global_timeout > 0)
    {
        time_out_config.tout_l = (uint8_t)(device_config->global_timeout & 0xFF); // Lower 8 bits
        time_out_config.tout_h =
            (uint8_t)((device_config->global_timeout >> 8) & 0xFF); // Upper 8 bits

        if (npz_write_TOUT(time_out_config) != OK)
        {
            printf("Failed to write Timeout register\r\n");
            return false;
        }
    }
    else
    {
        printf("Invalid global timeout\r\n");
        return false;
    }

    return true;
}

// Set Power Switch Control
static bool set_power_switch_control(npz_device_config_s * device_config)
{
    npz_register_pswctl_s pswctl = {0};

    if (device_config->power_switch_normal_mode_per1 > 1)
    {
        printf("Invalid power switch mode for peripheral 1\r\n");
        return false;
    }

    if (device_config->power_switch_normal_mode_per2 > 1)
    {
        printf("Invalid power switch mode for peripheral 2\r\n");
        return false;
    }

    if (device_config->power_switch_normal_mode_per3 > 1)
    {
        printf("Invalid power switch mode for peripheral 3\r\n");
        return false;
    }

    if (device_config->power_switch_normal_mode_per4 > 1)
    {
        printf("Invalid power switch mode for peripheral 4\r\n");
        return false;
    }

    switch (device_config->host_power_mode)
    {
        case HOST_POWER_MODE_SWITCH:
        case HOST_POWER_MODE_LOGIC_OUTPUT:
        case HOST_POWER_MODE_LOGIC_OUTPUT_INV:
            // Valid mode, do nothing
            break;
        default:
            printf("Invalid host power switch mode\r\n");
            return false;
    }

    if (device_config->power_switch_gate_boost > 1)
    {
        printf("Invalid gate boost value for power switches\r\n");
        return false;
    }

    pswctl.pswint_p1 = device_config->power_switch_normal_mode_per1; // Peripheral 1
    pswctl.pswint_p2 = device_config->power_switch_normal_mode_per2; // Peripheral 2
    pswctl.pswint_p3 = device_config->power_switch_normal_mode_per3; // Peripheral 3
    pswctl.pswint_p4 = device_config->power_switch_normal_mode_per4; // Peripheral 4
    pswctl.pswh_mode = device_config->host_power_mode;
    pswctl.psw_en_vn = device_config->power_switch_gate_boost;
    if (npz_write_PSWCTL(pswctl) != OK)
    {
        printf("Failed to write power control register\r\n");
        return false;
    }

    return true;
}

static bool set_system_config1(npz_device_config_s * device_config)
{
    npz_register_syscfg1_s syscfg1 = {0};

    syscfg1.wup1 = device_config->wake_up_per1;
    syscfg1.wup2 = device_config->wake_up_per2;
    syscfg1.wup3 = device_config->wake_up_per3;
    syscfg1.wup4 = device_config->wake_up_per4;

    syscfg1.adc_int_wakeup_enable = device_config->adc_channels[0]->wakeup_enable;
    syscfg1.adc_ext_wakeup_enable = device_config->adc_channels[1]->wakeup_enable;
    syscfg1.wake_up_any_or_all = device_config->wake_up_any_or_all;

    if (npz_write_SYSCFG1(syscfg1) != OK)
    {
        printf("Failed to write SYSCFG1 register\r\n");
        return false;
    }

    return true;
}

static bool set_system_config2(npz_device_config_s * device_config)
{
    npz_register_syscfg2_s syscfg2 = {0};

    switch (device_config->system_clock_divider)
    {
        case SCLK_DIV_DISABLE:
            syscfg2.sclk_div_en = 0; // Disable clock division
            syscfg2.sclk_div_sel = 0;
            break;
        case SCLK_DIV_2:
            syscfg2.sclk_div_en = 1; // Enable clock division
            syscfg2.sclk_div_sel = 0;
            break;
        case SCLK_DIV_4:
            syscfg2.sclk_div_en = 1; // Enable clock division
            syscfg2.sclk_div_sel = 1;
            break;
        case SCLK_DIV_8:
            syscfg2.sclk_div_en = 1; // Enable clock division
            syscfg2.sclk_div_sel = 2;
            break;
        case SCLK_DIV_16:
            syscfg2.sclk_div_en = 1; // Enable clock division
            syscfg2.sclk_div_sel = 3;
            break;
        default:
            printf("Invalid system clock divider value\r\n");
            return false;
    }

    syscfg2.sclk_sel = device_config->system_clock_source;

    syscfg2.adc_ext_on = device_config->adc_ext_sampling_enable;

    switch (device_config->adc_clock_sel)
    {
        case ADC_CLK_SC:
            syscfg2.adc_clk_sel = 0; // Disable ADC clock
            break;
        case ADC_CLK_64:
            syscfg2.adc_clk_sel = 1; // Enable ADC clock 64 Hz
            break;
        case ADC_CLK_256:
            syscfg2.adc_clk_sel = 2; // Enable ADC clock 256 Hz
            break;
        case ADC_CLK_1024:
            syscfg2.adc_clk_sel = 3; // Enable ADC clock 1024 Hz
            break;
        default:
            printf("Invalid adc clock divider value\r\n");
            return false;
    }

    if (npz_write_SYSCFG2(syscfg2) != OK)
    {
        printf("Failed to write SYSCFG2 register\r\n");
        return false;
    }

    return true;
}

static bool set_system_config3(npz_device_config_s * device_config)
{
    npz_register_syscfg3_s syscfg3 = {0};

    syscfg3.io_str = device_config->io_strength;

    syscfg3.i2c_pup_en = device_config->i2c_pull_mode & 0x01;
    syscfg3.i2c_pup_auto = (device_config->i2c_pull_mode >> 1) & 0x01;

    syscfg3.spi_auto = device_config->spi_auto;

    syscfg3.xo_clkout_div = device_config->xo_clock_out_sel;

    if (npz_write_SYSCFG3(syscfg3) != OK)
    {
        printf("Failed to write SYSCFG3 register\r\n");
        return false;
    }
    return true;
}

static bool set_interrupt_pin_config(npz_device_config_s * device_config)
{
    npz_register_intcfg_s intcfg = {0};

    switch (device_config->interrupt_pin_pull_up_pin1)
    {
        case INT_PIN_PULL_DISABLED:
            intcfg.pu_int1 = 0;
            intcfg.pu_s_int1 = 0;
            break;
        case INT_PIN_PULL_LOW:
            intcfg.pu_int1 = 1;
            intcfg.pu_s_int1 = INT_PIN_PULL_LOW >> 1;
            break;
        case INT_PIN_PULL_HIGH:
            intcfg.pu_int1 = 1;
            intcfg.pu_s_int1 = INT_PIN_PULL_HIGH >> 1;
            break;
        default:
            printf("Invalid interrupt pin 1 value\r\n");
            return false;
    }

    switch (device_config->interrupt_pin_pull_up_pin2)
    {
        case INT_PIN_PULL_DISABLED:
            intcfg.pu_int2 = 0;
            intcfg.pu_s_int2 = 0;
            break;
        case INT_PIN_PULL_LOW:
            intcfg.pu_int2 = 1;
            intcfg.pu_s_int2 = INT_PIN_PULL_LOW >> 1;
            break;
        case INT_PIN_PULL_HIGH:
            intcfg.pu_int2 = 1;
            intcfg.pu_s_int2 = INT_PIN_PULL_HIGH >> 1;
            break;
        default:
            printf("Invalid interrupt pin 2 value\r\n");
            return false;
    }

    switch (device_config->interrupt_pin_pull_up_pin3)
    {
        case INT_PIN_PULL_DISABLED:
            intcfg.pu_int3 = 0;
            intcfg.pu_s_int3 = 0;
            break;
        case INT_PIN_PULL_LOW:
            intcfg.pu_int3 = 1;
            intcfg.pu_s_int3 = INT_PIN_PULL_LOW >> 1;
            break;
        case INT_PIN_PULL_HIGH:
            intcfg.pu_int3 = 1;
            intcfg.pu_s_int3 = INT_PIN_PULL_HIGH >> 1;
            break;
        default:
            printf("Invalid interrupt pin 3 value\r\n");
            return false;
    }

    switch (device_config->interrupt_pin_pull_up_pin4)
    {
        case INT_PIN_PULL_DISABLED:
            intcfg.pu_int4 = 0;
            intcfg.pu_s_int4 = 0;
            break;
        case INT_PIN_PULL_LOW:
            intcfg.pu_int4 = 1;
            intcfg.pu_s_int4 = INT_PIN_PULL_LOW >> 1;
            break;
        case INT_PIN_PULL_HIGH:
            intcfg.pu_int4 = 1;
            intcfg.pu_s_int4 = INT_PIN_PULL_HIGH >> 1;
            break;
        default:
            printf("Invalid interrupt pin 4 value\r\n");
            return false;
    }

    if (npz_write_INTCFG(intcfg) != OK)
    {
        printf("Failed to write INTCFG register\r\n");
        return false;
    }

    return true;
}

static bool set_peripheral_power_mode(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].cfgp.pwmod = device_config->peripherals[index]->power_mode;
    peripheral[index].cfgp.tmod = device_config->peripherals[index]->polling_mode;
    peripheral[index].cfgp.pswmod = device_config->peripherals[index]->power_switch_mode;
    peripheral[index].cfgp.intmod = device_config->peripherals[index]->interrupt_pin_mode;
    if (npz_write_CFGP(switch_id, peripheral[index].cfgp) != OK)
    {
        printf("Failed to write CFGP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_mode(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].modp.cmod = device_config->peripherals[index]->comparison_mode;
    peripheral[index].modp.dtype = device_config->peripherals[index]->sensor_data_type;
    peripheral[index].modp.seqrw = device_config->peripherals[index]->multi_byte_transfer_enable;

    if (device_config->peripherals[index]->communication_protocol == COM_I2C)
    {
        peripheral[index].modp.wunak = device_config->peripherals[index]->i2c_cfg.wake_on_nak;
    }
    else if (device_config->peripherals[index]->communication_protocol == COM_SPI)
    {
        peripheral[index].modp.spimod = device_config->peripherals[index]->spi_cfg.mode;
    }

    peripheral[index].modp.swprreg = device_config->peripherals[index]->swap_registers;
    if (npz_write_MODP(switch_id, peripheral[index].modp) != OK)
    {
        printf("Failed to write MODP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_polling_period(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].perp.perp_l =
        (uint8_t)(device_config->peripherals[index]->polling_period & 0xFF);
    peripheral[index].perp.perp_h =
        (uint8_t)((device_config->peripherals[index]->polling_period >> 8) & 0xFF);

    if (npz_write_PERP(switch_id, peripheral[index].perp) != OK)
    {
        printf("Failed to write PERP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_init_cmds_number(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    uint8_t SRAM_number_bytes_2_write = 0;

    if (device_config->peripherals[index]->communication_protocol == COM_I2C)
    {
        peripheral[index].ncmdp.ncmdp = device_config->peripherals[index]->i2c_cfg.command_num;

        if (m_sram_count <= SRAM_SIZE)
        {
        	SRAM_number_bytes_2_write = device_config->peripherals[index]->i2c_cfg.command_num * 2;

            for (uint8_t i = 0; i < SRAM_number_bytes_2_write; i++)
            {
                if (npz_write_SRAM(SRAM_START + m_sram_count,
                        device_config->peripherals[index]->i2c_cfg.bytes_from_sram[i]) != OK)
                {
                    printf("Failed to write SRAM Value for peripheral %d", index + 1);
                    return false;
                }
                else
                {
//                    printf(
//                        "BYTE write SRAM Value for peripheral %d value [%02x] SRAM Addr [%02x] \n\r",
//                        index + 1,
//                        device_config->peripherals[index]->i2c_cfg.bytes_from_sram[i],
//                        SRAM_START + m_sram_count);
                    m_sram_count++;
                }
            }
        }
        else
        {
            printf("No SRAM space available for peripheral %d \n\r", index + 1);
        }
    }
    else if (device_config->peripherals[index]->communication_protocol == COM_SPI)
    {
    	peripheral[index].ncmdp.ncmdp = device_config->peripherals[index]->spi_cfg.bytes_from_sram_num;

    	if (m_sram_count <= SRAM_SIZE)
        {
        	SRAM_number_bytes_2_write = device_config->peripherals[index]->i2c_cfg.command_num;

            for (uint8_t i = 0; i < SRAM_number_bytes_2_write; i++)
            {
                if (npz_write_SRAM(SRAM_START + m_sram_count,
                        device_config->peripherals[index]->spi_cfg.bytes_from_sram[i]) != OK)
                {
                    printf("Failed to write SRAM Value for peripheral %d", index + 1);
                    return false;
                }
                else
                {
//                    printf(
//                        "BYTE write SRAM Value for peripheral %d value [%02x] SRAM Addr [%02x] \n\r",
//                        index + 1,
//                        device_config->peripherals[index]->spi_cfg.bytes_from_sram[i],
//                        SRAM_START + m_sram_count);
                    m_sram_count++;
                }
            }
        }
        else
        {
            printf("No SRAM space available for peripheral %d \n\r", index + 1);
        }

        if (device_config->peripherals[index]->spi_cfg.bytes_from_sram_read_num > 0)
        {
        	SRAM_number_bytes_2_write = device_config->peripherals[index]->spi_cfg.bytes_from_sram_read_num;
            for (uint8_t i = 0; i < SRAM_number_bytes_2_write; i++)
             {
                 if (npz_write_SRAM(SRAM_START + m_sram_count,
                         device_config->peripherals[index]->spi_cfg.bytes_from_sram_read[i]) != OK)
                 {
                     printf("Failed to write SRAM Value for peripheral %d", index + 1);
                     return false;
                 }
                 else
                 {
//                     printf(
//                         "BYTE write SRAM Value for peripheral %d value [%02x] SRAM Addr [%02x] \n\r",
//                         index + 1,
//                         device_config->peripherals[index]->spi_cfg.bytes_from_sram_read[i],
//                         SRAM_START + m_sram_count);
                     m_sram_count++;
                 }
             }

        }
        else
        {
            printf("No SRAM space available for peripheral %d \n\r", index + 1);
        }


    }

    if (npz_write_NCMDP(switch_id, peripheral[index].ncmdp) != OK)
    {
        printf("Failed to write NCMDP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_address(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    if (device_config->peripherals[index]->communication_protocol == COM_I2C)
    {
        peripheral[index].addrp.addrp =
            device_config->peripherals[index]->i2c_cfg.sensor_address;
    }
    else if (device_config->peripherals[index]->communication_protocol == COM_SPI)
    {
        peripheral[index].addrp.addrp =
            device_config->peripherals[index]->spi_cfg.bytes_from_sram_read_num;
    }

    peripheral[index].addrp.spi_en = device_config->peripherals[index]->communication_protocol;
    if (npz_write_ADDRP(switch_id, peripheral[index].addrp) != OK)
    {
        printf("Failed to write ADDRP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

// Set I2C Read Register for Peripheral
static bool set_peripheral_i2c_read_register(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    if (device_config->peripherals[index] != NULL &&
        device_config->peripherals[index]->communication_protocol == COM_I2C)
    {
        peripheral[index].rregp.rregp =
            device_config->peripherals[index]->i2c_cfg.reg_address_value;
        if (npz_write_RREGP(switch_id, peripheral[index].rregp) != OK)
        {
            printf("Failed to write RREGP register for peripheral %d\r\n", index + 1);
            return false;
        }
    }

    return true;
}

static bool set_peripheral_under_threshold(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].thrunp.thrunp_l =
        (uint8_t)(device_config->peripherals[index]->threshold_under & 0xFF);
    peripheral[index].thrunp.thrunp_h =
        (uint8_t)((device_config->peripherals[index]->threshold_under >> 8) & 0xFF);

    if (npz_write_THRUNP(switch_id, peripheral[index].thrunp) != OK)
    {
        printf("Failed to write THRUNP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_over_threshold(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].throvp.throvp_l =
        (uint8_t)(device_config->peripherals[index]->threshold_over & 0xFF);
    peripheral[index].throvp.throvp_h =
        (uint8_t)((device_config->peripherals[index]->threshold_over >> 8) & 0xFF);
    if (npz_write_THROVP(switch_id, peripheral[index].throvp) != OK)
    {
        printf("Failed to write THROVP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_time_to_wait_config(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    switch (device_config->peripherals[index]->pre_wait_time)
    {
        case PRE_WAIT_TIME_DISABLED:
            peripheral[index].tcfgp.twt_en = 0;
            peripheral[index].tcfgp.twt_ext = 0;
            break;
        case PRE_WAIT_TIME_EXTEND_256:
            peripheral[index].tcfgp.twt_en = 1;
            peripheral[index].tcfgp.twt_ext = PRE_WAIT_TIME_EXTEND_256 >> 1;
            break;
        case PRE_WAIT_TIME_EXTEND_4096:
            peripheral[index].tcfgp.twt_en = 1;
            peripheral[index].tcfgp.twt_ext = PRE_WAIT_TIME_EXTEND_4096 >> 1;
            break;
        default:
            printf("Invalid pre-initialization wait time\r\n");
            return false;
    }

    switch (device_config->peripherals[index]->post_wait_time)
    {
        case POST_WAIT_TIME_DISABLED:
            peripheral[index].tcfgp.tinit_en = 0;
            peripheral[index].tcfgp.tinit_ext = 0;
            break;
        case POST_WAIT_TIME_EXTEND_256:
            peripheral[index].tcfgp.tinit_en = 1;
            peripheral[index].tcfgp.tinit_ext = 0;
            break;
        case POST_WAIT_TIME_EXTEND_4096:
            peripheral[index].tcfgp.tinit_en = 1;
            peripheral[index].tcfgp.tinit_ext = 1;
            break;
        default:
            printf("Invalid post-initialization wait time\r\n");
            return false;
    }

    if (device_config->peripherals[index]->communication_protocol == COM_I2C)
    {
        peripheral[index].tcfgp.i2cret =
            device_config->peripherals[index]->i2c_cfg.num_of_retries_on_nak;
    }

    if (npz_write_TCFGP(switch_id, peripheral[index].tcfgp) != OK)
    {
        printf("Failed to write TCFGP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool set_peripheral_time_to_wait(npz_device_config_s * device_config,
    peripheral_config_s * peripheral, int index, npz_psw_e switch_id)
{
    peripheral[index].twtp.twtp = device_config->peripherals[index]->time_to_wait;
    if (npz_write_TWTP(switch_id, peripheral[index].twtp) != OK)
    {
        printf("Failed to write TWTP register for peripheral %d\r\n", index + 1);
        return false;
    }

    return true;
}

static bool validate_peripherals(npz_device_config_s * device_config)
{
    m_configured_count = 0; // Reset count for each validation

    for (int i = 0; i < 4; i++)
    {
        // Check if peripheral configuration is not NULL
        if (device_config->peripherals[i] != NULL)
        {
            m_configured_indices[m_configured_count++] = i; // Store index of configured peripheral
        }
    }

    return true;
}

static bool configure_peripherals(npz_device_config_s * device_config)
{
    peripheral_config_s peripherals[4] = {0};
    npz_psw_e switches[4] = {
        PSW_LP1, PSW_LP2, PSW_LP3, PSW_LP4}; // Array to hold corresponding low power switch enums

    // Iterate through the peripherals only if they are not NULL
    for (int j = 0; j < m_configured_count; j++)
    {
        int i = m_configured_indices[j]; // Get the index of the configured peripheral

        // Set power mode for peripheral configuration (CFGP)
        if (!set_peripheral_power_mode(device_config, peripherals, i, switches[i]))
        {
            printf("Failed to set peripheral config for peripheral %d\r\n", i + 1);
            return false;
        }

        // Set mode for Peripheral (MODP)
        if (!set_peripheral_mode(device_config, peripherals, i, switches[i]))
        {
            printf("Failed to set peripheral mode for peripheral %d\r\n", i + 1);
            return false;
        }

        // Set Polling Period for a Peripheral (PERP)
        if (!set_peripheral_polling_period(device_config, peripherals, i, switches[i]))
        {
            printf("Failed to set peripheral polling period for peripheral %d\r\n", i + 1);
            return false;
        }

        // Set Number of init Commands for Peripheral (NCMDP)
        if (!set_peripheral_init_cmds_number(device_config, peripherals, i, switches[i]))
        {
            printf(
                "Failed to set peripheral init commands number for peripheral %d\r\n", i + 1);
            return false;
        }

        if ((device_config->peripherals[i]->polling_mode ==
                POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode ==
                POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_WAIT_INTERRUPT))
        {
            // Set Address for a Peripheral (ADDRP)
            if (!set_peripheral_address(device_config, peripherals, i, switches[i]))
            {
                printf("Failed to set peripheral address for peripheral %d\r\n", i + 1);
                return false;
            }
        }

        if ((device_config->peripherals[i]->polling_mode ==
                POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode ==
                POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD))
        {
            // Set I2C Read Register for Peripheral (RREGP)
            if (!set_peripheral_i2c_read_register(device_config, peripherals, i, switches[i]))
            {
                printf(
                    "Failed to set peripheral I2C read register for peripheral %d\r\n", i + 1);
                return false;
            }

            // Set Under Threshold for a Peripheral (THRUNP)
            if (!set_peripheral_under_threshold(device_config, peripherals, i, switches[i]))
            {
                printf(
                    "Failed to set peripheral under threshold for peripheral %d\r\n", i + 1);
                return false;
            }

            // Set Over Threshold for a Peripheral (THROVP)
            if (!set_peripheral_over_threshold(device_config, peripherals, i, switches[i]))
            {
                printf(
                    "Failed to set peripheral over threshold for peripheral %d\r\n", i + 1);
                return false;
            }
        }

        // Set time to Wait for Peripheral (TWTP)
        if (!set_peripheral_time_to_wait(device_config, peripherals, i, switches[i]))
        {
            printf("Failed to set peripheral time to wait for peripheral %d\r\n", i + 1);
            return false;
        }

        // Set Time to Wait Config for Peripheral (TCFGP)
        if (!set_peripheral_time_to_wait_config(device_config, peripherals, i, switches[i]))
        {
            printf(
                "Failed to set peripheral time to wait config for peripheral %d\r\n", i + 1);
            return false;
        }
    }

    return true;
}

static bool configure_internal_adc(npz_device_config_s * device_config)
{
    int_adc_channel_config_s adc_channel = {0};

    if (device_config->adc_channels[0]->over_threshold > 0 &&
        device_config->adc_channels[0]->under_threshold > 0)
    {
        // Set Internal ADC over threshold Value
        adc_channel.throva1.throva = device_config->adc_channels[0]->over_threshold;
        if (npz_write_THROVA1(adc_channel.throva1) != OK)
        {
            printf("Failed to write THROVA1 register for internal ADC\r\n");
            return false;
        }

        // Set Internal ADC under threshold value
        adc_channel.thruna1.thruna = device_config->adc_channels[0]->under_threshold;
        if (npz_write_THRUNA1(adc_channel.thruna1) != OK)
        {
            printf("Failed to write THRUNA1 register for internal ADC\r\n");
            return false;
        }
    }
    else
    {
        printf("Failed to configure internal ADC\r\n");
        return false;
    }

    return true;
}

static bool configure_external_adc(npz_device_config_s * device_config)
{
    ext_adc_channel_config_s adc_channel = {0};

    if (device_config->adc_channels[1]->over_threshold > 0 &&
        device_config->adc_channels[1]->under_threshold > 0)
    {
        // Set External ADC over threshold Value
        adc_channel.throva2.throva = device_config->adc_channels[1]->over_threshold;
        if (npz_write_THROVA2(adc_channel.throva2) != OK)
        {
            printf("Failed to write THROVA2 register for external ADC\r\n");
            return false;
        }

        // Set External ADC under threshold value
        adc_channel.thruna2.thruna = device_config->adc_channels[1]->under_threshold;
        if (npz_write_THRUNA2(adc_channel.thruna2) != OK)
        {
            printf("Failed to write THRUNA2 register for external ADC\r\n");
            return false;
        }
    }
    else
    {
        printf("Failed to configure external ADC\r\n");
        return false;
    }

    return true;
}

static bool configure_global_settings(npz_device_config_s * device_config)
{
    // Set global time out
    if (!set_global_timeout(device_config))
    {
        printf("Failed to set global timeout\r\n");
        return false;
    }

    // Set System Config 1
    if (!set_system_config1(device_config))
    {
        printf("Failed to set system config 1\r\n");
        return false;
    }

    // Set System Config 2
    if (!set_system_config2(device_config))
    {
        printf("Failed to set system config 2\r\n");
        return false;
    }

    // Set System Config 3
    if (!set_system_config3(device_config))
    {
        printf("Failed to set system config 3\r\n");
        return false;
    }

    // Set power switch control
    if (!set_power_switch_control(device_config))
    {
        printf("Failed to set power switch control\r\n");
        return false;
    }

    // Set Interrupt Pin's Configuration
    if (!set_interrupt_pin_config(device_config))
    {
        printf("Failed to set interrupt pin's configuration\r\n");
        return false;
    }


    return true;
}

static bool adc_ext_code_to_voltage(uint8_t code)
{
    int n;
    bool code_found = false;
    int n_max = sizeof(adc_ext_code_map) / sizeof(struct m_adc_ext_code_level_map);

    for (n = 0; n < n_max; n++)
    {
        if (adc_ext_code_map[n].code == code)
        {
            printf("Reading value from adc channel is 0x%02X corresponds to an input voltage "
                         "of %d.%03d V \r\n",
                code, adc_ext_code_map[n].millivolts / 1000, adc_ext_code_map[n].millivolts % 1000);
            code_found = true;
            break;
        }
    }

    if (!code_found)
    {
        printf("ADC reading value 0x%02X does not match any expected code \r\n", code);
        return false;
    }

    return true;
}

static bool adc_core_code_to_voltage(uint8_t code)
{
    int n;
    bool code_found = false;
    int n_max = sizeof(adc_core_code_map) / sizeof(struct m_adc_core_code_level_map);

    for (n = 0; n < n_max; n++)
    {
        if (adc_core_code_map[n].code == code)
        {
            printf("Reading value from adc channel is 0x%02X corresponds to an input voltage "
                         "of %d.%03d V \r\n",
                code, adc_core_code_map[n].millivolts / 1000,
                adc_core_code_map[n].millivolts % 1000);
            code_found = true;
            break;
        }
    }

    if (!code_found)
    {
        printf("ADC reading value 0x%02X does not match any expected code \r\n", code);
        return false;
    }

    return true;
}


/*****************************************************************************
 * Public Methods
 *****************************************************************************/

bool npz_device_handle_adc_external(void)
{
    npz_register_adc_ext_s get_adc_ext_val = {0};
    npz_register_syscfg1_s syscfg1 = {0};
    npz_register_syscfg2_s syscfg2 = {0};

    if (npz_read_SYSCFG1(&syscfg1) != OK)
    {

        printf("Failed to read SYSCFG1 register\r\n");
        return false;
    }

    if (npz_read_SYSCFG2(&syscfg2) != OK)
    {

        printf("Failed to read SYSCFG2 register\r\n");
        return false;
    }

    if (syscfg2.adc_ext_on == 1 && syscfg1.adc_ext_wakeup_enable == 1)
    {
        if (npz_read_ADC_EXT(&get_adc_ext_val) != OK)
        {
            printf("Failed to read ADC_EXT register\r\n");
            return false;
        }

        if (get_adc_ext_val.adc_ext == 0x1F)
        {
            printf("ADC_IN analog pin not connected. Please connect the pin.\r\n");
        }
        else
        {
            printf("External ADC channel (connected to ADC_IN) was triggered\r\n");
            bool adc_ext = adc_ext_code_to_voltage(get_adc_ext_val.adc_ext);
            if (!adc_ext)
            {
                return false;
            }
        }
    }

    return true;
}

bool npz_device_handle_adc_internal(void)
{
    npz_register_adc_core_s get_adc_core_val = {0};

    printf("Internal ADC channel (connected to VBAT) was triggered\r\n");
    if (npz_read_ADC_CORE(&get_adc_core_val) != OK)
    {
        printf("Failed to read ADC_CORE register\r\n");
        return false;
    }

    bool adc_core = adc_core_code_to_voltage(get_adc_core_val.adc_core);
    if (!adc_core)
    {
        return false;
    }

    return true;
}


bool npz_device_read_peripheral_value(npz_psw_e psw_lp, int index, int * peripheral_value)
{
    npz_register_cfgp_s cfgp = {0};
    npz_register_valp_s valp = {0};
    npz_register_addrp_s addrp = {0};

    printf("External Trigger from Peripheral %d\r\n", psw_lp);

    if (npz_read_CFGP(psw_lp, &cfgp) != OK)
    {
        printf("Failed to read CFGP register for peripheral %d\r\n", psw_lp);
        return false;
    }

    printf("Polling mode: %d\r\n", cfgp.tmod);

    if ((cfgp.tmod == POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD) ||
        (cfgp.tmod == POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD))
    {
        if (npz_read_VALP(psw_lp, &valp) != OK)
        {
            printf("Failed to read VALP register for peripheral %d\r\n", psw_lp);
            return false;
        }

        if (npz_read_ADDRP(psw_lp, &addrp) != OK)
        {
            printf("Failed to read ADDRP register for peripheral %d\r\n", psw_lp);
            return false;
        }

        if (addrp.spi_en == 0) // I2C communication protocol
        {
            *peripheral_value = (valp.valp_h << 8) | valp.valp_l;

            printf("Reading value from I2C Peripheral %d is 0x%02X 0x%02X\r\n", psw_lp,
                valp.valp_h, valp.valp_l);
        }
        else // SPI communication protocol
        {
            *peripheral_value = (valp.valp_h << 8) | valp.valp_l;

            printf("Reading value from SPI Peripheral %d is 0x%02X 0x%02X\r\n", psw_lp,
                valp.valp_h, valp.valp_l);
        }
    }

    return true;
}

/**
 * @brief Put the nPZero device in idle mode.
 */
void npz_device_go_to_idle(void)
{
    printf("Enter idle mode and disable I2C bus\r\n");

    uint8_t idle_rst_value = 0xFF;
    if (npz_write_IDLE_RST(idle_rst_value) != OK)
    {
        printf("Failed to write to IDLE_RST register\r\n");
    }
}

/**
 * @brief Reset the nPZero device by software.
 */
void npz_device_soft_reset(void)
{
    printf("Software reset\r\n");

    uint8_t idle_rst_value = 0xA5;
    if (npz_write_IDLE_RST(idle_rst_value) != OK)
    {
        printf("Failed to write to IDLE_RST register\r\n");
    }
}

/**
 * @brief Modify device configuration.
 */
void npz_device_configure(npz_device_config_s * device_config)
{
    // Ensure the device_config is not NULL
    if (device_config == NULL)
    {
        printf("Invalid device configuration pointer\r\n");
        return;
    }

    // Configure global settings
    if (!configure_global_settings(device_config))
    {
        printf("Failed to configure global settings\r\n");
        return;
    }

    // Validate peripherals
    if (validate_peripherals(device_config))
    {
        // Configure peripherals
        if (!configure_peripherals(device_config))
        {
            printf("Failed to configure peripherals\r\n");
            return;
        }
    }

    if (device_config->adc_channels[0]->wakeup_enable == 1)
    {
        if (!configure_internal_adc(device_config))
        {
            printf("Failed to configure internal ADC\r\n");
            return;
        }
    }

    if (device_config->adc_channels[1]->wakeup_enable == 1 &&
        device_config->adc_ext_sampling_enable == 1)
    {
        if (!configure_external_adc(device_config))
        {
            printf("Failed to configure external ADC\r\n");
            return;
        }
    }
}

/**
 * @file npz.h
 *
 * @brief Header file for the nPZero Driver.
 *
 * This header file contains declarations and definitions for the driver for controlling and managing
 * the nPZero power-saving IC.
 * It provides function prototypes, constants, enumerations and structures necessary for interacting with the nPZero on a register level.
 *
 * This driver is designed to work with the nPZero G1S IC from Nanopower Semiconductor.
 * It utilizes the I2C communication protocol for interfacing the host MCU.
 */

#ifndef __NPZ_H
#define __NPZ_H

/** @cond */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "npz_hal.h"
/** @endcond */

/** Enumerations. */

/** Reset Reason, see npz_register_sta1_s. */
typedef enum
{
    RESETSOURCE_NONE = 0x00,       /**< None. */
    RESETSOURCE_PWR_RESET = 0x01,  /**< Power-on reset occurred. */
    RESETSOURCE_EXT_RESET = 0x02,  /**< External reset occurred (via RST pin). */
    RESETSOURCE_SOFT_RESET = 0x04, /**< Soft reset occurred (via I2C command). */
} npz_resetsource_e;

/** Host power switch mode (SW_HP pin), see npz_register_pswctl_s. */
typedef enum
{
    HOST_POWER_MODE_SWITCH = 0x01,           /**< Power switch (outputs VBAT or open). */
    HOST_POWER_MODE_LOGIC_OUTPUT = 0x02,     /**< Logic output (High when host enabled). */
    HOST_POWER_MODE_LOGIC_OUTPUT_INV = 0x03, /**< Logic output inverted (Low when host enabled). */
} npz_host_power_mode_e;

/** Low Power switches, see npz_register_pswctl_s. */
typedef enum
{
    PSW_LP1 = 0x01, /**< Low power switch 1, Default SPI. */
    PSW_LP2 = 0x02, /**< Low power switch 2, Default I2C. */
    PSW_LP3 = 0x03, /**< Low power switch 3, Default SPI. */
    PSW_LP4 = 0x04, /**< Low power switch 4, Default I2C. */
} npz_psw_e;

/** Peripheral power mode, see npz_register_cfgp_s. */
typedef enum
{
    POWER_MODE_DISABLED = 0x00,  /**< Disabled. */
    POWER_MODE_PERIODIC = 0x01,  /**< Periodic power-on. */
    POWER_MODE_ALWAYS_ON = 0x03, /**< Always on. */
} npz_power_mode_e;

/** Peripheral polling mode, see npz_register_cfgp_s. */
typedef enum
{
    POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD = 0x00, /**< Periodic initialization,
                                                          * read data, compare against threshold. */
    POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD =
        0x01,                                    /**< Periodic initialization,
                                                  * wait for interrupt, read data, compare against threshold. */
    POLLING_MODE_PERIODIC_WAIT_INTERRUPT = 0x02, /**< Periodic initialization, wait for interrupt. */
    POLLING_MODE_ASYNC_WAIT_INTERRUPT = 0x03,    /**< Wait for asynchronous interrupt. */
} npz_polling_mode_e;

/** Peripheral power switch mode (pins SW_LP*), see npz_register_cfgp_s. */
typedef enum
{
    POWER_SWITCH_MODE_VOLTAGE_DETECTION = 0x00, /**< Power switch with output voltage rise detection. */
    POWER_SWITCH_MODE_STANDARD = 0x01,          /**< Standard power switch mode. */
    POWER_SWITCH_MODE_LOGIC_OUTPUT_HIGH = 0x02, /**< Logic output mode (High when peripheral enabled). */
    POWER_SWITCH_MODE_LOGIC_OUTPUT_LOW = 0x03,  /**< Logic output inverted mode (Low when peripheral enabled). */
} npz_power_switch_mode_e;

/** Peripheral interrupt pin mode (pins INT*), see npz_register_cfgp_s. */
typedef enum
{
    INTERRUPT_PIN_MODE_INPUT_ACTIVE_HIGH = 0x00,   /**< Interrupt input mode (active High). */
    INTERRUPT_PIN_MODE_INPUT_ACTIVE_LOW = 0x01,    /**< Interrupt input mode (active Low). */
    INTERRUPT_PIN_MODE_TRIGGER_OUTPUT_HIGH = 0x02, /**< Peripheral trigger output mode (active High). */
    INTERRUPT_PIN_MODE_TRIGGER_OUTPUT_LOW = 0x03,  /**< Peripheral trigger output mode (active Low). */
} npz_interrupt_pin_mode_e;

/** Comparison mode for threshold values. */
typedef enum
{
    COMPARISON_MODE_INSIDE_THRESHOLD = 0x00,  /**< Compare inside threshold values. */
    COMPARISON_MODE_OUTSIDE_THRESHOLD = 0x01, /**< Compare outside threshold values. */
} npz_comparison_mode_e;

/** Data type value of value to be read from peripheral, see npz_register_modp_s. */
typedef enum
{
    DATA_TYPE_UINT16 = 0x00, /**< 16-bit unsigned integer. */
    DATA_TYPE_INT16 = 0x01,  /**< 16-bit signed integer. */
    DATA_TYPE_UINT8 = 0x02,  /**< 8-bit unsigned integer. */
} npz_data_type_e;

/** SPI modes available if SPI is enabled, see npz_register_modp_s. */
typedef enum
{
    SPIMOD_SPI_MODE_0 = 0x00, /**< CPOL = 0 CPHA = 0. */
    SPIMOD_SPI_MODE_1 = 0x01, /**< CPOL = 0 CPHA = 1. */
    SPIMOD_SPI_MODE_2 = 0x02, /**< CPOL = 1 CPHA = 0. */
    SPIMOD_SPI_MODE_3 = 0x03, /**< CPOL = 1 CPHA = 1. */
} npz_spimod_e;

/** Controls system clock source between internal slow oscillator or crystal oscillator. (0: Slow oscillator, 1: Crystal
 * oscillator). */
typedef enum
{
    SYS_CLOCK_10HZ = 0x00, /**< 10 Hz. */
    SYS_CLOCK_32KHZ = 0x01,  /**< 32.578 kHz. */
} npz_sclk_sel_e;

/** Controls system clock divider. */
typedef enum
{
    SCLK_DIV_DISABLE = 0x00, /**< Disable clock division. */
    SCLK_DIV_2 = 0x01,       /**< Divided by 2. */
    SCLK_DIV_4 = 0x03,       /**< Divided by 4. */
    SCLK_DIV_8 = 0x05,       /**< Divided by 8. */
    SCLK_DIV_16 = 0x07,      /**< Divided by 16. */
} npz_sclk_div_e;

/** Controls ADC clock select */
typedef enum
{
    ADC_CLK_SC = 0x00,      /**< System clock. */
    ADC_CLK_64 = 0x01,      /**< XO clock divided by 512 (64 Hz). */
    ADC_CLK_256 = 0x02,     /**< XO clock divided by 128 (256 Hz). */
    ADC_CLK_1024 = 0x03     /**< XO clock divided by 32 (1024 Hz). */
} npz_adc_clk_e;

/** Controls IO pull-up strength */
typedef enum
{
    IO_STR_NORMAL = 0x00,   /**< Normal pull-up strength */
    IO_STR_HIGH = 0x01,     /**< High pull-up strength */
} npz_io_str_e;

/** Controls I2C pull-ups */
typedef enum
{
    I2C_PULL_DISABLE = 0x00,   /**< I2C pull-ups disabled */
    I2C_PULL_ALWAYS_ON = 0x01, /**< I2C pull-ups always enabled */
    I2C_PULL_AUTO = 0x03,      /**< I2C pull-ups disabled in idle */
} npz_i2c_pull_sel_e;

/** Controls the SPI pin mode */
typedef enum
{
    SPI_PINS_ALWAYS_ON = 0x00,   /**< SPI pins always enabled */
    SPI_PINS_AUTO_DISABLE = 0x01,     /**< SPI pins disabled in idle (hi-Z) */
} npz_spi_auto_e;

/** Controls CLK_OUT DIV clock select */
typedef enum
{
    XO_CLK_OFF = 0x00,   /**< Turned_off. */
    XO_CLK_32K = 0x01,   /**< Divide by 1 (32 kHz). */
    XO_CLK_16K = 0x02,   /**< Divide by 2 (16 kHz). */
    XO_CLK_8K = 0x03,    /**< Divide by 4 (8 kHz). */
    XO_CLK_4K = 0x04,    /**< Divide by 8 (4 kHz). */
    XO_CLK_2K = 0x05,    /**< Divide by 16 (2 kHz). */
    XO_CLK_1K = 0x06     /**< Divide by 32 (1 kHz). */    
} npz_xo_clkout_div_e;

typedef enum
{
    DISABLED = 0x00, /**< Disabled. */
    ENABLED = 0x01,  /**< Enabled. */
} npz_states_e;

typedef enum
{
    MULTIBYTE_TRANSFER_DISABLE = 0x00, /**< Disabled multi-byte transfer. */
    MULTIBYTE_TRANSFER_ENABLE = 0x01,  /**< Enable multi-byte transfer. */
} npz_multibyte_e;

typedef enum
{
    COM_I2C = 0x00, /**< Use I2C as communication protocol. */
    COM_SPI = 0x01, /**< Use SPI as communication protocol. */
} npz_com_protocol_e;

typedef enum
{
    ENDIAN_LITTLE = 0x00, /**< Little endian. */
    ENDIAN_BIG = 0x01,    /**< Big endian. */
} npz_endianess_e;

typedef enum
{
    INT_PIN_PULL_DISABLED = 0x00,
    INT_PIN_PULL_LOW = 0x01,
    INT_PIN_PULL_HIGH = 0x03,
} npz_int_pin_pull_e;

typedef enum
{
    PRE_WAIT_TIME_DISABLED = 0x00,     /**< Pre-init wait time disable. */
    PRE_WAIT_TIME_EXTEND_256 = 0x01,  /**< Pre-init wait time x256 clocks. */
    PRE_WAIT_TIME_EXTEND_4096 = 0x03, /**< Pre-init wait time x4096 clocks. */
} npz_pre_wait_time_e;

typedef enum
{
    POST_WAIT_TIME_DISABLED = 0x00,     /**< Post-init wait time disable. */
    POST_WAIT_TIME_EXTEND_256 = 0x01,  /**< Post-init wait time x256 clocks. */
    POST_WAIT_TIME_EXTEND_4096 = 0x03, /**< Post-init wait time x4096 clocks. */
} npz_post_wait_time_e;

typedef enum
{
    WAKEUP_ANY = 0x00, /** Wake up system on any trigger. */
    WAKEUP_ALL = 0x01, /** Wake up system on all triggers. */
} npz_wakeup_e;

/* Structures */

/**  Idle/Reset register for shutting down the host MCU or resetting the nPZero device. */
typedef struct
{
    uint8_t value; /**< When set to OxFF, the device will enter idle mode, shutting down the host power and assuming
                    * control of the I2C bus. When set to 0xA5, the device will soft reset. */
} npz_register_idle_rst_s;

/** (Read) ID register for Device ID. */
typedef struct
{
    uint8_t id; /**< Device identification register. */
} npz_register_id_s;

/** (Read) Status register for verifying the reset reason and selected wakeup triggers. */
typedef struct
{
    npz_resetsource_e reset_source : 3; /**< Indicates the reason for the device reset, see @npz_resetsource_e. */
    uint8_t padding : 2;                /**< Not used. */
    uint8_t ext_adc_triggered : 1; /**< When flag is set to 1, indicates the external ADC channel (connected to ADC_IN)
                                    * was triggered. */
    uint8_t int_adc_triggered : 1; /**< When flag is set to 1, indicates the internal ADC cahnnel (connected to VBAT)
                                    * was triggered. */
    uint8_t global_timeout_triggered : 1; /**< When flag is set to 1, indicates the wake up reason was a
                                           * global timeout before any wake up source was triggered. */
} npz_register_sta1_s;

/** (Read) Status register listing which peripheral (if any) woke up the device. */
typedef struct
{
    uint8_t per1_triggered : 1;      /**< When flag is set to 1, indicates peripheral 1 was triggered. */
    uint8_t per1_global_timeout : 1; /**< When flag is set to 1, indicates the wake up reason was a global
                                      * time out before any wake up source was triggered. */
    uint8_t per2_triggered : 1;      /**< When flag is set to 1, indicates peripheral 2 was triggered. */
    uint8_t per2_global_timeout : 1; /**< When flag is set to 1, indicates the wake up reason was a global
                                      * time out before any wake up source was triggered. */
    uint8_t per3_triggered : 1;      /**< When flag is set to 1, indicates peripheral 3 was triggered. */
    uint8_t per3_global_timeout : 1; /**< When flag is set to 1, indicates the wake up reason was a global
                                      * time out before any wake up source was triggered. */
    uint8_t per4_triggered : 1;      /**< When flag is set to 1, indicates peripheral 4 was triggered. */
    uint8_t per4_global_timeout : 1; /**< When flag is set to 1, indicates the wake up reason was a global
                                      * time out before any wake up source was triggered. */
} npz_register_sta2_s;

/**  Power Control register for power switch configuration for peripherals and Host MCU. */
typedef struct
{
    uint8_t pswint_p1 : 1;               /**< Controls the power switch output state for peripheral 1. */
    uint8_t pswint_p2 : 1;               /**< Controls the power switch output state for peripheral 2. */
    uint8_t pswint_p3 : 1;               /**< Controls the power switch output state for peripheral 3. */
    uint8_t pswint_p4 : 1;               /**< Controls the power switch output state for peripheral 4. */
    npz_host_power_mode_e pswh_mode : 2; /**< Sets the host power switch mode (SW_HP pin), see npz_host_power_mode_e. */
    uint8_t psw_en_vn : 1; 				 /**< Enables power switch gate boost for reduced Rds(on). Gate boost automatically
                                              turns on for VBAT lower than 1.7 V. (0: Normal operation, 1: Switch gate boost) */
    uint8_t psw_vn_on : 1;               /**< When the flag is set to 1, it indicates that the gate boost is active. */
} npz_register_pswctl_s;

/**  System Config 1 register for clock source and peripheral wake up configuration. */
typedef struct
{
    uint8_t wup1 : 1; /**< Enables this peripheral as a wake up source when triggered. (0: Peripheral does not wake up
                       * system, 1: Peripheral can wake up system). */
    uint8_t wup2 : 1; /**< Enables this peripheral as a wake up source when triggered. (0: Peripheral does not wake up
                       * system, 1: Peripheral can wake up system). */
    uint8_t wup3 : 1; /**< Enables this peripheral as a wake up source when triggered. (0: Peripheral does not wake up
                       * system, 1: Peripheral can wake up system). */
    uint8_t wup4 : 1; /**< Enables this peripheral as a wake up source when triggered. (0: Peripheral does not wake up
                       * system, 1: Peripheral can wake up system). */
    uint8_t adc_int_wakeup_enable : 1; /**< Enables wake up on internal ADC channel trigger (VBAT pin).
                                        * (0: ADC trigger disabled, 1: ADC trigger enabled). */
    uint8_t adc_ext_wakeup_enable : 1; /**< Enables wake up on external ADC channel trigger (ADC_IN pin).
                                        * (0: ADC trigger disabled, 1: ADC trigger enabled). */
    uint8_t wake_up_any_or_all : 1;    /**< Controls if any wake up source trigger is sufficient to wake up the system,
                                        * or if all wake up source triggers are necessary to wake up.
                                        * (0: Wake up system on any trigger, 1: Wake up system on all triggers). */                      
    uint8_t padding : 1;               /**< Not used. */
} npz_register_syscfg1_s;

/**  System Config 2 register for ADC clock and wakeup configuration. */
typedef struct
{
    uint8_t sclk_div_en : 1;  /**< Enables division of system clock. (0: Disable clock
                               * division, 1: Enable clock division). */
    uint8_t sclk_div_sel : 2; /**< Controls system clock divider. */
    uint8_t sclk_sel : 1; /**< Controls system clock source between internal slow oscillator or crystal oscillator, see
                           * SCLK_SEL. */
    uint8_t adc_ext_on : 1;  /**< Enables external ADC input sampling (ADC_IN pin). When enabled, the sampling rate for
                              * both ADC channels (VBAT and ADC_IN) is halved. (0: Disable ADC_IN, 1: Enable ADC_IN). */
    uint8_t adc_clk_sel : 2; /**<Selects ADC sampling frequency, see Table 28 - ADC clock selection.
                              *For XO-derived clocks, the SCLK_SEL field must be set to 1 */
    uint8_t padding : 1;               /**< Not used. */
} npz_register_syscfg2_s;

/**  System Config 3 register for I2C/SPI configuration. */
typedef struct
{
    uint8_t io_str : 1;  /**< Controls the I/O pins’ output strength, (0: Normal strength; 1: High strength). */
    uint8_t i2c_pup_en : 1;  /**< Controls the internal I²C pull-up of ≈40KΩ. (0: Disabled; 1: Enabled). */
    uint8_t i2c_pup_auto : 1; /**< I²C internal pull-up auto control requires I2C_PUP_EN to be enabled.
                                *(0: Pull-ups always on; 1: Pull-ups in idle mode disabled). */
    uint8_t spi_auto : 1; /**< Controls the SPI outputs when the interface is not in use.
                            *(0: Always on; 1: Auto disable (HiZ)). */
    uint8_t xo_clkout_div : 3; /**< Enables the crystal oscillator and selects the division on CLK_OUT pin,
                                 * - CLK_OUT clock selection. */
    uint8_t sclk_sel_status : 1;  /**< System clock selection status. (0: Low-power oscillator; 1: Crystal oscillator). */
} npz_register_syscfg3_s;

/**  Time out register for Host MCU wake up. */
typedef struct
{
    uint8_t tout_l; /**< Time out until host wakes up if no wake up source is triggered, specified as a 16 bit value of
                     * system clock periods. */
    uint8_t tout_h; /**< Time out until host wakes up if no wake up source is triggered, specified as a 16 bit value of
                     * system clock periods. */
} npz_register_tout_s;

/**  Interrupt Config register for interrupt pull-up resistors and pull-up strength configuration. */
typedef struct
{
    uint8_t pu_int1 : 1;   /**< Enables internal pull-up resistor on interrupt pin (0: Pull-up disabled, 1: Pull-up
                            * enabled). */
    uint8_t pu_s_int1 : 1; /**< Controls pull-up resistor strength on interrupt pin (0: Low-strength pull-up, 1: High
                            * strength pull-up). */
    uint8_t pu_int2 : 1;   /**< Enables internal pull-up resistor on interrupt pin (0: Pull-up disabled, 1: Pull-up
                            * enabled). */
    uint8_t pu_s_int2 : 1; /**< Controls pull-up resistor strength on interrupt pin (0: Low-strength pull-up, 1: High
                            * strength pull-up). */
    uint8_t pu_int3 : 1;   /**< Enables internal pull-up resistor on interrupt pin (0: Pull-up disabled, 1: Pull-up
                            * enabled). */
    uint8_t pu_s_int3 : 1; /**< Controls pull-up resistor strength on interrupt pin (0: Low-strength pull-up, 1: High
                            * strength pull-up). */
    uint8_t pu_int4 : 1;   /**< Enables internal pull-up resistor on interrupt pin (0: Pull-up disabled, 1: Pull-up
                            * enabled). */
    uint8_t pu_s_int4 : 1; /**< Controls pull-up resistor strength on interrupt pin (0: Low-strength pull-up, 1: High
                            * strength pull-up). */
} npz_register_intcfg_s;

/** Peripheral Config register for Power Mode, Polling Mode, Power Switch Mode and Interrupt Mode configuration. There
 * is 4 cfgp registers, one for each Peripheral. */
typedef struct
{
    npz_power_mode_e pwmod : 2;          /**< Controls peripheral power mode, see npz_power_mode_e. */
    npz_polling_mode_e tmod : 2;         /**< Controls peripheral polling mode, see npz_polling_mode_e. */
    npz_power_switch_mode_e pswmod : 2;  /**< Sets the peripheral power switch mode (pins SW_LP*),
                                          * see @npz_power_switch_mode_e. */
    npz_interrupt_pin_mode_e intmod : 2; /**< Sets the peripheral interrupt pin mode (pins INT*), either as an input
                                          * that can be used during polling, or as a trigger output,
                                          * see @npz_interrupt_pin_mode_e. */
} npz_register_cfgp_s;

/**  Peripheral Mode register for configuration of the different modes. */
typedef struct
{
    npz_comparison_mode_e cmod : 1; /**< Comparison mode of over/under thresholds.
                                     * Inside mode triggers when value is higher or equal than over threshold or
                                     * lower or equal than under threshold. Outside mode triggers when value
                                     * is lower than over threshold and higher than under threshold.
                                     * (0: Inside mode; 1: Outside mode). */
    npz_data_type_e dtype : 2;      /**< Data type of value read From peripheral, see @npz_data_type_e. */
    uint8_t seqrw : 1; /**< Transmit sequential read and write addresses as multi-byte transfers. (0: Disable multi-byte
                        * transfers; 1: Enable multi-byte transfers). */
    uint8_t wunak : 1; /**< Wake up if peripheral does not acknowledge I2C address (NAK). (0: Continue on NAK; 1: Wake
                        * up on NAK). */
    uint8_t swprreg : 1;     /**< Swap high/low read registers of peripheral to match endianess of the data. (0: Little
                              * endian; 1: Big endian). */
    npz_spimod_e spimod : 2; /**< Sets the SPI mode if SPI communication is enabled, see @npz_spimod_e. */
} npz_register_modp_s;

/**  Peripheral Polling register for configuration of the polling period. NOTE: Zero is not a valid value and can result
 * in undefined behavior. */
typedef struct
{
    uint8_t perp_l; /**< Polling period for peripheral, specified as a 16 bit value of system clock periods. This value
                     * does not apply if peripheral is set to disabled or always-on. */
    uint8_t perp_h; /**< Polling period for peripheral, specified as a 16 bit value of system clock periods. This value
                     * does not apply if peripheral is set to disabled or always-on. */
} npz_register_perp_s;

/**  Peripheral Number of Commands register for configuration of the amount of commands to send to initialize the
 * peripheral. */
typedef struct
{
    uint8_t ncmdp : 7;   /**< Number of write commands for peripheral initialization to be read from SRAM.
                          * 12C mode: number of commands to send, where each command consists of an address byte and a
                          * value byte.   < SPI mode: number of total bytes to send from the SRAM. */
    uint8_t padding : 1; /**< Not used. */
} npz_register_ncmdp_s;

/**  Peripheral Address register for configuration of communication protocol and I2C salve address/SPI bytes for SPI
 * read. */
typedef struct
{
    uint8_t addrp : 7;  /**< I2C mode: the 7 bit I2C address of the peripheral.
                         * SPI mode: number of bytes to send from SRAM For SPI read. */
    uint8_t spi_en : 1; /**< Sets which communication protocol should be used with each peripheral.(0: I2C, 1: SPI). */
} npz_register_addrp_s;

/**  Peripheral Address of I2C register that holds the value to be read. */
typedef struct
{
    uint8_t rregp; /**< Address of IC peripheral register which contains the lower 8 bits of the value to be read.
                    * When data type is 16-bit, this register is incremented to retrieve the higher 8 bits. */
} npz_register_rregp_s;

/**  Peripheral Threshold Over Value. */
typedef struct
{
    uint8_t throvp_l; /**< Value of the peripheral over threshold, up to 16-bit, should match the data type set in
                       * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
    uint8_t throvp_h; /**< Value of the peripheral over threshold, up to 16-bit, should match the data type set in
                       * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
} npz_register_throvp_s;

/**  Peripheral Threshold Under Value. */
typedef struct
{
    uint8_t thrunp_l; /**< Value of the peripheral under threshold, up to 16-bit, should match the data type set in
                       * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
    uint8_t thrunp_h; /**< Value of the peripheral under threshold, up to 16-bit, should match the data type set in
                       * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
} npz_register_thrunp_s;

/**  Peripheral To Wait Time before reading value. */
typedef struct
{
    uint8_t twtp; /**< Period of time to wait before and/or after peripheral initialization until the value is read, or
                   * until timeout while waiting for an interrupt assertion, < defined in units of 256 or 4096 periods
                   * of the internal 400 kHz oscillator.The actual time to wait, and when to wait, depends on the value
                   * of the tcfgp register. */
} npz_register_twtp_s;

/**  Peripheral To Wait Time configuration. */
typedef struct
{
    uint8_t twt_en : 1;   /**< Enable pre-initialization wait time, as defined by TWTP1. */
    uint8_t twt_ext : 1;  /**< Extends the pre-initialization wait time to be twtp units of 4096 periods of the internal
                           * 400 kHz oscillator, instead of the default 256. (0: pre-init time x256 clocks, 1: pre-init
                           * time x4096 clocks). */
    uint8_t tinit_en : 1; /**< Enable post-initialization wait time, as defined by TWTP1. */
    uint8_t tinit_ext : 1; /**< Extends the post-initialization wait time to be twtp units of 4096 periods of the
                            * internal 400 kHz oscillator, instead of the default 256. (0: post-init time x256 clocks,
                            * 1: post-init time x4096 clocks). */
    uint8_t i2cret : 2;    /**< Number of I2C peripheral communication retries in case of NAK. A value of 0 means no
                            * retries, the polling is aborted immediately when a NAK is received. */
    uint8_t padding : 2;   /**< Not used. */
} npz_register_tcfgp_s;

/**  Internal ADC (VBAT)Threshold Over Value. */
typedef struct
{
    uint8_t throva : 5;  /**< Value of the ADC (internal) over threshold, defined as 5-bit unsigned. */
    uint8_t padding : 3; /**< Not used. */
} npz_register_throva1_s;

/**  External ADC (ADC_IN) Threshold Over Value. */
typedef struct
{
    uint8_t throva : 5;  /**< Value of the ADC (external) over threshold, defined as 5-bit unsigned. */
    uint8_t padding : 3; /**< Not used. */
} npz_register_throva2_s;

/**  Internal ADC (VBAT)Threshold Under Value. */
typedef struct
{
    uint8_t thruna : 5;  /**< Value of the ADC (internal) under threshold, defined as 5-bit unsigned. */
    uint8_t padding : 3; /**< Not used. */
} npz_register_thruna1_s;

/**  External ADC (ADC_IN) Threshold Under Value */
typedef struct
{
    uint8_t thruna : 5;  /**< Value of the ADC (external) under threshold, defined as 5-bit unsigned. */
    uint8_t padding : 3; /**< Not used. */
} npz_register_thruna2_s;

/** (Read) Peripheral Value. */
typedef struct
{
    uint8_t valp_l; /**< Last read value from peripheral, either 8-bit or 16-bit, matching the data type set in
                     * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
    uint8_t valp_h; /**< Last read value from peripheral, either 8-bit or 16-bit, matching the data type set in
                     * npz_register_modp_s (see npz_DTYPE in npz_register_modp_s). */
} npz_register_valp_s;

/** (Read) Internal ADC (VBAT) Value. */
typedef struct
{
    uint8_t adc_core; /**< Last read value From internal ADC channel (VBAT pin). */
} npz_register_adc_core_s;

/** (Read) External ADC (ADC_IN) Value. */
typedef struct
{
    uint8_t adc_ext; /**< Last read value from external ADC channel (ADC_IN pin). */
} npz_register_adc_ext_s;

/**  SRAM register value. */
typedef struct
{
    uint8_t value[128]; /**< SRAM value. */
} npz_register_sram_s;

/** Device status struct*/
typedef struct
{
	npz_register_sta1_s status1;
	npz_register_sta2_s status2;
} npz_status_s;

/** User configuration (Global parameters). */

/** Struct that holds configuration for ADC channels. */
typedef struct
{
    uint8_t over_threshold;    /**< Value of the ADC over threshold, defined as 5-bit unsigned. */
    uint8_t under_threshold;   /**< Value of the ADC under threshold, defined as 5-bit unsigned. */
    uint8_t wakeup_enable : 1; /**< Enables wake up on internal ADC channel trigger or external ADC channel trigger.
                                * (0: ADC trigger disabled, 1: ADC trigger enabled). */
} npz_adc_config_channels_s;

/** Struct that holds all registers that can be configured for each peripheral. */
typedef struct
{
    /** Peripheral Address Configuration (ADDRP). */
    npz_com_protocol_e communication_protocol; /**< Sets which communication protocol should be
                                                * used with each peripheral.(0: I2C, 1: SPI). */

    /** Peripheral Configuration(CFGP). */
    npz_power_mode_e power_mode;                 /**< Controls peripheral power mode. */
    npz_polling_mode_e polling_mode;             /**< Controls peripheral polling mode. */
    npz_power_switch_mode_e power_switch_mode;   /**< Sets the peripheral power switch mode (pins SW_LP*). */
    npz_interrupt_pin_mode_e interrupt_pin_mode; /**< Sets the peripheral interrupt pin mode (pins INT*),
                                                  * either as an input that can be used during polling,
                                                  * or as a trigger output. */

    /** Peripheral Mode Configuration (MODP). */
    npz_comparison_mode_e comparison_mode;   /**< Comparison mode for threshold values. */
    npz_data_type_e sensor_data_type;               /**< Data type of value read From peripheral. */
    npz_multibyte_e multi_byte_transfer_enable  : 1; /**< Transmit sequential read and write addresses as multi-byte transfers.
                                              * (0:Disable multi-byte transfers; 1: Enable multi-byte transfers). */
    npz_endianess_e swap_registers : 1;  /**< Swap high/low read registers of peripheral to match endianess
                                              * of the data. (0: Little endian; 1: Big endian). */

    /** Peripheral Polling Configuration (PERP). */
    uint16_t polling_period; /**< Polling period for peripheral. NOTE: Zero is not a valid value and can result
                              * in undefined behavior. */

    union
    {
        struct
        {
            uint8_t command_num;        /**< Number of write commands for peripheral initialization (NCMDP). */
            uint8_t sensor_address; /**< The 7-bit I2C address of the peripheral (ADDRP). */
            uint8_t reg_address_value;  /**< I2C register address of the peripheral that holds the value to be read
                                         * (RREGP). */
            uint8_t wake_on_nak : 1;    /**< Wake up on NAK (No Acknowledgment) (MODP).
                                         * (0: Continue on NAK, 1: Wake up on NAK). */
            uint8_t num_of_retries_on_nak : 2; /**< Number of I2C peripheral communication retries in case of NAK.
                                                * A value of 0 means no retries, the polling is aborted immediately
                                                * when a NAK is received (TCFGP). */
            uint8_t bytes_from_sram[40]; /**< bytes to send from the SRAM (NCMDP). */ /*TODO Test*/
        } i2c_cfg;
        struct
        {
            uint8_t bytes_from_sram_num;          /**< The number of bytes to send from the SRAM for initialization (NCMDP). */
            uint8_t bytes_from_sram_read_num; /**< The number of bytes to send from SRAM for value read (ADDRP). */
            uint8_t bytes_from_sram[40];          /**< The bytes to send from SRAM for initialization. */ /*TODO Test*/
            uint8_t bytes_from_sram_read[40];     /**< The bytes to send from SRAM for value read. */ /*TODO Test*/
            npz_spimod_e mode;                    /**< Sets the SPI mode if SPI communication is enabled (MODP). */
        } spi_cfg;
    };

    /** Peripheral Threshold Over configuration (THROVP). */
    uint16_t threshold_over; /**< Value of the peripheral over threshold, up to 16-bit. */

    /** Peripheral Threshold Under Value (THRUNP) */
    uint16_t threshold_under; /**< Value of the peripheral under threshold. */

    /** Peripheral To Wait Time before reading value (TWTP). */
    uint8_t time_to_wait; /**< Period of time to wait before and/or after peripheral initialization until the value
                           * is read, or until timeout while waiting for an interrupt assertion, defined in units of
                           * 256 or 4096 periods of the internal 400 kHz oscillator.The actual time to wait, and
                           * when to wait, depends on the value of the tcfgp register. */

    /** Peripheral To Wait Time configuration (TCFGP). */
    npz_pre_wait_time_e pre_wait_time;   /**< Pre-initialization wait time. */
    npz_post_wait_time_e post_wait_time; /**< Post-initialization wait time. */

} npz_peripheral_config_s;

/** Struct that holds the configuration of the npz device. */
typedef struct
{
    /** Power Control for peripherals (PSWCTL). */
    uint8_t power_switch_normal_mode_per1 : 1; /**< Controls the power switch output state for peripheral 1. */
    uint8_t power_switch_normal_mode_per2 : 1; /**< Controls the power switch output state for peripheral 2. */
    uint8_t power_switch_normal_mode_per3 : 1; /**< Controls the power switch output state for peripheral 3. */
    uint8_t power_switch_normal_mode_per4 : 1; /**< Controls the power switch output state for peripheral 4. */
    npz_host_power_mode_e host_power_mode;     /**< Sets the host power switch mode (SW_HP pin). */
    uint8_t power_switch_gate_boost : 1;       /**< Enables power switch gate boost for reduced Rds(on). Gate boost automatically
                                                *   turns on for VBAT lower than 1.7 V. (0: Normal operation, 1: Switch gate boost) */

    /** System Config 1 (SYSCFG1) for clock source and peripheral wake up configuration. */
    uint8_t wake_up_per1 : 1; /**< Enables peripheral 1 as a wake up source when triggered. (0: Peripheral does not
                               * wake up system, 1: Peripheral can wake up system). */
    uint8_t wake_up_per2 : 1; /**< Enables peripheral 2 as a wake up source when triggered. (0: Peripheral does not
                               * wake up system, 1: Peripheral can wake up system). */
    uint8_t wake_up_per3 : 1; /**< Enables peripheral 3 as a wake up source when triggered. (0: Peripheral does not
                               * wake up system, 1: Peripheral can wake up system). */
    uint8_t wake_up_per4 : 1; /**< Enables peripheral 4 as a wake up source when triggered. (0: Peripheral does not
                               * wake up system, 1: Peripheral can wake up system). */
    npz_wakeup_e wake_up_any_or_all : 1; /**< (wupmod) Controls if any wake up source trigger is sufficient to wake up
                                          * the system, or if all wake up source triggers are necessary to wake up.
                                          * (0: Wake up system on any trigger, 1: Wake up system on all triggers). */

    /** System Config 2 (SYSCFG2) for wakeup configuration. */
    npz_sclk_div_e system_clock_divider; /**< Controls system clock divider. */
    npz_sclk_sel_e system_clock_source;  /**< Controls system clock source between internal slow oscillator or
                                          * crystal oscillator. */

    /** System Config 3 (SYSCFG3) for pin, I2C pullup and clock out configuration. */
	npz_io_str_e 		io_strength;  	/**< IO pull-up strength configuration */
	npz_i2c_pull_sel_e 	i2c_pull_mode;	/**< IO pull-up mode */
	npz_spi_auto_e		spi_auto;		/**< SPI pin configuration */
    npz_xo_clkout_div_e xo_clock_out_sel;	/**< CLK_OUT pin configuration */

    /** Timeout (TOUT) for Host MCU wake up. */
    uint16_t global_timeout; /**< Global time out (TOUT) for Host MCU wake up.*/

    /** Interrupt Config (INTCFG) for interrupt pull-up resistors and pull-up strength configuration. */
    npz_int_pin_pull_e interrupt_pin_pull_up_pin1; /**< Interrupt Config for interrupt pull-up resistors and
                                                    * pull-up strength for peripheral 1. */
    npz_int_pin_pull_e interrupt_pin_pull_up_pin2; /**< Interrupt Config for interrupt pull-up resistors and
                                                    * pull-up strength for peripheral 2. */
    npz_int_pin_pull_e interrupt_pin_pull_up_pin3; /**< Interrupt Config for interrupt pull-up resistors and
                                                    * pull-up strength for peripheral 3. */
    npz_int_pin_pull_e interrupt_pin_pull_up_pin4; /**< Interrupt Config for interrupt pull-up resistors and
                                                    * pull-up strength for peripheral 4. */

    /** Peripheral configurations (Disabled if null). */
    npz_peripheral_config_s *peripherals[4];

    /** System Config 2 (SYSCFG2) register for ADC clock configurartion. */
    uint8_t adc_ext_sampling_enable : 1; /**< Enables external ADC input sampling (ADC_IN pin).
                                          * (0: Disable ADC_IN, 1: Enable ADC_IN). */
    npz_adc_clk_e adc_clock_sel;     /**< Controls ADC clock. */

    npz_adc_config_channels_s *adc_channels[2];

} npz_device_config_s;


/* Function Prototypes */

/**
 * @brief Writes the idle_rst struct to the IDLE_RST register.
 * @brief When set to OxFF, the device will enter idle mode, shutting down the host power and assuming control
 * of the I2C bus.
 * @brief When set to 0xA5, the device will soft reset.
 *
 *
 * @param [in] idle_rst_value that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_IDLE_RST(uint8_t idle_rst_value);

/**
 * @brief Reads the IDLE_RST register and stores it in the npz_register_idle_rst_s struct.
 *
 *
 * @param [out] idle_rst_value Pointer to store the read idle value.
 * @return npz_status_e Status
 */
npz_status_e npz_read_IDLE_RST(uint8_t *idle_rst_value);

/**
 * @brief Reads the ID register and stores it in the npz_register_id_s struct.
 *
 *
 * @param [out] id Pointer to id where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_ID(uint8_t *id);

/**
 * @brief Reads the STA1 register and writes it to the npz_register_sta1_s struct.
 *
 *
 * @param [out] sta1 Pointer to the status 1 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_STA1(npz_register_sta1_s *sta1);

/**
 * @brief Reads the STA2 register and writes it to the npz_register_sta2_s struct.
 *
 *
 * @param [out] sta2 Pointer to the status 2 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_STA2(npz_register_sta2_s *sta2);

/**
 * @brief Writes the pswctl struct to the PSWCTL register.
 *
 *
 * @param [in] pswctl Power switch control register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_PSWCTL(const npz_register_pswctl_s pswctl);

/**
 * @brief Reads the PSWCTL register and writes it to the npz_register_pswctl_s struct.
 *
 *
 * @param [out] pswctl Pointer to power switch register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_PSWCTL(npz_register_pswctl_s *pswctl);

/**
 * @brief Writes the syscfg1 struct to the SYSCFG1 register.
 *
 *
 * @param [in] syscfg1 System config 1 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_SYSCFG1(const npz_register_syscfg1_s syscfg1);

/**
 * @brief Reads the SYSCFG1 register and writes it to npz_register_syscfg1_s struct.
 *
 *
 * @param [out] syscfg1 Pointer to system config 1 where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_SYSCFG1(npz_register_syscfg1_s *syscfg1);

/**
 * @brief Writes the syscfg2 struct to the SYSCFG2 register.
 *
 *
 * @param [in] syscfg2 System config 2 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_SYSCFG2(const npz_register_syscfg2_s syscfg2);

/**
 * @brief Reads the SYSCFG2 register and writes it to npz_register_syscfg2_s struct.
 *
 *
 * @param [out] syscfg2 Pointer to System config 2 register where the value will be stored..
 * @return npz_status_e Status
 */
npz_status_e npz_read_SYSCFG2(npz_register_syscfg2_s *syscfg2);

/**
 * @brief Writes the syscfg3 struct to the SYSCFG3 register.
 *
 *
 * @param [in] syscfg3 System config 3 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_SYSCFG3(const npz_register_syscfg3_s syscfg3);

/**
 * @brief Reads the syscfg3 register and writes it to the npz_register_syscfg3_s struct.
 *
 *
 * @param [out] syscfg3 Pointer to System config 3 register where the value will be stored..
 * @return npz_status_e Status
 */
npz_status_e npz_read_SYSCFG3(npz_register_syscfg3_s *syscfg3);

/**
 * @brief Writes the tout struct to the TOUT_L and TOUT_H registers.
 *
 *
 * @param [in] tout Global Timeout register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_TOUT(const npz_register_tout_s tout);

/**
 * @brief Reads the TOUT_L and TOUT_H registers and writes them to the npz_register_tout_s struct.
 *
 *
 * @param [out] tout Pointer to Global Timeout register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_TOUT(npz_register_tout_s *tout);

/**
 * @brief Writes the intcfg struct to the INTCFG register.
 *
 *
 * @param [in] intcfg Interrupt pin config register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_INTCFG(const npz_register_intcfg_s intcfg);

/**
 * @brief Reads the INTCFG register and writes it to the npz_register_intcfg_s struct.
 *
 *
 * @param [out] intcfg Pointer to Interrupt pin config register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_INTCFG(npz_register_intcfg_s *intcfg);

/**
 * @brief Writes the throva1 struct to the THROVA1 register.
 *
 *
 * @param [in] throva1 Threshold Over ADC 1 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THROVA1(const npz_register_throva1_s throva1);

/**
 * @brief Reads the THROVA1 register and writes it to the npz_register_throva1_s struct.
 *
 *
 * @param [out] throva1 Pointer to Threshold Over ADC 1 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THROVA1(npz_register_throva1_s *throva1);

/**
 * @brief Writes the throva2 struct to the THROVA2 register.
 *
 *
 * @param [in] throva2 Threshold Over ADC 2 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THROVA2(const npz_register_throva2_s throva2);

/**
 * @brief Reads the THROVA2 register and writes it to npz_register_throva2_s struct.
 *
 *
 * @param [out] throva2 Pointer to Thershold Over ADC 2 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THROVA2(npz_register_throva2_s *throva2);

/**
 * @brief Writes the thruna1 struct to the TRHUNA1 register.
 *
 *
 * @param [in] thruna1 Threshold Under ADC 1 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THRUNA1(const npz_register_thruna1_s thruna1);

/**
 * @brief Reads the THRUNA1 register and writes it to npz_register_thruna1_s struct.
 *
 *
 *
 * @param [out] thruna1 Pointer to Threshold Under ADC 1 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THRUNA1(npz_register_thruna1_s *thruna1);

/**
 * @brief Writes the thruna2 struct to the TRHUNA2 register.
 *
 *
 * @param [out] thruna2 Threshold Under ADC 2 register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THRUNA2(const npz_register_thruna2_s thruna2);

/**
 * @brief Reads the THRUNA2 register and writes it to the npz_register_thruna2_s struct.
 *
 *
 * @param [out] thruna2 Pointer to Threshold Under ADC 2 register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THRUNA2(npz_register_thruna2_s *thruna2);

/**
 * @brief Reads the last value from the internal ADC channel (VBAT) and stores it in the npz_register_adc_core_s struct.
 *
 *
 * @param [out] adc_core Pointer to the ADC Core register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_ADC_CORE(npz_register_adc_core_s *adc_core);

/**
 * @brief Reads the last value from the external ADC channel (ADC_IN) and stores it in the npz_register_adc_ext_s
 * struct.
 *
 *
 * @param [out] adc_ext Pointer to the ADC External register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_ADC_EXT(npz_register_adc_ext_s *adc_ext);

/**
 * @brief Write one byte to one register in SRAM.
 *
 *
 * @param [in] sram_reg Register address in SRAM to write to.
 * @param [in] sram SRAM register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_SRAM(const uint8_t sram_reg, const uint8_t sram);

/**
 * @brief Reads one SRAM register and writes it to the npz_register_sram_s struct.
 *
 *
 * @param [in] sram_reg Register address in SRAM to read a byte from.
 * @param [out] sram Pointer to the SRAM register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_SRAM(const uint8_t sram_reg, npz_register_sram_s *sram);

/**
 * @brief Writes the cfgp struct to the CFGP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] cfgp Config Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_CFGP(const npz_psw_e sw, const npz_register_cfgp_s cfgp);

/**
 * @brief Reads the CFGP register that is connected to the low power switch and writes it to the npz_register_cfgp_s
 * struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] cfgp Pointer to the Config Peripheral where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_CFGP(const npz_psw_e sw, npz_register_cfgp_s *cfgp);

/**
 * @brief Writes the modp struct to the MODP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] modp Mode Peripheral register that holds value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_MODP(const npz_psw_e sw, const npz_register_modp_s modp);

/**
 * @brief Reads the NODP register that is connected to the low power switch and writes it to the npz_register_modp_s
 * struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] modp Pointer to Mode Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_MODP(const npz_psw_e sw, npz_register_modp_s *modp);

/**
 * @brief Writes the perp struct to the PERP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] perp Polling Period register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_PERP(const npz_psw_e sw, const npz_register_perp_s perp);

/**
 * @brief Reads the PERP register that is connected to the low power switch and writes it to npz_register_perp_s
 * struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] perp Pointer to Polling Period register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_PERP(const npz_psw_e sw, npz_register_perp_s *perp);

/**
 * @brief Writes the ncmdp struct to the NCMDP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] ncmdp Number Of Commands register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_NCMDP(const npz_psw_e sw, const npz_register_ncmdp_s ncmdp);

/**
 * @brief Reads the NCMDP register that is connected to the low power switch and writes it to
 * npz_register_ncmdp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] ncmdp Pointer to Number Of Commands register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_NCMDP(const npz_psw_e sw, npz_register_ncmdp_s *ncmdp);

/**
 * @brief Writes the addrp struct to the ADDRP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] addrp Address Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_ADDRP(const npz_psw_e sw, const npz_register_addrp_s addrp);

/**
 * @brief Reads the ADDRP register that is connected to the low power switch and writes it to
 * npz_register_addrp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] addrp Pointer to Address Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_ADDRP(const npz_psw_e sw, npz_register_addrp_s *addrp);

/**
 * @brief Writes the rregp struct to the RREGP register that is connected to the low power switch.
 *
 *
 * @param [in] sw Low power switch indicates which peripheral will be written.
 * @param [in] rregp Read Register Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_RREGP(const npz_psw_e sw, const npz_register_rregp_s rregp);

/**
 * @brief Reads the RREGP register that is connected to the low power switch and writes it to the
 * npz_register_rregp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] rregp Pointer to Read Register Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_RREGP(const npz_psw_e sw, npz_register_rregp_s *rregp);

/**
 * @brief Writes the throvp struct to the THROVP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] throvp Threshold Over Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THROVP(const npz_psw_e sw, const npz_register_throvp_s throvp);

/**
 * @brief Reads the THROVP register that is connected to the low power switch and writes it to
 * npz_register_throvp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] throvp Pointer to Threshold Over Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THROVP(const npz_psw_e sw, npz_register_throvp_s *throvp);

/**
 * @brief Writes the thrunp struct to the THRUNP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral that will be written.
 * @param [in] thrunp Threshold Under Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_THRUNP(const npz_psw_e sw, const npz_register_thrunp_s thrunp);

/**
 * @brief Reads the THRUNP register that is connected to the low power switch and writes it to
 * npz_register_thrunp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] thrunp Pointer to Threshold Under Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_THRUNP(const npz_psw_e sw, npz_register_thrunp_s *thrunp);

/**
 * @brief Writes the twtp struct to the TWTP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] twtp Time To Wait Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_TWTP(const npz_psw_e sw, const npz_register_twtp_s twtp);

/**
 * @brief Reads the TWTP register that is connected to the low power switch and writes it to the npz_register_twtp_s
 * struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] twtp Pointer to Time To Wait Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_TWTP(const npz_psw_e sw, npz_register_twtp_s *twtp);

/**
 * @brief Writes the tcfgp struct to the TCFGP register that is connected to the low power switch.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be written.
 * @param [in] tcfgp Time To Wait Config Peripheral register that holds the value to be written.
 * @return npz_status_e Status
 */
npz_status_e npz_write_TCFGP(const npz_psw_e sw, const npz_register_tcfgp_s tcfgp);

/**
 * @brief Reads the TCFGP register that is connected to the low power switch and writes it to the
 * npz_register_tcfgp_s struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] tcfgp Pointer to Time To Wait Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_TCFGP(const npz_psw_e sw, npz_register_tcfgp_s *tcfgp);

/**
 * @brief Reads the VALP register that is connected to the low power switch and writes it to the npz_register_valp_s
 * struct.
 *
 *
 * @param [in] sw The low power switch indicates which peripheral will be read.
 * @param [out] valp Pointer to Value Peripheral register where the value will be stored.
 * @return npz_status_e Status
 */
npz_status_e npz_read_VALP(const npz_psw_e sw, npz_register_valp_s *valp);

/**
 * @brief Generic function to read a device register over I2C.
 *
 * @param register_address The address of the register to read from.
 * @param buffer Pointer to the buffer where the read data will be stored.
 * @param size The size of the data to read.
 * @return OK if the read operation is successful, ERR otherwise.
 */
npz_status_e npz_read_register(uint8_t register_address, void *buffer, size_t size);

#endif /* __NPZ_H */

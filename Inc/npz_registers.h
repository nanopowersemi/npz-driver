/**
 * @file npz_registers.h
 *
 *  @brief Header file for the registers of the nPZero IC.
 */

#ifndef __NPZ_REGISTERS_H
#define __NPZ_REGISTERS_H

#define REG_IDLE_RST           0x00 /**< Idle/Reset Register*/
#define REG_ID                  0x01 /**< Chip ID Register*/
#define REG_STA1                0x02 /**< Status 1 Register*/
#define REG_STA2                0x03 /**< Status 2 Register*/
#define REG_PSWCTL              0x04 /**< Power Switch Control Register*/
#define REG_SYSCFG1             0x05 /**< System Config 1 Register */
#define REG_SYSCFG2             0x06 /**< System Config 2 Register*/
#define REG_SYSCFG3             0x07 /**< System Config 3 Register*/
#define REG_TOUT_L              0x08 /**< Global Timeout Lower Register*/
#define REG_TOUT_H              0x09 /**< Global Timeout Higher Register*/
#define REG_INTCFG              0x0A /**< Interrupt Pin Config Register*/

#define REG_CFGP1               0x10 /**< Config Peripheral 1 Register*/
#define REG_MODP1               0x11 /**< Mode Peripheral 1 Register*/
#define REG_PERP1_L             0x12 /**< Polling Period Peripheral 1 Lower Register*/
#define REG_PERP1_H             0x13 /**< Polling Period Peripheral 1 Higher Register*/
#define REG_NCMDP1              0x14 /**< Number of init Commands Peripheral 1 Register*/
#define REG_ADDRP1              0x15 /**< I2C Address Peripheral 1 or Nbr of Bytes for SPI Read Peripheral 1*/
#define REG_RREGP1              0x16 /**< I2C Read Register Peripheral 1*/
#define REG_THROVP1_L           0x17 /**< Over Threshold Lower Peripheral 1 Register*/
#define REG_THROVP1_H           0x18 /**< Over Threshold Higher Peripheral 1 Register*/
#define REG_THRUNP1_L           0x19 /**< Under Threshold Lower Peripheral 1 Register*/
#define REG_THRUNP1_H           0x1A /**< Under Threshold Higher Peripheral 1 Register*/
#define REG_TWTP1               0x1B /**< Time to Wait Peripheral 1 Register*/
#define REG_TCFGP1              0x1C /**< Time to Wait Config Peripheral 1 Register*/

#define REG_CFGP2               0x1D /**< Config Peripheral 2 Register*/
#define REG_MODP2               0x1E /**< Mode Peripheral 2 Register*/
#define REG_PERP2_L             0x1F /**< Polling Period Peripheral 2 Lower Register*/
#define REG_PERP2_H             0x20 /**< Polling Period Peripheral 2 Higher Register*/
#define REG_NCMDP2              0x21 /**< Number of init Commands Peripheral 2 Register*/
#define REG_ADDRP2              0x22 /**< I2C Address Peripheral 2 or Nbr of Bytes for SPI Read Peripheral 2*/
#define REG_RREGP2              0x23 /**< I2C Read Register Peripheral 2*/
#define REG_THROVP2_L           0x24 /**< Over Threshold Lower Peripheral 2 Register*/
#define REG_THROVP2_H           0x25 /**< Over Threshold Higher Peripheral 2 Register*/
#define REG_THRUNP2_L           0x26 /**< Under Threshold Lower Peripheral 2 Register*/
#define REG_THRUNP2_H           0x27 /**< Under Threshold Higher Peripheral 2 Register*/
#define REG_TWTP2               0x28 /**< Time to Wait Peripheral 2 Register*/
#define REG_TCFGP2              0x29 /**< Time to Wait Config Peripheral 2 Register*/

#define REG_CFGP3               0x2A /**< Config Peripheral 3 Register*/
#define REG_MODP3               0x2B /**< Mode Peripheral 3 Register*/
#define REG_PERP3_L             0x2C /**< Polling Period Peripheral 3 Lower Register*/
#define REG_PERP3_H             0x2D /**< Polling Period Peripheral 3 Higher Register*/
#define REG_NCMDP3              0x2E /**< Number of init Commands Peripheral 3 Register*/
#define REG_ADDRP3              0x2F /**< I2C Address Peripheral 3 or Nbr of Bytes for SPI Read Peripheral 3*/
#define REG_RREGP3              0x30 /**< I2C Read Register Peripheral 3*/
#define REG_THROVP3_L           0x31 /**< Over Threshold Lower Peripheral 3 Register*/
#define REG_THROVP3_H           0x32 /**< Over Threshold Higher Peripheral 3 Register*/
#define REG_THRUNP3_L           0x33 /**< Under Threshold Lower Peripheral 3 Register*/
#define REG_THRUNP3_H           0x34 /**< Under Threshold Higher Peripheral 3 Register*/
#define REG_TWTP3               0x35 /**< Time to Wait Peripheral 3 Register*/
#define REG_TCFGP3              0x36 /**< Time to Wait Config Peripheral 3 Register*/

#define REG_CFGP4               0x37 /**< Config Peripheral 4 Register*/
#define REG_MODP4               0x38 /**< Mode Peripheral 4 Register*/
#define REG_PERP4_L             0x39 /**< Polling Period Peripheral 4 Lower Register*/
#define REG_PERP4_H             0x3A /**< Polling Period Peripheral 4 Higher Register*/
#define REG_NCMDP4              0x3B /**< Number of init Commands Peripheral 4 Register*/
#define REG_ADDRP4              0x3C /**< I2C Address Peripheral 4 or Nbr of Bytes for SPI Read Peripheral 4*/
#define REG_RREGP4              0x3D /**< I2C Read Register Peripheral 4*/
#define REG_THROVP4_L           0x3E /**< Over Threshold Lower Peripheral 4 Register*/
#define REG_THROVP4_H           0x3F /**< Over Threshold Higher Peripheral 4 Register*/
#define REG_THRUNP4_L           0x40 /**< Under Threshold Lower Peripheral 4 Register*/
#define REG_THRUNP4_H           0x41 /**< Under Threshold Higher Peripheral 4 Register*/
#define REG_TWTP4               0x42 /**< Time to Wait Peripheral 4 Register*/
#define REG_TCFGP4              0x43 /**< Time to Wait Config Peripheral 4 Register*/

#define REG_THROVA1             0x44 /**< Over Threshold Internal ADC Register*/
#define REG_THRUNA1             0x45 /**< Under Threshold Internal ADC Register*/
#define REG_THROVA2             0x46 /**< Over Thershold External ADC Register*/
#define REG_THRUNA2             0x47 /**< Under Threshold External ADC Register*/
#define REG_VALP1_L             0x50 /**< Value Lower Peripheral 1 Register*/
#define REG_VALP1_H             0x51 /**< Value Higher Peripheral 1 Register*/
#define REG_VALP2_L             0x52 /**< Value Lower Peripheral 2 Register*/
#define REG_VALP2_H             0x53 /**< Value Higher Peripheral 2 Register*/
#define REG_VALP3_L             0x54 /**< Value Lower Peripheral 3 Register*/
#define REG_VALP3_H             0x55 /**< Value Higher Peripheral 3 Register*/
#define REG_VALP4_L             0x56 /**< Value Lower Peripheral 4 Register*/
#define REG_VALP4_H             0x57 /**< Value Higher Peripheral 4 Register*/

#define REG_ADC_CORE            0x58 /**< Value Internal ADC Register*/
#define REG_ADC_EXT             0x59 /**< Value External ADC Register*/

#define REG_SRAM_START          0x80 /**< SRAM Base Address Register*/
#define REG_SRAM_END            0xFF /**< SRAM End Address Register*/
#define SRAM_REG_SIZE           0x40 /**< SRAM Register Size*/

#endif /* __NPZ_REGISTERS_H */

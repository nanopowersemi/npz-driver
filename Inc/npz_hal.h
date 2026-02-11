/**
 * @file npz_hal.h
 * 
 * @brief Header file for nPZero Hardware Abstraction Layer (HAL)
 *
 * This file contains the hardware-specific functions and configurations for
 * the I2C communication interface. Users are required to customize these
 * functions according to their target hardware and MCU setup.
 *
 * The functions in this file serve as an abstraction layer between the 
 * driver and the hardware-specific I2C implementation on the target system.
 * Users should modify these functions to match the I2C peripheral and
 * communication protocol used in their system.
 *
 * @note This file provides placeholder functions and configurations that need
 * to be adapted to the specific hardware and MCU setup. The user is expected to modify
 * the function declaration in the npz_hal.c to fit their setup.
 *
 * @warning Incorrect configuration or improper implementation of these functions
 * may result in I2C communication failures or unexpected behavior.
 */

#ifndef __NPZ_HAL_H
#define __NPZ_HAL_H

/** @cond */
#include <stdint.h>
/** @endcond */

#define NPZ_I2C_ADDRESS			0x7a  // 0x3D nPZero I2C address shifted left by 1 bit
#define I2C_TRANSMISSION_TIMEOUT_MS 1300

/** Enumerations. */

/** Operation value. */
typedef enum
{
    OK = 0x00,            /**< OK. */
    ERR = 0x01,           /**< Error. */
    INVALID_PARAM = 0x02, /**< Invalid Parameter. */
} npz_status_e;

/**
 * @brief Function to read nPZero registers over I2C.
 *
 * @note The device's 7-bit address value in the datasheet must be shifted to the left before calling this function.
 * @note Function is blocking.
 * @param [in] slave_address I2C address for the slave.
 * @param [out] pData Pointer to the data buffer where read data will be stored.
 * @param [in] size Size of the data to be received.
 * @param [in] timeout Timeout before the transmission is aborted.
 * @return npz_status_e Status
 */
npz_status_e npz_hal_read(uint8_t slave_address, uint8_t slave_register,
		uint8_t *pData, uint16_t size, uint32_t timeout);

/**
 * @brief Function to write nPZero registers over I2C.
 *
 * @note The device's 7-bit address value in datasheet must be shifted to the left before calling this function.
 * @note Function is blocking.
 * @param [in] slave_address I2C address for the slave.
 * @param [in] pData Pointer to the data buffer to write.
 * @param [in] size Size of the data buffer to be sent.
 * @param [in] timeout Timeout before transmission is aborted.
 * @return npz_status_e Status
 */
npz_status_e npz_hal_write(uint8_t slave_address, uint8_t *pData, uint16_t size,
		uint32_t timeout);

/**
 * @brief Function to initialize hardware dependent I2C interface.
 *
 * @return npz_status_e Status
 */
npz_status_e npz_hal_init(void);

#endif /* __NPZ_HAL_H */

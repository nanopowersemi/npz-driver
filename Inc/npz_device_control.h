/**
 * @file npz_device_control.h
 * @brief Header file for functions to read and handle status registers (STA1 and STA2), 
 * put the nPZero device in idle mode, perform a software reset, and set a wake-up timeout.
 *
 * The registers in STA1 and STA2 can be read to determine the wake-up reason.
 */

#ifndef __NPZ_DEVICE_CONTROL_H
#define __NPZ_DEVICE_CONTROL_H

/** @cond */
#include <stdbool.h>
#include <stdint.h>

#include "npz.h"
/** @endcond */

/**
 * @brief Reads the last read value from a specified peripheral.
 *
 * @param [in]  psw_lp          The low power switch indicates which peripheral value should be read.
 * @param [in]  index           Index of the peripheral.
 * @param [out] peripheral_value Pointer to where the value read from the peripheral should be stored.
 *
 * @return True if the peripheral value was successfully read, false otherwise.
 */
bool npz_device_read_peripheral_value(npz_psw_e psw_lp, int index, int *peripheral_value);

/**
 * @brief Handles the internal ADC and retrieves the current value.
 *
 * @return True if the internal ADC was successfully handled, false otherwise.
 */
bool  npz_device_handle_adc_internal(void);

/**
 * @brief Handles the external ADC and retrieves the current value.
 *
 * @return True if the external ADC was successfully handled, false otherwise.
 */
bool npz_device_handle_adc_external(void);

/**
 * @brief Put the device into idle mode.
 *
 */
void npz_device_go_to_idle(void);

/**
 * @brief Reset the device by software.
 *
 */
void npz_device_soft_reset(void);

/**
 * @brief Change the nPZero device configuration.
 *
 * @param [in] npz_device_config_s Pointer to the device configuration structure.
 */
void npz_device_configure(npz_device_config_s *device_config);

#endif /* __NPZ_DEVICE_CONTROL_H */

/**
 * @file npz_logs.c
 * @brief This file contains functions for reading and logging configurations of the nPZero device, including global
 * settings, peripheral configurations, and ADC setup. The logging functions are designed to ensure that all configurations
 * are read correctly and presented in a standardized format for easier debugging and verification.
 */

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../Inc/npz_logs.h"
#include "../Inc/npz_registers.h"

/*****************************************************************************
 * Defines
 *****************************************************************************/

/*****************************************************************************
 * Data
 *****************************************************************************/

// Array to hold indices of configured peripherals
static int m_configured_indices[4] = {-1, -1, -1, -1}; /**< Initialize with invalid indices. */
static int m_configured_count = 0;

// Define a struct to hold register address and description
typedef struct
{
    uint8_t reg_address;  // Register address
    const char *reg_name; // Register name
} register_info_t;


static const register_info_t global_register_info[] = {
    {REG_IDLE_RST, "IDLE_RST"}, {REG_ID, "ID"},           {REG_PSWCTL, "PSWCTL"},
	{REG_SYSCFG1, "SYSCFG1"},     {REG_SYSCFG2, "SYSCFG2"},     {REG_SYSCFG3, "SYSCFG3"},
    {REG_TOUT_L, "TOUT_L"},     {REG_TOUT_H, "TOUT_H"},       {REG_INTCFG, "INTCFG"}
};


// An array to hold all register information for peripherals
static const register_info_t peripheral_registers[][13] = {
    // Peripheral 1
    {
        {REG_CFGP1, "CFGP1"},         {REG_MODP1, "MODP1"},         {REG_PERP1_L, "PERP1_L"},     {REG_PERP1_H, "PERP1_H"},
        {REG_NCMDP1, "NCMDP1"},       {REG_ADDRP1, "ADDRP1"},       {REG_RREGP1, "RREGP1"},       {REG_THRUNP1_L, "THRUNP1_L"},
        {REG_THRUNP1_H, "THRUNP1_H"}, {REG_THROVP1_L, "THROVP1_L"}, {REG_THROVP1_H, "THROVP1_H"}, {REG_TWTP1, "TWTP1"},
        {REG_TCFGP1, "TCFGP1"}
    },
    // Peripheral 2
    {
        {REG_CFGP2, "CFGP2"},         {REG_MODP2, "MODP2"},         {REG_PERP2_L, "PERP2_L"},     {REG_PERP2_H, "PERP2_H"},
        {REG_NCMDP2, "NCMDP2"},       {REG_ADDRP2, "ADDRP2"},       {REG_RREGP2, "RREGP2"},       {REG_THRUNP2_L, "THRUNP2_L"},
        {REG_THRUNP2_H, "THRUNP2_H"}, {REG_THROVP2_L, "THROVP2_L"}, {REG_THROVP2_H, "THROVP2_H"}, {REG_TWTP2, "TWTP2"},
        {REG_TCFGP2, "TCFGP2"}
    },
    // Peripheral 3
    {
        {REG_CFGP3, "CFGP3"},         {REG_MODP3, "MODP3"},         {REG_PERP3_L, "PERP3_L"},     {REG_PERP3_H, "PERP3_H"},
        {REG_NCMDP3, "NCMDP3"},       {REG_ADDRP3, "ADDRP3"},       {REG_RREGP3, "RREGP3"},       {REG_THRUNP3_L, "THRUNP3_L"},
        {REG_THRUNP3_H, "THRUNP3_H"}, {REG_THROVP3_L, "THROVP3_L"}, {REG_THROVP3_H, "THROVP3_H"}, {REG_TWTP3, "TWTP3"},
        {REG_TCFGP3, "TCFGP3"}
    },
    // Peripheral 4
    {
        {REG_CFGP4, "CFGP4"},         {REG_MODP4, "MODP4"},         {REG_PERP4_L, "PERP4_L"},     {REG_PERP4_H, "PERP4_H"},
        {REG_NCMDP4, "NCMDP4"},       {REG_ADDRP4, "ADDRP4"},       {REG_RREGP4, "RREGP4"},       {REG_THRUNP4_L, "THRUNP4_L"},
        {REG_THRUNP4_H, "THRUNP4_H"}, {REG_THROVP4_L, "THROVP4_L"}, {REG_THROVP4_H, "THROVP4_H"}, {REG_TWTP4, "TWTP4"},
        {REG_TCFGP4, "TCFGP4"}
    }
};

/*****************************************************************************
 * Private Methods
 *****************************************************************************/

// Function to convert a byte to a binary string (replace with your actual implementation)
static char *char_to_binary_string(uint8_t byte, char *buffer)
{
    for (int i = 7; i >= 0; i--)
    {
        buffer[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    buffer[8] = '\0'; // Null-terminate the string
    return buffer;
}


static void log_register_data(const char *register_name, uint8_t register_address, uint8_t *values, int count)
{
    char binary_str[9];         // Buffer to hold binary strings for each register value
    char combined_bin[17] = ""; // Buffer to hold combined binary strings for display, limited to 16 bits
    char combined_hex[7] = "";  // Buffer to hold 4-character hex strings for display with "0x" prefix

    // Combine binary values in little-endian order (LSB first) and hex representation
    for (int i = 0; i < count && i < 2; i++)
    { // Limit to 2 bytes for 16-bit display
        char_to_binary_string(values[i], binary_str);
        strcat(combined_bin, binary_str);
    }

    // Format HEX output as 4 characters long with "0x" prefix, showing LSB first

    if (count == 1)
    {
        sprintf(combined_hex, "0x%02X ", values[0]); // Pad with 00 if only one byte
    }
    else
    {
        sprintf(combined_hex, "0x00 "); // Pad with 0000 if no bytes
    }

    combined_bin[16] = '\0'; // Ensure the binary string is exactly 16 bits

    // Print the register data in the specified format with proper alignment
    printf("[  %02X   | %-14s | %-10s | %-4s ] \r\n", register_address, register_name, combined_bin, combined_hex);
}

static bool read_peripherals(npz_device_config_s *device_config)
{
    npz_psw_e switches[4] = {PSW_LP1, PSW_LP2, PSW_LP3, PSW_LP4}; // Corresponding switches

    // Iterate through the peripherals
    for (int j = 0; j < m_configured_count; j++)
    {
        npz_register_cfgp_s cfgp = {0};
        npz_register_modp_s modp = {0};
        npz_register_perp_s perp = {0};
        npz_register_ncmdp_s ncmdp = {0};
        npz_register_twtp_s twtp = {0};
        npz_register_tcfgp_s tcfgp = {0};

        int i = m_configured_indices[j]; // Get the index of the configured peripheral

        // Log the header for the table
        printf("----------------------------------------------\r\n");
        printf("         Read peripheral %d registers       \r\n", i + 1);
        printf("----------------------------------------------\r\n");
        printf("[  ADDR |    REGISTER    |     BIN    | HEX  ]\r\n");
        printf("----------------------------------------------\r\n");

        // Read and log CFGP register
        if (npz_read_CFGP(switches[i], &cfgp) == OK)
        {
            log_register_data(peripheral_registers[i][0].reg_name, peripheral_registers[i][0].reg_address,
                              (uint8_t *) &cfgp, sizeof(cfgp));
        }
        else
        {
            printf("Failed to read CFGP register for peripheral %d \r\n", i + 1);
            return false;
        }

        // Read and log MODP register
        if (npz_read_MODP(switches[i], &modp) == OK)
        {
            log_register_data(peripheral_registers[i][1].reg_name, peripheral_registers[i][1].reg_address,
                              (uint8_t *) &modp, sizeof(modp));
        }
        else
        {
            printf("Failed to read MODP register for peripheral %d \r\n", i + 1);
            return false;
        }

        // Read and log PERP register (split into low and high bytes)
        if (npz_read_PERP(switches[i], &perp) == OK)
        {
            log_register_data(peripheral_registers[i][2].reg_name, peripheral_registers[i][2].reg_address,
                              (uint8_t *) &perp.perp_l, 1);
            log_register_data(peripheral_registers[i][3].reg_name, peripheral_registers[i][3].reg_address,
                              (uint8_t *) &perp.perp_h, 1);
        }
        else
        {
            printf("Failed to read PERP register for peripheral %d \r\n", i + 1);
            return false;
        }

        // Read and log NCMDP register
        if (npz_read_NCMDP(switches[i], &ncmdp) == OK)
        {
            log_register_data(peripheral_registers[i][4].reg_name, peripheral_registers[i][4].reg_address,
                              (uint8_t *) &ncmdp, sizeof(ncmdp));
        }
        else
        {
            printf("Failed to read NCMDP register for peripheral %d \r\n", i + 1);
            return false;
        }

        if ((device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_WAIT_INTERRUPT))
        {
            npz_register_addrp_s addrp = {0};

            // Read and log ADDRP register
            if (npz_read_ADDRP(switches[i], &addrp) == OK)
            {
                log_register_data(peripheral_registers[i][5].reg_name, peripheral_registers[i][5].reg_address,
                                  (uint8_t *) &addrp, sizeof(addrp));
            }
            else
            {
                printf("Failed to read ADDRP register for peripheral %d \r\n", i + 1);
                return false;
            }
        }

        if ((device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_READ_COMPARE_THRESHOLD) ||
            (device_config->peripherals[i]->polling_mode == POLLING_MODE_PERIODIC_WAIT_INTERRUPT_COMPARE_THRESHOLD))
        {
            npz_register_rregp_s rregp = {0};
            npz_register_thrunp_s thrunp = {0};
            npz_register_throvp_s throvp = {0};

            // Read and log RREGP register
            if (npz_read_RREGP(switches[i], &rregp) == OK)
            {
                log_register_data(peripheral_registers[i][6].reg_name, peripheral_registers[i][6].reg_address,
                                  (uint8_t *) &rregp, sizeof(rregp));
            }
            else
            {
                printf("Failed to read RREGP register for peripheral %d \r\n", i + 1);
                return false;
            }

            // Read and log THRUNP register (split into low and high bytes)
            if (npz_read_THRUNP(switches[i], &thrunp) == OK)
            {
                log_register_data(peripheral_registers[i][7].reg_name, peripheral_registers[i][7].reg_address,
                                  (uint8_t *) &thrunp.thrunp_l, 1);
                log_register_data(peripheral_registers[i][8].reg_name, peripheral_registers[i][8].reg_address,
                                  (uint8_t *) &thrunp.thrunp_h, 1);
            }
            else
            {
                printf("Failed to read THRUNP register for peripheral %d \r\n", i + 1);
                return false;
            }

            // Read and log THROVP register (split into low and high bytes)
            if (npz_read_THROVP(switches[i], &throvp) == OK)
            {
                log_register_data(peripheral_registers[i][9].reg_name, peripheral_registers[i][9].reg_address,
                                  (uint8_t *) &throvp.throvp_l, 1);
                log_register_data(peripheral_registers[i][10].reg_name, peripheral_registers[i][10].reg_address,
                                  (uint8_t *) &throvp.throvp_h, 1);
            }
            else
            {
                printf("Failed to read THROVP register for peripheral %d \r\n", i + 1);
                return false;
            }
        }

        // Read and log TWTP register
        if (npz_read_TWTP(switches[i], &twtp) == OK)
        {
            log_register_data(peripheral_registers[i][11].reg_name, peripheral_registers[i][11].reg_address,
                              (uint8_t *) &twtp, sizeof(twtp));
        }
        else
        {
            printf("Failed to read TWTP register for peripheral %d \r\n", i + 1);
            return false;
        }

        // Read and log TCFGP register
        if (npz_read_TCFGP(switches[i], &tcfgp) == OK)
        {
            log_register_data(peripheral_registers[i][12].reg_name, peripheral_registers[i][12].reg_address,
                              (uint8_t *) &tcfgp, sizeof(tcfgp));
        }
        else
        {
            printf("Failed to read TCFGP register for peripheral %d \r\n", i + 1);
            return false;
        }

    }

    return true;
}

static bool adc_config_read(npz_device_config_s *device_config)
{
    if (device_config->adc_channels[0]->wakeup_enable == 1)
    {
        printf("----------------------------------------------\r\n");
        printf("          Read internal adc registers         \r\n");
        printf("----------------------------------------------\r\n");
        printf("[  ADDR |    REGISTER    |     BIN    | HEX  ]\r\n");
        printf("----------------------------------------------\r\n");

        npz_register_throva1_s throva1 = {0};
        npz_register_thruna1_s thruna1 = {0};

        if (npz_read_THROVA1(&throva1) != OK)
        {
            printf("Failed to read THROVA1 register\r\n");
            return false;
        }

        // Log internal ADC over threshold register
        log_register_data("THROVA1", REG_THROVA1, (uint8_t *) &throva1, sizeof(throva1));

        if (npz_read_THRUNA1(&thruna1) != OK)
        {
            printf("Failed to read THRUNA1 register\r\n");
            return false;
        }

        // Log internal ADC under threshold register
        log_register_data("THRUNA1", REG_THRUNA1, (uint8_t *) &thruna1, sizeof(thruna1));
        printf("----------------------------------------------\r\n");
    }

    if (device_config->adc_channels[1]->wakeup_enable == 1 && device_config->adc_ext_sampling_enable == 1)
    {
        printf("----------------------------------------------\r\n");
        printf("          Read external adc registers          \r\n");
        printf("----------------------------------------------\r\n");
        printf("[  ADDR |    REGISTER    |     BIN    | HEX  ]\r\n");
        printf("----------------------------------------------\r\n");

        npz_register_throva2_s throva2 = {0};
        npz_register_thruna2_s thruna2 = {0};

        if (npz_read_THROVA2(&throva2) != OK)
        {
            printf("Failed to read THROVA2 register\r\n");
            return false;
        }

        // Log external ADC over threshold register
        log_register_data("THROVA2", REG_THROVA2, (uint8_t *) &throva2, sizeof(throva2));

        if (npz_read_THRUNA2(&thruna2) != OK)
        {
            printf("Failed to read THRUNA2 register\r\n");
            return false;
        }

        // Log external ADC under threshold register
        log_register_data("THRUNA2", REG_THRUNA2, (uint8_t *) &thruna2, sizeof(thruna2));
    }

    printf("----------------------------------------------\r\n");

    return true;
}

static bool validate_peripherals(npz_device_config_s *device_config)
{
    m_configured_count = 0; // Reset count for each validation

    for (int i = 0; i < 4; i++)
    {
        // Check if peripheral configuration is not NULL
        if (device_config->peripherals[i] != NULL)
        {
            m_configured_indices[m_configured_count++] = i;
        }
    }

    return true;
}

static bool peripheral_config_read(npz_device_config_s *device_config)
{
    // Validate peripherals
    if (validate_peripherals(device_config))
    {
        // Configure peripherals
        if (!read_peripherals(device_config))
        {
            printf("Failed to read peripherals\r\n");
            return false;
        }
    }

    return true;
}

static bool global_config_read()
{
    uint8_t data;        // Buffer to store data read from registers
    char binary_str[9];  // Buffer to hold binary strings for each register value
    npz_status_e status; // Variable to hold the status of I2C read

    printf("----------------------------------------------\n\r");
    printf("          Read global registers               \n\r");
    printf("----------------------------------------------\n\r");
    printf("[  ADDR |    REGISTER    |     BIN    | HEX  ]\n\r");
    printf("----------------------------------------------\n\r");

    // Iterate over the register_info array to read and log each register
    for (size_t i = 0; i < sizeof(global_register_info) / sizeof(global_register_info[0]); i++)
    {
        // Read the register value using the generic read function
        status = npz_read_register(global_register_info[i].reg_address, &data, 1);
        if (status != OK)
        {
            printf("Failed to read register at address 0x%02X \n\r", global_register_info[i].reg_address);
            return false;
        }

        // Convert the register value to a binary string for logging
        char_to_binary_string(data, binary_str);


        printf("[  %02X   | %-14s | %-10s | 0x%02X ] \n\r", global_register_info[i].reg_address,
                global_register_info[i].reg_name, binary_str, data);
    }

    printf("----------------------------------------------");

    return true;
}


static bool read_status_registers(void)
{
    npz_register_sta1_s status1 = {0}; // Variable to store status1 register values
    npz_register_sta2_s status2 = {0}; // Variable to store status2 register values

    // Read the first status register
    if (npz_read_STA1(&status1) != OK)
    {
        return false;
    }

    // Read the second status register
    if (npz_read_STA2(&status2) != OK)
    {
        return false;
    }

    // Log the registers
    printf("----------------------------------------------\n\r");
    printf("          Read status registers               \n\r");
    printf("----------------------------------------------\n\r");
    printf("[  ADDR |    REGISTER    |     BIN    | HEX  ]\n\r");
    printf("----------------------------------------------\n\r");

    // Log STA1 register
    char binary_str[9];
    char_to_binary_string(*((uint8_t *) &status1), binary_str); // Use the address to cast
    printf("[  %02X   | %-14s | %-10s | 0x%02X ]\n\r", REG_STA1, "STA1", binary_str, *((uint8_t *) &status1));

    // Log STA2 register
    char_to_binary_string(*((uint8_t *) &status2), binary_str); // Use the address to cast
    printf("[  %02X   | %-14s | %-10s | 0x%02X ]\n\r", REG_STA2, "STA2", binary_str, *((uint8_t *) &status2));

    printf("----------------------------------------------");

    return true;
}


/*****************************************************************************
 * Public Methods
 *****************************************************************************/

void npz_log_configurations(npz_device_config_s *device_config)
{
    if (!global_config_read())
    {
        printf("Failed to read configure global settings");
        return;
    }

    if (!read_status_registers())
    {
		printf("Failed to read status registers \n\r");
		return;
	}

    if (!peripheral_config_read(device_config))
    {
        printf("Failed to read peripherals configuration \n\r");
        return;
    }

    if (!adc_config_read(device_config))
    {
        printf("Failed to read ADC configuration \n\r");
        return;
    }
}

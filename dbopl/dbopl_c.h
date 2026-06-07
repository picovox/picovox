#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * dbopl_c files are written by me (jansakos) as a way to minimize use of any other language except C in the main code.
 * This is therefore C API that should handle all the needed operations for simulation of OPL2/OPL3 via DBOPL.
 */

typedef struct DBOPL_Device DBOPL_Device;

/**
 * @brief Creates a new OPL sound device.
 * 
 * @param sample_rate is a sample rate to which data should be resampled
 * @return pointer to the created (and loaded) device.
 */
DBOPL_Device* dbopl_create(int sample_rate);

/**
 * @brief Destroys (unloads) given OPL device.
 * 
 * @param dev is a pointer to the loaded OPL device.
 */
void dbopl_destroy(DBOPL_Device* dev);

/**
 * @brief Sends new data into OPL.
 * 
 * @param dev is a pointer to the loaded OPL device.
 * @param reg is the address of the desired register.
 * @param value is a value that should be put into the register.
 */
void dbopl_write_reg(DBOPL_Device* dev, uint16_t reg, uint8_t value);

/**
 * @brief Generates new OPL3 sample.
 * 
 * @param dev is a pointer to the loaded OPL device.
 * @param left is a pointer to the left sample destination.
 * @param right is a pointer to the right sample destination.
 */
void dbopl_generate(DBOPL_Device* dev, int16_t* left, int16_t* right);


/**
 * @brief Generates new OPL2 sample.
 * 
 * @param dev is a pointer to the loaded OPL device.
 * @param sample is a pointer to the sample destination.
 */
void dbopl_generate2(DBOPL_Device* dev, int16_t* sample);

#ifdef __cplusplus
}
#endif
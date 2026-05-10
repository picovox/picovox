#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Common interface for all the simulated devices.
 */
typedef struct device_t {

    /**
     * @brief Method loads all the things needed for using the device (such as start PIO program, initialize registers etc.)
     * 
     * @return true if device is loaded, false if anything failed.
     */
    bool (*load_device)();

    /**
     * @brief Method unloads all the things needed after using the device (such as the PIO program etc.)
     * 
     * @return true if device is unloaded, false if anything failed.
     */
    bool (*unload_device)();

    /**
     * @brief Function reads data from PIO buffer and generates a sound sample based on data sent.
     * 
     * @param left_sample is a pointer to number where sample for left channel is placed.
     * @param right_sample is a pointer to number where sample for right channel is placed.
     * 
     * @return number of samples available in internal device buffer.
     */
    size_t (*generate_sample)(int16_t *left_sample, int16_t *right_sample);
} device_t;

/**
 * @brief Declarations for creating instances of all types of modes.
 * 
 * Each mode is stored in its own file, implementing all the functions of struct above.
 * Always check if return value is not NULL (allocation failed)!
 */
device_t *create_covox();
device_t *create_stereo();
device_t *create_ftl();
device_t *create_dss();
device_t *create_opl2();
device_t *create_tandy();
device_t *create_cms();
device_t *create_debugger();

#endif // DEVICE_H
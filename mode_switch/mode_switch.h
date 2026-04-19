#ifndef MODE_SWITCH_H
#define MODE_SWITCH_H

#include <stdbool.h>

/**
 * @brief Initializes the PIO program and pins for mode changing.
 * 
 * @return true if everything succeeded, false if not.
 */
bool init_mode_change();

/**
 * @brief Function reading FIFO for called changes and handling these instructions.
 * 
 * @param change_to is a pointer to a variable where id of device to switch to is stored.
 * @param current is a number representation of current device.
 * @param device_list is a list of all device pointer structs (used for unloading BUSY-using devices).
 */
void mode_change(int8_t *change_to, int8_t current, Device ** device_list);

#endif
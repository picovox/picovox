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
 */
void mode_change(int8_t *change_to);

#endif
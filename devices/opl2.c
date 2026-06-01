#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "device.h"
#include "pio_manager.h"
#include "ringbuffer.h"
#include "dbopl/dbopl_c.h"
#include "pico/multicore.h"
#include "opl2.pio.h"

// Buffer storing samples generated
#define OPL_RINGBUFFER_SIZE 4096

// Sample repeated 2 times -> for 96kHz, only 48 kHz needed (still high quality, but fast enough)
#define SAMPLE_REPEAT 2

// Variables for PIO
static PIO used_pio;
static int8_t used_sm;
static int used_offset;

// Since emulator runs at half the rate, repeat all samples twice
static int16_t last_left_sample = 0;
static int16_t last_right_sample = 0;
static int8_t sample_used = 0;

// Killswitch for core 1
static volatile bool stop_core1 = false;

// Chip emulation - running on core 1
static void load_new_instruction(int16_t *register_address, DBOPL_Device* opl) {

    // If no instruction, go back, cannot happen (failsafe)
    if (pio_sm_is_rx_fifo_empty(used_pio, used_sm)) {
        return;
    }

    uint16_t new_instruction = (pio_sm_get(used_pio, used_sm) >> 22);

    // Strobe determines address/data difference
#if LPT_STROBE_SWAPPED
    if ((new_instruction & 1) == 0) {
        *register_address = (new_instruction >> 1) & 255;
    } else {
        OPL_Pico_WriteRegister(*register_address, ((new_instruction >> 1) & 255));
    }
#else

    if (((new_instruction >> 9) & 1) == 0) {
        *register_address = ((new_instruction >> 1) & 255) + ((new_instruction & 1) * 0x100);
    } else {
        dbopl_write_reg(opl, *register_address, ((new_instruction >> 1) & 255));
    }
#endif
}

static void core1_operation(void) {
    DBOPL_Device* opl = dbopl_create(SAMPLE_RATE/SAMPLE_REPEAT);
    int16_t current_sample_left = 0;
    int16_t current_sample_right = 0;
    int16_t register_address = 0;

    while (!stop_core1) {

        // Before generating new sample load all instructions
        while ((!pio_sm_is_rx_fifo_empty(used_pio, used_sm))) {
            load_new_instruction(&register_address, opl);
        }

        dbopl_generate(opl, &current_sample_left, &current_sample_right);

        // While ringbuffer is full, load new instructions
        while (ringbuffer_is_full() && !stop_core1) {
            load_new_instruction(&register_address, opl);
        }

        ringbuffer_push(current_sample_left);
        ringbuffer_push(current_sample_right);
    }

    // Core 1 should be stopped -> remove device from memory
    dbopl_destroy(opl);
}

bool load_opl2() {
    ringbuffer_init(OPL_RINGBUFFER_SIZE);

    // Load PIO program
    used_offset = pio_manager_load(&used_pio, &used_sm, &opl2_program);

    if (used_offset < 0) {
        return false;
    }

    // Configure PIO GPIO pins
    pio_sm_config used_config = opl2_program_get_default_config(used_offset);
#if LPT_STROBE_SWAPPED
    sm_config_set_in_pins(&used_config, LPT_STROBE_PIN);
#else
    sm_config_set_in_pins(&used_config, LPT_BASE_PIN);
#endif
    sm_config_set_fifo_join(&used_config, PIO_FIFO_JOIN_RX);

    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) { // Set all data pins
        pio_gpio_init(used_pio, i);
    }

    pio_gpio_init(used_pio, LPT_STROBE_PIN);
    pio_gpio_init(used_pio, LPT_INIT_PIN);
#if LPT_STROBE_SWAPPED
    pio_sm_set_consecutive_pindirs(used_pio, used_sm, LPT_STROBE_PIN, 9, false); // Set pins in PIO to be inputs
#else
    pio_sm_set_consecutive_pindirs(used_pio, used_sm, LPT_BASE_PIN, 9, false); // Set pins in PIO to be inputs
#endif

    // Start PIO program
    if (pio_sm_init(used_pio, used_sm, used_offset, &used_config) < 0) {
        return false;
    }

    pio_sm_set_enabled(used_pio, used_sm, true);

    // Start core 1
    stop_core1 = false;
    multicore_reset_core1();
    multicore_launch_core1(core1_operation);
    return true;
}

bool unload_opl2() {

    // Stop core 1
    stop_core1 = true;

    // Stop PIO program
    pio_sm_set_enabled(used_pio, used_sm, false);
    pio_manager_unload(used_pio, used_sm, used_offset, &opl2_program);

    // Deinit all GPIO pins
    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) {
        gpio_deinit(i);
    }

    gpio_deinit(LPT_STROBE_PIN);
    gpio_deinit(LPT_INIT_PIN);
    return true;
}

size_t generate_opl2(int16_t *left_sample, int16_t *right_sample) {

    if (sample_used >= SAMPLE_REPEAT) {

        // Wait for sample if none is generated (cannot lock since samples are generated automatically)
        while (ringbuffer_is_empty()) {
            tight_loop_contents();
        }

        // Cannot happen - added as a failsafe
        if (!ringbuffer_pop(&last_left_sample)) {
            last_left_sample = 0;
            last_right_sample = 0;
        } else {
            while (ringbuffer_is_empty()) {
                tight_loop_contents();
            }

            ringbuffer_pop(&last_right_sample);
        }

        sample_used = 0;
    }

    *left_sample = last_left_sample;
    *right_sample = last_right_sample;
    sample_used++;
    return 0;
}

device_t *create_opl2(void) {
    static device_t opl2_struct = {load_opl2, unload_opl2, generate_opl2};
    return &opl2_struct;
}
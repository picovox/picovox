#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pio_manager.h"
#include "device.h"
#include "hardware/clocks.h"
#include "ftl.pio.h"

// Variables for PIO - sound and detection programs
static PIO sound_pio;
static int8_t sound_sm;
static int sound_offset;

static PIO detection_pio;
static int8_t detection_sm;
static int detection_offset;

bool load_ftl(Device *self) {

    // Load PIO programs
    sound_offset = pio_manager_load(&sound_pio, &sound_sm, &ftl_sound_program);
    if (sound_offset < 0) {
        return false;
    }

    detection_offset = pio_manager_load(&detection_pio, &detection_sm, &ftl_detection_program);
    if (detection_offset < 0) {
        return false;
    }

    // Configure sound PIO GPIO pins
    pio_sm_config used_config_sound = ftl_sound_program_get_default_config(sound_offset);
    sm_config_set_in_pins(&used_config_sound, LPT_BASE_PIN);
    sm_config_set_fifo_join(&used_config_sound, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&used_config_sound, (((float) clock_get_hz(clk_sys)) / (SAMPLE_RATE * 6))); // Frequency same as sample rate

    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) { // Set all data pins
        pio_gpio_init(sound_pio, i);
    }

    pio_sm_set_consecutive_pindirs(sound_pio, sound_sm, LPT_BASE_PIN, 8, false); // Set pins in PIO to be inputs/outputs

    // Start sound PIO program
    if (pio_sm_init(sound_pio, sound_sm, sound_offset, &used_config_sound) < 0) {
        return false;
    }

    pio_sm_set_enabled(sound_pio, sound_sm, true);

    // Configure detection PIO GPIO pins
    pio_sm_config used_config_detection = ftl_detection_program_get_default_config(detection_offset);
    sm_config_set_in_pins(&used_config_detection, LPT_SELIN_PIN);
    sm_config_set_out_pins(&used_config_detection, LPT_PAPEREND_PIN, 1);
    sm_config_set_clkdiv(&used_config_detection, 10.0);

    pio_gpio_init(detection_pio, LPT_PAPEREND_PIN);
    pio_gpio_init(detection_pio, LPT_SELIN_PIN);
    pio_sm_set_consecutive_pindirs(detection_pio, detection_sm, LPT_SELIN_PIN, 1, false);
    pio_sm_set_consecutive_pindirs(detection_pio, detection_sm, LPT_PAPEREND_PIN, 1, true);

    // Start detection PIO program
    if (pio_sm_init(detection_pio, detection_sm, detection_offset, &used_config_detection) < 0) {
        return false;
    }

    pio_sm_set_enabled(detection_pio, detection_sm, true);
    return true;
}

bool unload_ftl(Device *self) {

    // Stop PIO programs
    pio_sm_set_enabled(sound_pio, sound_sm, false);
    pio_sm_set_enabled(detection_pio, detection_sm, false);
    pio_manager_unload(sound_pio, sound_sm, sound_offset, &ftl_sound_program);
    pio_manager_unload(detection_pio, detection_sm, detection_offset, &ftl_detection_program);

    // Deinit all GPIO pins
    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) {
        gpio_deinit(i);
    }

    gpio_deinit(LPT_PAPEREND_PIN);
    gpio_deinit(LPT_SELIN_PIN);
    return true;
}

static inline int16_t read_sample(void) {

    // Wait for sample (generated continuosly -> will not lock)
    while (pio_sm_is_rx_fifo_empty(sound_pio, sound_sm)) {
        tight_loop_contents();
    }

    // If all pins low -> empty sample, otherwise subtract 128 (to generate negative amplitude)
    uint8_t result = ((pio_sm_get(sound_pio, sound_sm) >> 24) & 0xFF);
    return (result == 0) ? 0 : ((result - 128) << 8);
}

// Return middle value to remove random wrong reads
static inline int16_t best_sample(int16_t a, int16_t b, int16_t c) {
    int16_t max_ab = a;
    int16_t min_ab = b;

    if (a < b) {
        max_ab = b;
        min_ab = a;
    }

    if (c >= max_ab) {
        return max_ab;
    }

    if (c >= min_ab) {
        return c;
    }

    return min_ab;
}

size_t generate_ftl(Device *self, int16_t *left_sample, int16_t *right_sample) {
    int16_t sample1 = read_sample();
    int16_t sample2 = read_sample();
    int16_t sample3 = read_sample();
    int16_t current_sample = best_sample(sample1, sample2, sample3);
    *left_sample = current_sample;
    *right_sample = current_sample;
    return 0;
}

Device *create_ftl(void) {
    static Device ftl_struct;

    ftl_struct.load_device = load_ftl;
    ftl_struct.unload_device = unload_ftl;
    ftl_struct.generate_sample = generate_ftl;
    return &ftl_struct;
}
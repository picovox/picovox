#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pio_manager.h"
#include "ringbuffer.h"
#include "device.h"
#include "square/square_c.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "tandy.pio.h"

#define TND_RINGBUFFER_SIZE 2048
#define TND_DETECTION_FREQ_HZ 4000000

// Sample repeated 2 times -> for 96kHz, only 48 kHz needed (still high quality, but fast enough)
#define SAMPLE_REPEAT 2

// Variables for PIO - each device simulated has its own
static PIO sound_pio;
static int8_t sound_sm;
static int sound_offset;

static PIO detection_pio;
static int8_t detection_sm;
static int detection_offset;

// Since emulator runs at half the rate, repeat all samples twice
static int16_t last_sample = 0;
static int8_t sample_used = 0;

// Killswitch for core 1
static volatile bool stop_core1 = false;

static void load_new_instruction(tandy_t *device) {

    // If no instruction, go back, cannot happen (failsafe)
    if (pio_sm_is_rx_fifo_empty(sound_pio, sound_sm)) {
        return;
    }

    // Since TNDLPT has both address and data in one call, no need to wait for more data
    tandy_write(device, pio_sm_get(sound_pio, sound_sm) >> 24);
}

static void reset_chip(tandy_t **device) {
    if (device == NULL) {
        return;
    }

    tandy_destroy(*device);
    *device = tandy_create();
}

static void core1_operation(void) {
    tandy_t *device = tandy_create();
    int16_t current_sample = 0;

    while (!stop_core1) {

        // Before generating new sample load all instructions
        while ((!pio_sm_is_rx_fifo_empty(sound_pio, sound_sm))) {
            load_new_instruction(device);
        }

        current_sample = tandy_get_sample(device);

        // While ringbuffer is full, load new instructions
        while (ringbuffer_is_full() && !stop_core1) {
            load_new_instruction(device);
        }

        ringbuffer_push(current_sample);
    }

    // Core 1 should be stopped -> remove device from memory
    tandy_destroy(device);
}

bool load_tandy(Device *self) {
    ringbuffer_init(TND_RINGBUFFER_SIZE);

    // Load PIO programs
    sound_offset = pio_manager_load(&sound_pio, &sound_sm, &tandy_sound_program);

    if (sound_offset < 0) {
        return false;
    }

    detection_offset = pio_manager_load(&detection_pio, &detection_sm, &tandy_detection_program);

    if (detection_offset < 0) {
        return false;
    }

    // Configure sound PIO GPIO pins
    pio_sm_config sound_config = tandy_sound_program_get_default_config(sound_offset);
    sm_config_set_in_pins(&sound_config, LPT_BASE_PIN);
    sm_config_set_fifo_join(&sound_config, PIO_FIFO_JOIN_RX);

    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) { // Set all data pins
        pio_gpio_init(sound_pio, i);
    }

    pio_gpio_init(sound_pio, LPT_STROBE_PIN);
    pio_gpio_init(sound_pio, LPT_INIT_PIN);
    gpio_init(LPT_SELIN_PIN); // Reset is not directed by PIO, but is loaded as a part of sound production
    pio_sm_set_consecutive_pindirs(sound_pio, sound_sm, LPT_BASE_PIN, 8, false); // Set pins in PIO to be inputs

    // Start sound PIO program
    if (pio_sm_init(sound_pio, sound_sm, sound_offset, &sound_config) < 0) {
        return false;
    }

    pio_sm_set_enabled(sound_pio, sound_sm, true);

    /** Information regarding TNDLPT detection
     *  There is not any important autodetect logic, only thing this program does is
     *  to return ACK pin signal when anything should be processed by real chips.
     *  Detection response is not based on any real measurements (may be improved),
     *  but is enough for detection by the TNDLPT driver. Not really sure if reset
     *  is supposed to put ACK pin high for any period, thankfully it is not detected.
     */

    // Configure detection PIO GPIO pins
    pio_sm_config detection_config = tandy_detection_program_get_default_config(detection_offset);
    sm_config_set_set_pins(&detection_config, LPT_ACK_PIN, 1);
    sm_config_set_clkdiv(&detection_config, clock_get_hz(clk_sys) / TND_DETECTION_FREQ_HZ);
    pio_gpio_init(detection_pio, LPT_STROBE_PIN);
    pio_gpio_init(detection_pio, LPT_INIT_PIN);
    pio_gpio_init(detection_pio, LPT_ACK_PIN);
    pio_sm_set_consecutive_pindirs(detection_pio, detection_sm, LPT_ACK_PIN, 1, true); // Set pins in PIO to be inputs

    // Start detection PIO program
    if (pio_sm_init(detection_pio, detection_sm, detection_offset, &detection_config) < 0) {
        return false;
    }

    pio_sm_set_enabled(detection_pio, detection_sm, true);

    // Start core 1
    stop_core1 = false;
    multicore_reset_core1();
    multicore_launch_core1(core1_operation);
    return true;
}

bool unload_tandy(Device *self) {

    // Stop core 1
    stop_core1 = true;

    // Stop PIO programs
    pio_sm_set_enabled(sound_pio, sound_sm, false);
    pio_manager_unload(sound_pio, sound_sm, sound_offset, &tandy_sound_program);

    pio_sm_set_enabled(detection_pio, detection_sm, false);
    pio_manager_unload(detection_pio, detection_sm, detection_offset, &tandy_detection_program);

    // Deinit all GPIO pins
    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) {
        gpio_deinit(i);
    }

    gpio_deinit(LPT_STROBE_PIN);
    gpio_deinit(LPT_ACK_PIN);
    gpio_deinit(LPT_INIT_PIN);
    gpio_deinit(LPT_SELIN_PIN);
    return true;
}

size_t generate_tandy(Device *self, int16_t *left_sample, int16_t *right_sample) {

    if (sample_used >= SAMPLE_REPEAT) {

        // Wait for sample if none is generated (cannot lock since samples are generated automatically)
        while (ringbuffer_is_empty()) {
            tight_loop_contents();
        }

        // Cannot happen - added as a failsafe
        if (!ringbuffer_pop(&last_sample)) {
            last_sample = 0;
        }

        sample_used = 0;
    }

    *left_sample = last_sample;
    *right_sample = last_sample;
    sample_used++;
    return 0;
}

Device *create_tandy(void) {
    static Device tandy_struct;

    tandy_struct.load_device = load_tandy;
    tandy_struct.unload_device = unload_tandy;
    tandy_struct.generate_sample = generate_tandy;
    return &tandy_struct;
}
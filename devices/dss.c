#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pio_manager.h"
#include "ringbuffer.h"
#include "device.h"
#include "pico/time.h"
#include "dss.pio.h"

// Buffer storing samples to process
#define DSS_RINGBUFFER_SIZE 16
#define DSS_SAMPLE_RATE 7000

static const double DSS_RATE_TO_SAMPLE = SAMPLE_RATE / (double) DSS_SAMPLE_RATE;

// Variables for PIO - each device simulated has its own
static PIO used_pio;
static int8_t used_sm;
static int used_offset;
static int8_t used_pio_irq;

// Sample repeating -> DSS has 7 kHz, Picovox 96 kHz -> not precise division
repeating_timer_t dss_buffer_timer;
static int16_t current_sample = 0;
static int16_t repeated_sample = 0;
static double sample_repeated = 0;
static bool is_new_sample = true;

// If new sample received -> try to buffer it
void __isr ringbuffer_filler(void) {

    while (!pio_sm_is_rx_fifo_empty(used_pio, used_sm)) {
        int16_t pushed_data = (((pio_sm_get(used_pio, used_sm) >> 24) & 0xFF) - 128);

        // If buffer is full -> discard current sample and let computer know
        if (!ringbuffer_push(pushed_data)) {
            gpio_put(LPT_ACK_PIN, true);
        }
    }
}

static bool new_sample(repeating_timer_t *timer_for_buffer) {

    // If no sample available -> return empty sample
    if (ringbuffer_is_empty()) {
        current_sample = 0;
        is_new_sample = true;
        return true;
    }

    // If buffer was full -> now it is not (reset pin)
    if (ringbuffer_is_full()) {
        gpio_put(LPT_ACK_PIN, false);
    }

    ringbuffer_pop(&current_sample);
    is_new_sample = true;
    current_sample = current_sample << 8;
    return true;
}

bool load_dss(Device *self) {
    ringbuffer_init(DSS_RINGBUFFER_SIZE);

    // Load PIO program
    used_offset = pio_manager_load(&used_pio, &used_sm, &dss_program);
    if (used_offset < 0) {
        return false;
    }

    // Configure PIO GPIO pins
    pio_sm_config used_config = dss_program_get_default_config(used_offset);
    sm_config_set_in_pins(&used_config, LPT_BASE_PIN);
    sm_config_set_fifo_join(&used_config, PIO_FIFO_JOIN_RX);
    sm_config_set_jmp_pin(&used_config, LPT_SELIN_PIN);

    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) { // Set all data pins
        pio_gpio_init(used_pio, i);
    }

    pio_gpio_init(used_pio, LPT_SELIN_PIN);
    gpio_init(LPT_ACK_PIN); // FifoFull is not directed by PIO (buffer is not in PIO)
    gpio_set_dir(LPT_ACK_PIN, true);
    pio_sm_set_consecutive_pindirs(used_pio, used_sm, LPT_BASE_PIN, 8, false); // Set pins in PIO to be inputs

    // Set IRQ for correct buffer filling
    used_pio_irq = pio_manager_get_irq(used_pio);
    irq_set_exclusive_handler(used_pio_irq, ringbuffer_filler);
    irq_set_enabled(used_pio_irq, true);
    pio_set_irq0_source_enabled(used_pio, irq_sources[used_sm], true);

    // Start PIO program
    if (pio_sm_init(used_pio, used_sm, used_offset, &used_config) < 0) {
        return false;
    }
    pio_sm_set_enabled(used_pio, used_sm, true);

    // Set buffer popping
    add_repeating_timer_us(1000000 / DSS_SAMPLE_RATE, new_sample, NULL, &dss_buffer_timer);
    return true;
}

bool unload_dss(Device *self) {

    // Stop buffer unloading
    cancel_repeating_timer(&dss_buffer_timer);

    // Stop PIO program
    pio_sm_set_enabled(used_pio, used_sm, false);
    pio_set_irq0_source_enabled(used_pio, irq_sources[used_sm], false);
    irq_set_enabled(used_pio_irq, false);
    irq_remove_handler(used_pio_irq, ringbuffer_filler);
    pio_manager_unload(used_pio, used_sm, used_offset, &dss_program);

    // Deinit all GPIO pins
    for (int i = LPT_BASE_PIN; i < LPT_BASE_PIN + 8; i++) {
        gpio_deinit(i);
    }

    gpio_deinit(LPT_ACK_PIN);
    gpio_deinit(LPT_SELIN_PIN);
    return true;
}

/** DSS sample repetition
 *  Since samples should run at 7 kHz, but Picovox uses 96 kHz as base frequency,
 *  we count how many times the sample has already been used and if the count
 *  exceeds the number of repetition (13 or 14) we load a new sample. Float
 *  is not precise, but in the worst case one sample is repeated 14 times instead
 *  of 13, which is not noticeable in the final sound.
 */ 
static inline void correct_sample(void) {

    if (sample_repeated > DSS_RATE_TO_SAMPLE) {

        while (!is_new_sample) {
            tight_loop_contents;
        }

        sample_repeated -= DSS_RATE_TO_SAMPLE;
        repeated_sample = current_sample;
        is_new_sample = false;
    }

    sample_repeated++;
}

size_t generate_dss(Device *self, int16_t *left_sample, int16_t *right_sample) {
    correct_sample();
    *left_sample = repeated_sample;
    *right_sample = repeated_sample;
    return 0;
}

Device *create_dss(void) {
    Device *dss_struct = calloc(1, sizeof(Device));

    if (dss_struct == NULL) {
        return NULL;
    }

    dss_struct->load_device = load_dss;
    dss_struct->unload_device = unload_dss;
    dss_struct->generate_sample = generate_dss;
    return dss_struct;
}
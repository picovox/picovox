#include "config.h"

// Definitions for I2S library
#define PICO_AUDIO_PIO 0
#define PICO_AUDIO_DMA_IRQ 1
#define SAMPLES_PER_BUFFER 512
#define NUM_BUFFERS 10
#define CHANNEL_COUNT 2

#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "device.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "mode_switch/mode_switch.h"

// List of all devices
#define NUM_DEVICES 8
device_t *devices[NUM_DEVICES];
int8_t current_device = 0;
int8_t wanted_device = 0;

bool load_device_list() {
    devices[0] = create_covox();
    devices[1] = create_stereo();
    devices[2] = create_ftl();
    devices[3] = create_dss();
    devices[4] = create_opl2();
    devices[5] = create_tandy();
    devices[6] = create_cms();
    devices[7] = create_opl3();

    for (int i = 0; i < NUM_DEVICES; i++) {
        if (devices[i] == NULL) {
            return false;
        }
    }

    return true;
}

bool change_device(void) {

    if (!devices[current_device]->unload_device()) {
        wanted_device = 0;
        return false;
    }

    current_device = wanted_device;

    if (!devices[current_device]->load_device()) {
        wanted_device = 0;
        return false;
    }

    return true;
}

void unmute_device(void) {
    gpio_init(PICO_AUDIO_MUTE);
    gpio_set_dir(PICO_AUDIO_MUTE, GPIO_OUT);

    gpio_put(PICO_AUDIO_MUTE, true); // Unmutes device
}

// I2S library setup
audio_buffer_pool_t *load_audio(void) {

    static audio_format_t audio_requested_format = {
        .sample_freq = SAMPLE_RATE,
        .channel_count = CHANNEL_COUNT,
        .format = AUDIO_BUFFER_FORMAT_PCM_S16
    };

    static audio_i2s_config_t config = {
        .data_pin = PICO_AUDIO_I2S_DATA_PIN,
        .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
        .dma_channel = 0,
        .pio_sm = 0
    };

    const audio_format_t *audio_given_format = audio_i2s_setup(&audio_requested_format, &config);

    if (audio_given_format == NULL) {
        return NULL;
    }

    static audio_buffer_format_t buffer_format = {
        .format = &audio_requested_format,
        .sample_stride = 4
    };

    audio_buffer_pool_t *buffer_pool = audio_new_producer_pool(&buffer_format, NUM_BUFFERS, SAMPLES_PER_BUFFER);

    if (!audio_i2s_connect(buffer_pool)) {
        return NULL;
    }

    audio_i2s_set_enabled(true);
    return buffer_pool;
}

// Main function
int main() {
    set_sys_clock_khz(280000, false); // Overclock 280 MHz (tested as safe and stable, other projects run even faster)
    sleep_ms(100);
    stdio_init_all();

    if (!load_device_list()) {
        return 1;
    }

    if (!devices[current_device]->load_device(devices[current_device])) {
        return 1;
    }

    audio_buffer_pool_t *buffer_pool = load_audio();

    if (buffer_pool == NULL) {
        return 1;
    }

    init_mode_change();

    //load_change_device_irq();

    unmute_device();
    
    int16_t left_sample = 0;
    int16_t right_sample = 0;
    audio_buffer_t *buffer = NULL;

    while (true) {

        mode_change(&wanted_device, current_device, devices);
        
        // Device should change => change device
        if (current_device != wanted_device) {
            change_device();
        }

        // Wait for free buffer
        while ((buffer = take_audio_buffer(buffer_pool, false)) == NULL) {
            tight_loop_contents();
        }

        int16_t *samples = (int16_t *)buffer->buffer->bytes;

        // Fill the buffer
        for (uint i = 0; i < buffer->max_sample_count; i++) {
            devices[current_device]->generate_sample(&left_sample, &right_sample);
            samples[2 * i]     = left_sample;
            samples[2 * i + 1] = right_sample;
        }

        // Send filled buffer
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(buffer_pool, buffer);
    }
}
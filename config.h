#ifndef CONFIG_H
#define CONFIG_H

#define SAMPLE_RATE 96000

// Definitions of GPIO LPT pins
#define LPT_BASE_PIN 1
#define LPT_STROBE_PIN 9    // Beware! STROBE pin must be exactly one position before or after the DATA pins! (Check control down.)
#define LPT_AUTOFEED_PIN 10
#define LPT_INIT_PIN 11
#define LPT_SELIN_PIN 12
#define LPT_ERROR_PIN 13
#define LPT_ACK_PIN 14
#define LPT_BUSY_PIN 15
#define LPT_PAPEREND_PIN 16
#define LPT_SELECT_PIN 17

#define LPT_DATA_DIR 0     // Pin for switching the D0-D7 direction (PC->picovox × picovox->PC)

// Definitions of GPIO I2S pins
#define PICO_AUDIO_I2S_CLOCK_PIN_BASE 27
#define PICO_AUDIO_I2S_CLOCK_PINS_SWAPPED 0
#define PICO_AUDIO_I2S_DATA_PIN 26

// Definition of unused pin
#define PICO_UNUSED_PIN 22

// Definitions of GPIO mode switch button (GPIO - ground)
#define CHANGE_BUTTON_PIN 18

// Control of correct LPT_STROBE_PIN definition
#if LPT_STROBE_PIN == (LPT_BASE_PIN - 1)
    #define LPT_STROBE_SWAPPED 1
#elif LPT_STROBE_PIN == (LPT_BASE_PIN + 8)
    #define LPT_STROBE_SWAPPED 0
#else
    #error "STROBE pin must be either LPT_BASE_PIN-1 or LPT_BASE_PIN+8"
#endif

#endif // CONFIG_H
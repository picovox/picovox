#include "dbopl_c.h"
#include "dbopl.h"

struct DBOPL_Device {
    DBOPL::Chip* chip;
};

DBOPL_Device* dbopl_create(int sample_rate) {
    DBOPL_Device* dev = new DBOPL_Device();

    dev->chip = new DBOPL::Chip(true);
    dev->chip->Setup(sample_rate);

    return dev;
}

void dbopl_destroy(DBOPL_Device* dev) {
    if (!dev) {
        return;
    }

    delete dev->chip;
    delete dev;
}

void dbopl_write_reg(DBOPL_Device* dev, uint16_t reg, uint8_t value) {
    dev->chip->WriteReg(reg, value);
}

void dbopl_generate(DBOPL_Device* dev, int16_t* left, int16_t* right) {
    int32_t buffer[2];

    dev->chip->GenerateBlock3(1, buffer);

    int32_t l = buffer[0];
    int32_t r = buffer[1];

    if (l < -32768) l = -32768;
    if (l >  32767) l =  32767;

    if (r < -32768) r = -32768;
    if (r >  32767) r =  32767;

    *left  = (int16_t)l;
    *right = (int16_t)r;
}

void dbopl_generate2(DBOPL_Device* dev, int16_t* sample) {
    int32_t curr_sample[2];

    dev->chip->GenerateBlock2(1, curr_sample);

    if (curr_sample[0] < -32768) {
        curr_sample[0] = -32768;
    }

    if (curr_sample[0] > 32767) {
        curr_sample[0] = 32767;
    }

    *sample = (int16_t)curr_sample[0];
}
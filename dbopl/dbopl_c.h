#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DBOPL_Device DBOPL_Device;

DBOPL_Device* dbopl_create(int sample_rate);

void dbopl_destroy(DBOPL_Device* dev);

void dbopl_write_reg(
    DBOPL_Device* dev,
    uint16_t reg,
    uint8_t value
);

void dbopl_generate(
    DBOPL_Device* dev,
    int16_t* left,
    int16_t* right
);

#ifdef __cplusplus
}
#endif
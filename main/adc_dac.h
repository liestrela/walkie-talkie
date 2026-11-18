#ifndef ADC_DAC_H
#define ADC_DAC_H

#include "esp_adc/adc_continuous.h"
#include "driver/dac_continuous.h"

#include "types.h"

#define SAMPLE_RATE 20000 /* Hz */
#define FRAME_SIZE 256
#define MAX_S_BUF_SIZE 1024
#define READ_TIMEOUT 1000

void init_adc(void);
void init_dac(void);
void finish_adc(void);
void finish_dac(void);

void adc_read_samples(u32 total, u8 *buf, u32 *samples_read);
void dac_write_samples(u8 *buf, u32 cnt);

#endif

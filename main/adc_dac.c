#include "esp_log.h"
#include "esp_adc/adc_continuous.h"
#include "driver/dac_continuous.h"
#include "log.h"
#include "types.h"

#include "adc_dac.h"

adc_continuous_handle_t adc_handle = NULL;
dac_continuous_handle_t dac_handle = NULL;

void
init_adc(void)
{
    /* adc handle config */
    adc_continuous_handle_cfg_t h_config = {
        .max_store_buf_size = MAX_S_BUF_SIZE,
        .conv_frame_size = FRAME_SIZE,
    };
    ESP_ERROR_CHECK(
        adc_continuous_new_handle(&h_config, &adc_handle)
    );

    /* adc pattern config */
    adc_digi_pattern_config_t pattern = {
        .atten = ADC_ATTEN_DB_12, /* attenuation to about 12 dB */
        .channel = ADC_CHANNEL_6, /* gpio34 = adc1 channel 6 */
        .unit = ADC_UNIT_1,       /* adc1 */
        .bit_width = ADC_BITWIDTH_12,
    };

    /* channel config */
    adc_continuous_config_t c_config = {
        .sample_freq_hz = SAMPLE_RATE,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1, /* use only adc1 */
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
        .pattern_num = 1,
        .adc_pattern = &pattern,
    };
    ESP_ERROR_CHECK(
        adc_continuous_config(adc_handle, &c_config)
    );

    ESP_ERROR_CHECK(
        adc_continuous_start(adc_handle)
    );

    ESP_LOGI(LOG_WT, "started adc");
}

void
finish_adc(void)
{
    ESP_ERROR_CHECK(adc_continuous_stop(adc_handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(adc_handle));
}

void
init_dac(void)
{
    dac_continuous_config_t config = {
        .chan_mask = DAC_CHANNEL_MASK_CH0, /* gpio25 */
        .desc_num = 4,
        .buf_size = 2048,
        .freq_hz = SAMPLE_RATE,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
    };

    ESP_ERROR_CHECK(
        dac_continuous_new_channels(&config, &dac_handle)
    );
    ESP_ERROR_CHECK(
        dac_continuous_enable(dac_handle)
    );
}

void
finish_dac(void)
{
    ESP_ERROR_CHECK(
        dac_continuous_disable(dac_handle)
    );
    ESP_ERROR_CHECK(
        dac_continuous_del_channels(dac_handle)
    );
}

void
adc_read_samples(u32 total, u8 *buf, u32 *samples_read)
{
    u32 bytes_read = 0;
    static u8  adc_buf[FRAME_SIZE] = {0};

    esp_err_t read_res = adc_continuous_read(
        adc_handle,
        adc_buf,
        sizeof(adc_buf),
        &bytes_read,
        READ_TIMEOUT
    );
    ESP_ERROR_CHECK(read_res);

    adc_digi_output_data_t *res_arr = (adc_digi_output_data_t *) adc_buf;
    for (u32 i=0;
         i < bytes_read/SOC_ADC_DIGI_RESULT_BYTES && (*samples_read)<total;
         i++) {
        if (res_arr[i].type1.channel != ADC_CHANNEL_6) continue;

        /* truncate 16 bit sample to 8 bit */
        buf[*samples_read] = (u8)(res_arr[i].type1.data>>4);
        (*samples_read)++;
    }
}

void
dac_write_samples(u8 *buf, u32 cnt)
{
    ESP_ERROR_CHECK(
        dac_continuous_write(dac_handle, buf, cnt, NULL, -1)
    );
}

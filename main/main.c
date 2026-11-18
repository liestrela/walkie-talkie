#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"

#include "adc_dac.h"
#include "log.h"
#include "types.h"

#define SECS_TO_SAMPLES(secs) SAMPLE_RATE*secs
#define FREE_HMEM() \
	do { \
		ESP_LOGI(LOG_SI, "free hmem: %ld bytes\n", esp_get_free_heap_size()); \
	} while(0) \

const char *LOG_SI = "wt-sysinfo";
const char *LOG_WT = "wt";

void
app_main(void)
{
	const u32 total_samples = SECS_TO_SAMPLES(3);
	u32 samples_read = 0;
	u8 *snippet_buf = calloc(sizeof(u8), total_samples);

	if (!snippet_buf)
		ESP_LOGE(LOG_WT, "couldn't allocate audio snippet buffer");

	FREE_HMEM();

	for (;;) {
		init_adc();

		ESP_LOGI(LOG_WT, "listening...");
		while (samples_read < total_samples) {
			adc_read_samples(total_samples, snippet_buf, &samples_read);
		}

		finish_adc();

		ESP_LOGI(
			LOG_WT, "finished reading %ld samples. waiting 0.5 s", samples_read
		);

		/* wait 0.5 s */
		vTaskDelay(pdMS_TO_TICKS(500));

		/* play 8-bit audio on speaker via dac_continuous */
		init_dac();

		ESP_LOGI(LOG_WT, "writing samples to speaker");

		dac_write_samples(snippet_buf, samples_read);

		finish_dac();

		/* reseting buffers */
		memset(snippet_buf, 0, total_samples);
		samples_read = 0;
	}
}

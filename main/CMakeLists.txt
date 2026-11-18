idf_component_register(SRCS "main.c"
	                        "adc_dac.c"
                       INCLUDE_DIRS "."
					   REQUIRES esp_adc
				                esp_driver_dac
						        freertos)

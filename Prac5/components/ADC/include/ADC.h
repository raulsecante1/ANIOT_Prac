#include "esp_adc/adc_oneshot.h"

adc_oneshot_unit_handle_t ADC_init();

float read_voltage(adc_oneshot_unit_handle_t adc_handle);
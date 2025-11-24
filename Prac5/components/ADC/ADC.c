#include "esp_adc/adc_oneshot.h"

static const char *TAG = "ADC_one_shot";

adc_oneshot_unit_handle_t ADC_init(){
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));

    return adc1_handle;
}

float read_voltage(adc_oneshot_unit_handle_t adc_handle){
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &adc_raw));
    
    float voltage = (float) adc_raw / 4095.0f * 5.0f;

    return voltage;
}
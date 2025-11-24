#include "ADC.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(PRAC5_EVENTS);

enum {
    DATA_READY,
};

typedef struct {
    adc_oneshot_unit_handle_t adc_handle;
    esp_event_loop_handle_t loop;
} adc_task_params_t;


static void i_read_the_god_damn_datas_and_fuking_dont_call_back(void* arg);
float read_distance(float voltage);
adc_oneshot_unit_handle_t GP2Y0A41SK0F_init();
void GP2Y0A41SK0F_start(adc_oneshot_unit_handle_t adc_handle, esp_event_loop_handle_t loop);
void GP2Y0A41SK0F_stop();
QueueHandle_t get_distance_handle();
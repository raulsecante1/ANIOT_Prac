#include <stdio.h>

#include "ADC.h"
#include "GP2Y0A41SK0F.h"

#include "esp_event.h"

#include "esp_log.h"

static const char *TAG = "Prac5";


void Prac5_run_event(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

    switch (event_id) {
        case DATA_READY:
            QueueHandle_t distance_queue = get_distance_handle();
            float distance;
            if (xQueueReceive(distance_queue, &distance, pdMS_TO_TICKS(10)) == pdPASS){
                ESP_LOGI(TAG, "measured distance %f", distance);
            }
            else{
                ESP_LOGI(TAG, "queue empty");
            }
            break;
        default:
            ESP_LOGI(TAG, "unknown event");
            break;
    }
}


void app_main(void){

    esp_event_loop_handle_t PR5_loop;
    
    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "loop_task",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 2048,
        .task_core_id = tskNO_AFFINITY
    };
    
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &PR5_loop));

        ESP_ERROR_CHECK(esp_event_handler_register_with(
        PR5_loop, 
        PRAC5_EVENTS, 
        ESP_EVENT_ANY_ID, 
        Prac5_run_event, 
        NULL
    ));

    adc_oneshot_unit_handle_t adc_handle = GP2Y0A41SK0F_init();

    GP2Y0A41SK0F_start(adc_handle, PR5_loop);

    while(1){
        vTaskDelay(1000);
    }

    GP2Y0A41SK0F_stop();

}
#include <stdlib.h>
#include "ADC.h"
#include "GP2Y0A41SK0F.h"

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_event.h"

#include "esp_log.h"

ESP_EVENT_DEFINE_BASE(PRAC5_EVENTS);

static const char *TAG = "GP2Y0A41SK0F";

esp_timer_handle_t adc_timer;

QueueHandle_t distance_queue;

TaskHandle_t adc_gp2_handle;

extern esp_event_loop_handle_t loop;


/*
static void i_read_the_god_damn_datas_and_fuking_dont_call_back(void* arg){

    adc_oneshot_unit_handle_t adc_handle = (adc_oneshot_unit_handle_t)arg;

    float adc_reading = 0.0f;
    float distance = 0.0f;
    float suma = 0.0f;
    int count = 0;

    while(count < CONFIG_CONS_N){

        adc_reading = read_voltage(adc_handle);
        
        if(adc_reading > 0.005f && adc_reading < 3.2f) {  //else accroding to the graph are readings senselesses
            distance = read_distance(adc_reading);
            suma += distance;
            count++;
        }
        
    }

    esp_event_post(PRAC5_EVENTS, DATA_READY, NULL, 0, portMAX_DELAY);
}
*/


static void timer_callback(void *arg){

    xTaskNotifyGive(adc_gp2_handle);
}


static void i_read_the_god_damn_datas_and_fuking_dont_call_back(void* arg){

    adc_task_params_t* params = (adc_task_params_t*) arg;
    adc_oneshot_unit_handle_t adc_handle = params->adc_handle;
    esp_event_loop_handle_t loop = params->loop;

    float adc_reading = 0.0f;
    float distance = 0.0f;
    float suma = 0.0f;
    int count = 0;
    int retries = 0;
    const int max_retries = CONFIG_CONS_N * 10;

    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        suma = 0.0f;
        count = 0;
        retries = 0;

        while(count < CONFIG_CONS_N && retries < max_retries){

            adc_reading = read_voltage(adc_handle);
        
            if(adc_reading > 0.005f && adc_reading < 3.2f) {  //the rest accroding to the graph are readings senselesses
                distance = read_distance(adc_reading);
                suma += distance;
                count++;
            }

            retries++;
        
        }
        float average = suma / CONFIG_CONS_N;

        xQueueSend(distance_queue, &average, 0); 

        ESP_LOGI(TAG, "GP2 posted a data");

        esp_event_post_to(loop, PRAC5_EVENTS, DATA_READY, NULL, 0, portMAX_DELAY);

    }

}


float read_distance(float voltage){
    float equ_a = atof(CONFIG_EQU_A);
    float equ_b = atof(CONFIG_EQU_B);
    float equ_c = atof(CONFIG_EQU_C);
    
    float distance = equ_a / (voltage - equ_c) - equ_b;

    return distance;
}


adc_oneshot_unit_handle_t GP2Y0A41SK0F_init(){
    adc_oneshot_unit_handle_t adc_handle = ADC_init();
    return adc_handle;
}


void GP2Y0A41SK0F_start(adc_oneshot_unit_handle_t adc_handle, esp_event_loop_handle_t loop){
    
    adc_task_params_t* task_params = malloc(sizeof(adc_task_params_t));
    task_params->adc_handle = adc_handle;
    task_params->loop = loop;

    distance_queue = xQueueCreate(12, sizeof(float));

    xTaskCreate(i_read_the_god_damn_datas_and_fuking_dont_call_back, "ADC_GP2_reading", 4096, task_params, 5, &adc_gp2_handle);

    const esp_timer_create_args_t adc_timer_config = {
        .callback = &timer_callback,
    };
    ESP_ERROR_CHECK(esp_timer_create(&adc_timer_config, &adc_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(adc_timer, 1000000));

    ESP_LOGI(TAG, "GP2Y0A41SK0F startted");
    
}

void GP2Y0A41SK0F_stop(){
    ESP_ERROR_CHECK(esp_timer_stop(adc_timer));
    ESP_ERROR_CHECK(esp_timer_delete(adc_timer));
    adc_timer = NULL;
    
    vQueueDelete(distance_queue);
    distance_queue = NULL;
    
    ESP_LOGI(TAG, "GP2Y0A41SK0F stopped");
}

QueueHandle_t get_distance_handle(){
    return distance_queue;
}
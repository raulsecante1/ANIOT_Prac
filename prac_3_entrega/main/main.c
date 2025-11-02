#include <stdio.h>
#include "shtc3.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "mock_wifi.h"
#include "mock_flash.h"

bool debug = false;

bool data_connection = false;

size_t precision = sizeof(float);

float temp = 0.0f;
float hum = 0.0f;

QueueHandle_t xQueue = NULL; 


static const char *TAG = "example_of_group_3";  //i guess not so sure

shtc3_t tempSensor;
i2c_master_bus_handle_t bus_handle;

void init_i2c(void) {
    //uint16_t id;
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = 8,
        .sda_io_num = 10,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    shtc3_init(&tempSensor, bus_handle, 0x70);
}

void sensor(void * pvParameters){
    float ticks = *((float *) pvParameters);

    while (1){
        if (data_connection) {
            size_t data_in_flash = getDataLeft();
            while(data_in_flash>0){
                temp = readFloatFromFlash(precision);
                hum = readFloatFromFlash(precision);
                ESP_LOGI(TAG, "Temp is %f and hum is %f", temp, hum);
                data_in_flash = getDataLeft();
            }
            shtc3_get_temp_and_hum(&tempSensor, &temp, &hum);
            ESP_LOGI(TAG, "Temp is %f and hum is %f", temp, hum);
        } else {
            shtc3_get_temp_and_hum(&tempSensor, &temp, &hum);
            writeToFlash(&temp, precision);
            writeToFlash(&hum, precision);
        }
        vTaskDelay(pdMS_TO_TICKS(ticks));
    }
}

void wifi_run_event(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    
    if (debug) {ESP_LOGI(TAG, "wifi_run_event: base=%p id=%d", event_base, event_id);}

    switch (event_id) {
        /*
        case NOT_INITIALIZED:
            //ESP_LOGI(TAG, "WIFI_NOT_INITIALIZED");
            break;
        case INITIALIZED:
            //ESP_LOGI(TAG, "WIFI_INITIALIZED");
            break;
        */
        case WIFI_MOCK_EVENT_WIFI_CONNECTED:
            //ESP_LOGI(TAG, "WIFI_CONNECTED");
            break;
        case WIFI_MOCK_EVENT_WIFI_GOT_IP:
            //ESP_LOGI(TAG, "WIFI_CONNECTED_WITH_IP");
            data_connection = true;  // calls the sending
            break;
        case WIFI_MOCK_EVENT_WIFI_DISCONNECTED:
            //ESP_LOGI(TAG, "WIFI_DISCONNECTED");
            data_connection = false;
            wifi_connect();
            break;
    }
}

void app_main(void)
{
    init_i2c();

    static float time = 1000.0f * CONFIG_PERIOD_N;

    size_t capacity = 1024;

    mock_flash_init(capacity);

    TaskHandle_t sensor_handle;

    xTaskCreate(sensor, "PeriodicTask", 2048, &time, 5, &sensor_handle);

    esp_event_loop_handle_t loop;
    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "loop_task",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 2048,
        .task_core_id = tskNO_AFFINITY
    };
    
    esp_err_t ret = esp_event_loop_create(&loop_args, &loop);
    if (debug){
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Event loop creation failed: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "Event loop created successfully");
        }
    }

    wifi_mock_init(loop);  //entrar el estado no wifi

    esp_err_t rett = esp_event_handler_register_with(loop, WIFI_MOCK, ESP_EVENT_ANY_ID, &wifi_run_event, NULL);
    if (debug){
        if (rett != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register event handler: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "Event handler registered successfully");
        }
    }

    wifi_connect();  // Entrar el estado no ip

    ESP_LOGI(TAG, "Conectando a la WIFI");

    while(1){
        vTaskDelay(pdMS_TO_TICKS(time*30));
    }

    wifi_disconnect();

    vTaskSuspend(sensor_handle);
    vTaskDelete(sensor_handle);

    mock_flash_destroy();

}
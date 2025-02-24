#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_timer.h>
#include <esp_vfs_fat.h>
#include <freertos/ringbuf.h>

// Include MAX30102 driver
#include "max30102.h"

TaskHandle_t readMAXTask_handle = NULL;

/**
 * @brief Read data from MAX30102 at 25 Hz and send to ring buffer
 * 
 * @param pvParameters 
 */
void max30102_test(void *pvParameters)
{
    i2c_dev_t dev;
    memset(&dev, 0, sizeof(i2c_dev_t));

    ESP_ERROR_CHECK(max30102_initDesc(&dev, 0, 34, 35));

    struct max30102_record record;

    if (max30102_readPartID(&dev) == ESP_OK) {
        ESP_LOGI(__func__, "Found MAX30102!");
    } else {
        ESP_LOGE(__func__, "Not found MAX30102");
    }

    if (max30102_init(0x1F, 4, 2, 1000, 118, 4096, &record, &dev) == ESP_OK) {
        ESP_LOGI(__func__, "Init OK");
    } else {
        ESP_LOGE(__func__, "Init fail!");
    }

    uint16_t samplesTaken = 0;
    const TickType_t delay_25hz = pdMS_TO_TICKS(40); // 40 ms delay for 25 Hz

    while (1)
    {
        max30102_check(&record, &dev); // Check the sensor, read up to 3 samples

        while (max30102_available(&record)) // Do we have new data?
        {
            samplesTaken++;

            printf("%ld,%ld\n", max30102_getFIFORed(&record), max30102_getFIFOIR(&record));

            max30102_nextSample(&record); // We're finished with this sample so move to next sample
        }

        vTaskDelay(delay_25hz); // Delay to maintain 25 Hz
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(i2cdev_init()); 

    // Create tasks
    xTaskCreatePinnedToCore(max30102_test, "max30102_test", 1024 * 5, NULL, 6, &readMAXTask_handle, 0);
}

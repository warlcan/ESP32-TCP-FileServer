#include "include/display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "DISPLAY";
static SSD1306_t dev;
static QueueHandle_t display_queue = NULL;

static void display_controller_task(void *pvParameters) {
    DisplayCommand display_command;
    const char ws_str[] = "                ";
    while (1){
        if (xQueueReceive(display_queue, &display_command, portMAX_DELAY)){
            switch (display_command.cmd) {
            case 1:
                if (strlen(display_command.data) < 16){
                    memcpy(display_command.data + strlen(display_command.data), ws_str, 16 - strlen(display_command.data));
                }
                ssd1306_display_text(&dev, display_command.y, display_command.data, strlen(display_command.data), false);
                break;
            default:
                ESP_LOGE(TAG, "Error cmd");
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void display_init(void) {
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    display_queue = xQueueCreate(DISPLAY_QUEUE_SIZE, sizeof(DisplayCommand));
    xTaskCreate(display_controller_task, "DisplayController", 4096, NULL, 7, NULL);
}

void send_display_command(int cmd, int y, char *data){
    DisplayCommand display_command = {
        .cmd = cmd,
        .y = y,
    };
    memcpy(display_command.data, data, strlen(data));
    xQueueSend(display_queue, &display_command, portMAX_DELAY);
}

void send_display_clear(){
    ssd1306_clear_screen(&dev, false);
}

void draw_intro(){
    uint8_t circle[] = {
        0b11100111,
        0b10000001,
        0b10000001,
        0b00000000,
        0b00000000,
        0b10000001,
        0b10000001,
        0b11100111,
    };
    ssd1306_display_text(&dev, 0, "  Server up", 11, false); 
    ssd1306_bitmaps(&dev, 0, 0, circle, 8, 8, true);
}
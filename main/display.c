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

void show_intro(bool is_clear_screen){
    if(is_clear_screen){
        ssd1306_clear_screen(&dev, false);
    }
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

void show_mnt_status(int mnt_status){
    switch (mnt_status) {
    case 1:
        send_display_command(1, MNT_STATUS_STR, "MNT: Y");
        break;
    case 2:
        send_display_command(1, MNT_STATUS_STR, "MNT: N");
        break;
    case 3:
        send_display_command(1, MNT_STATUS_STR, "MNT: ERROR");
        break;
    default:
        send_display_command(1, MNT_STATUS_STR, "INT ERROR");
        break;
    }
}

void show_cnt_status(int cnt_status){
    switch (cnt_status) {
    case 1:
        send_display_command(1, CNT_STATUS_STR, "CNT: Y");
        break;
    case 2:
        send_display_command(1, CNT_STATUS_STR, "CNT: N");
        break;
    case 3:
        send_display_command(1, CNT_STATUS_STR, "CNT: ERROR");
        break;
    default:
        send_display_command(1, CNT_STATUS_STR, "INT ERROR");
        break;
    }
}

void show_animation_loading(bool status_loading){

}

void show_file_size(size_t file_size){
    char buf[16];
    sprintf(buf, "FILE: %zu B", file_size);
    send_display_command(1, FILE_SIZE_STR, buf);
}

void show_progress_status(uint8_t progress){
    char buf[16];
    sprintf(buf, "LOAD: %u%%", progress);
    send_display_command(1, PERCENT_STATUS_STR, buf);
}

void show_file_name(char *file_name){
    char buf[16];
    sprintf(buf, "NME: %s", file_name);
    send_display_command(1, FILE_NAME_STR, buf);
}
#include "include/display.h"

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
                size_t data_str_len = strlen(display_command.data);
                if (strlen(display_command.data) < 16){
                    memcpy(display_command.data + data_str_len, ws_str, 16 - data_str_len);
                }
                ssd1306_display_text(&dev, display_command.y, display_command.data, data_str_len, false);
                break;
            default:
                ESP_LOGE(TAG, "Error cmd");
                break;
            }
        }
    }
}

void display_init(void) {
    i2c_master_init(&dev, PIN_SDA_GPIO, PIN_SCL_GPIO, PIN_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xff);

    display_queue = xQueueCreate(DISPLAY_QUEUE_SIZE, sizeof(DisplayCommand));
    xTaskCreate(display_controller_task, "DisplayController", 4096, NULL, 6, NULL);
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
    show_mnt_status(-1);
    show_cnt_status(-1);
    show_status_load(STATUS_VOID);
    show_file_size(0);
    show_progress_status(0);
    show_file_name("-");
}

void show_esp_ip(char *esp_ip){
    send_display_command(1, ESP_IP_STATUS_STR, esp_ip);
}

void show_status_load(StatusLoading status_loading){
    switch (status_loading)
    {
    case STATUS_UPLOAD:
        send_display_command(1, LOADING_STATUS_STR, "SRVR <- CLNT");
        break;
    case STATUS_DOWNLOAD:
        send_display_command(1, LOADING_STATUS_STR, "SRVR -> CLNT");
        break;
    case STATUS_VOID:
        send_display_command(1, LOADING_STATUS_STR, "SRVR    CLNT");
        break;
    default:
        break;
    }
}

void show_mnt_status(int mnt_status){
    switch (mnt_status) {
    case DISPLAY_YES:
        send_display_command(1, MNT_STATUS_STR, "MNT: Y");
        break;
    case DISPLAY_NO:
        send_display_command(1, MNT_STATUS_STR, "MNT: N");
        break;
    case DISPLAY_ERROR:
        send_display_command(1, MNT_STATUS_STR, "MNT: ERROR");
        break;
    default:
        send_display_command(1, MNT_STATUS_STR, "MNT: -");
        break;
    }
}

void show_cnt_status(int cnt_status){
    switch (cnt_status) {
    case DISPLAY_YES:
        send_display_command(1, CNT_STATUS_STR, "CNT: Y");
        break;
    case DISPLAY_NO:
        send_display_command(1, CNT_STATUS_STR, "CNT: N");
        break;
    case DISPLAY_ERROR:
        send_display_command(1, CNT_STATUS_STR, "CNT: ERROR");
        break;
    default:
        send_display_command(1, CNT_STATUS_STR, "CNT: -");
        break;
    }
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
    snprintf(buf, sizeof(buf), "NAME: %.9s", file_name);
    send_display_command(1, FILE_NAME_STR, buf);
}
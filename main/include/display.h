#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>
#include "ssd1306.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int cmd;
    char data[16];
    int y;
} DisplayCommand;

typedef enum {
    STATUS_UPLOAD = 0,
    STATUS_DOWNLOAD = 1,
    STATUS_VOID = -1
} StatusLoading;

enum {
    DISPLAY_YES = 0,
    DISPLAY_NO = 1,
    DISPLAY_ERROR = 2
};

void display_init(void);
void send_display_command(int cmd, int y, char *data);
void show_intro(bool);
void show_esp_ip(char *esp_ip);
void show_mnt_status(int mnt_status);
void show_cnt_status(int cnt_status);
void show_status_load(StatusLoading status_loading);
void show_file_size(size_t file_size);
void show_progress_status(uint8_t progress);
void show_file_name(char *file_name);

#endif
#ifndef DISPLAY_H
#define DISPLAY_H

#include "ssd1306.h"
#include <stdint.h>
#include <stdbool.h>

#define DISPLAY_QUEUE_SIZE 10

#define CNT_STATUS_STR 2
#define MNT_STATUS_STR 3
#define FILE_SIZE_STR 5
#define PERCENT_STATUS_STR 6
#define FILE_NAME_STR 7

typedef struct {
    int cmd;
    char data[16];
    int y;
} DisplayCommand;

void display_init(void);
void send_display_command(int cmd, int y, char *data);
void send_display_clear(void);
void show_intro(void);
void show_mnt_status(int mnt_status);
void show_cnt_status(int cnt_status);
void show_file_size(size_t file_size);
void show_progress_status(uint8_t progress);
void show_file_name(char *file_name);

#endif
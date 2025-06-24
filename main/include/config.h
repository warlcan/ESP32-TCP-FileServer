#ifndef CONFIG_H
#define CONFIG_H

// general
#define CHUNK_SIZE 32
#define CALCULATING_PROGRESS_INTERVAL 500

// server controller (main)
#define PORT 12035
// FAT limit (8 bytes for the name, 1 byte for the dot, 
// and 3 bytes for the file extension) 
#define FILE_NAME_BUFFER_SIZE 8 + 1 + 3
#define UPLOAD_PACKET_TYPE 0x01
#define DOWNLOAD_PACKET_TYPE 0x02
#define DATA_PACKET_TYPE 0xDD
#define MOUNT_POINT "/sdcard"

// p_upload
#define BUFFER_UPLOAD_FILE_SIZE 8192

// p_download
#define BUFFER_DOWNLOAD_FILE_SIZE 8192

// display.h
#define DISPLAY_QUEUE_SIZE 10

#define CNT_STATUS_STR 2
#define MNT_STATUS_STR 3
#define LOADING_STATUS_STR 4
#define FILE_SIZE_STR 5
#define PERCENT_STATUS_STR 6
#define FILE_NAME_STR 7

#define PIN_SDA_GPIO 21
#define PIN_SCL_GPIO 22
#define PIN_RESET_GPIO -1


// sd_card.h
#define SD_CARD_MAX_FEQ_KHZ 6000

#define PIN_SPI_MISO 19
#define PIN_SPI_MOSI 23
#define PIN_SPI_CLK 18
#define PIN_SPI_CS 5

// wfi.h
#define WIFI_SSID "MODEM"
#define WIFI_PASS "7341793hi"

#endif
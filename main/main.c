#include "include/config.h"
#include "include/sd_card.h"
#include "include/wifi.h"
#include "include/display.h"
#include "include/p_download.h"
#include "include/p_upload.h"

#include <math.h>
#include <sys/socket.h>

#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "Server";

typedef enum {
    CONNECT_TYPE_DOWNLOAD = 0,
    CONNECT_TYPE_UPLOAD = 1,
    CONNECT_TYPE_ERROR = -1
} ConnectType;

// structure of service packet
typedef struct {
    ConnectType connect_type;
    uint32_t file_size;
    char file_name[FILE_NAME_BUFFER_SIZE];
} ServicePacketStruct;

// processing first packet
static ServicePacketStruct parsing_service_packet(char *buffer){
    ServicePacketStruct service_packet;
    uint16_t offset = 0;

    switch (buffer[0]) {
    case DOWNLOAD_PACKET_TYPE:
        service_packet.connect_type = CONNECT_TYPE_DOWNLOAD;
        offset = 1;
        memcpy(&service_packet.file_size, buffer + offset, sizeof(service_packet.file_size));
        offset += sizeof(service_packet.file_size);
        memcpy(service_packet.file_name, buffer + offset, sizeof(service_packet.file_name));
        break;
    case UPLOAD_PACKET_TYPE:
        service_packet.connect_type = CONNECT_TYPE_UPLOAD;
        offset = 1;
        memcpy(service_packet.file_name, buffer + offset, sizeof(service_packet.file_name));
        break;
    default:
        service_packet.connect_type = CONNECT_TYPE_ERROR;
        break;
    }
    return service_packet;
}

// open sock for wait connect
static int open_listen_sock(int server_port){
    struct sockaddr_in server_addr;
    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, 1);
    return listen_sock;
}

// main task
static void server_controller_task(void *pvParameters){
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int listen_sock = open_listen_sock(PORT);

    while (1){
        char buffer[CHUNK_SIZE];

        // accept connect and show information in display
        int main_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        ESP_LOGI(TAG, "Connect");
        // show_intro(false);
        show_cnt_status(DISPLAY_YES);

        // mount and show mount status
        if(mount_sd() != ESP_OK){
            show_mnt_status(DISPLAY_ERROR);
            close(main_sock);
            continue;
        } else show_mnt_status(DISPLAY_YES);

        // receive service packet
        recv(main_sock, buffer, CHUNK_SIZE, 0);
        ServicePacketStruct service_packet = parsing_service_packet(buffer);
        show_file_size(service_packet.file_size);
        show_file_name(service_packet.file_name);
        
        // start download / upload
        switch (service_packet.connect_type){
        case CONNECT_TYPE_DOWNLOAD:
            show_status_load(STATUS_DOWNLOAD);
            start_download(main_sock, service_packet.file_name, service_packet.file_size);
            break;
        case CONNECT_TYPE_UPLOAD:
            show_status_load(STATUS_UPLOAD);
            start_upload(main_sock, service_packet.file_name);
            break;
        default:
            ESP_LOGE(TAG, "Error: Bad flag in service packet");
            break;
        }

        // close connect
        close(main_sock);
        ESP_LOGI(TAG, "Connect close");
        show_cnt_status(DISPLAY_NO);
        umount_sd();
        show_mnt_status(DISPLAY_NO);
    }
}

void app_main(void){
    display_init();
    show_intro(1);

    nvs_flash_init();
    wifi_init();

    sd_card_init(MOUNT_POINT);

    xTaskCreate(server_controller_task, "ServerController", 8192, NULL, 8, NULL);
}
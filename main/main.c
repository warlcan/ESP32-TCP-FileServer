#include "include/sd_card.h"
#include "include/wifi.h"
#include "include/display.h"
#include <math.h>
#include <sys/socket.h>

#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define UPLOAD_PACKET_TYPE 0xEE
#define DOWNLOAD_PACKET_TYPE 0xAA
#define DATA_PACKET_TYPE 0xDD
#define FIN_PACKET_TYPE 0xFF

#define PORT 12035
#define PACKET_SIZE 32
#define BUFFER_SD_SIZE 8192

#define STATUS_DOWNLOAD false
#define STATUS_UPLOAD true

static const char mount_point[] = "/sdcard";
static const char *TAG = "MAIN";

static bool status_loading;
static char file_name[16] = {0};
static uint32_t max_packets = 0;


static int calculate_load_progress(size_t counter){
    int progress = round((counter * 100) / max_packets);
    return progress;
}
static size_t calculate_file_size(){
    return max_packets * (PACKET_SIZE - 1);
}

static void download_file(int main_sock, FILE *file){
    size_t counter = 0;
    uint8_t last_load_progress = 255;
    char buffer[PACKET_SIZE];
    size_t file_size = calculate_file_size();
    show_file_size(file_size);
    show_progress_status(0);
    show_file_name(file_name);

    while(1) {
        uint8_t received_l = recv(main_sock, buffer, PACKET_SIZE, 0); // if PACKET_SIZE > 255: uint8_t >> uint16_t or more
        if (received_l <= 0) break;

        if ((counter % 100) == 0) {
            uint8_t load_progress = calculate_load_progress(counter);
            if (last_load_progress != load_progress) {    
                show_progress_status(load_progress);
                last_load_progress = load_progress;
            }
        }

        if (buffer[0] == DATA_PACKET_TYPE) {
            fwrite(buffer + 1, sizeof(char), PACKET_SIZE - 1, file);
        } else {
            ESP_LOGE(TAG, "Error: Bad data flag: %d", buffer[0]);
        }

        counter++;
    }
    show_progress_status(100);
}

static size_t find_file_size(FILE* file){
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file_size;
}

static int calculate_upload_progress(size_t file_size, size_t counter){
    int progress = (counter * 100) / max_packets;
    return progress;
}

static void upload_file(int main_sock, FILE *file){
    char buffer[PACKET_SIZE];
    size_t file_size = find_file_size(file);
    show_file_size(file_size);
    max_packets = file_size / PACKET_SIZE;
    show_progress_status(0);
    show_file_name(file_name);

    size_t counter = 0;
    while (1) {
        int bytes_read = fread(buffer, 1, PACKET_SIZE, file);
        if (bytes_read <= 0) break;
        
        send(main_sock, buffer, bytes_read, 0);
        if ((counter % 100) == 0) {
            int progress = calculate_upload_progress(file_size, counter);
            show_progress_status(progress);
        }
        counter++;
    }
    show_progress_status(100);
}


static void packet_upload_request(char *buffer){
    memcpy(file_name, buffer + 1, sizeof(file_name));
    ESP_LOGI(TAG, "%s", file_name);
    status_loading = STATUS_UPLOAD;
}

static void packet_download_request(char *buffer){
    memcpy(&max_packets, buffer + 1, sizeof(max_packets));
    memcpy(file_name, buffer + 1 + sizeof(max_packets), sizeof(file_name));
    status_loading = STATUS_DOWNLOAD;
}

static void parsing_service_packet(char *buffer){
    switch (buffer[0]) {
    case DOWNLOAD_PACKET_TYPE:
        packet_download_request(buffer);
        break;
    case UPLOAD_PACKET_TYPE:
        packet_upload_request(buffer);
        break;
    default:
        ESP_LOGE(TAG, "Error: Bad flag in service packet");
        break;
    }
}

static FILE* open_file(char *open_type){
    char path[256] = {0};
    strcat(path, mount_point);
    strcat(path, "/");
    strcat(path, file_name);

    FILE* file = fopen(path, open_type);
    if (file == NULL){
        ESP_LOGE(TAG, "Error: File \"%s\" not found", path);
        return NULL;
    }

    ESP_LOGI(TAG, "File open: %s", path);
    setvbuf(file, NULL, _IOFBF, BUFFER_SD_SIZE);
    return file;
}

static void end_connect(int *main_sock){
    ESP_LOGI(TAG, "Close connect");
    umount_sd();
    show_mnt_status(2);
    close(*main_sock);
    show_cnt_status(2);
}

static void server_controller_task(void *pvParameters){
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, 1);

    while (1) {
        FILE *file = NULL;
        char buffer[PACKET_SIZE];

        int main_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        ESP_LOGI(TAG, "Accept connect");
        sd_card_init(mount_point);
        send_display_clear();
        show_intro();
        show_cnt_status(1);
        if (mount_sd()) {
            show_mnt_status(3);
            close(main_sock);
            continue;
        }
        show_mnt_status(1);

        recv(main_sock, buffer, sizeof(buffer), 0);
        parsing_service_packet(buffer);

        if (status_loading == STATUS_UPLOAD){
            ESP_LOGI(TAG, "Upload");
            file = open_file("rb");
            if (file == NULL) {
                end_connect(&main_sock);
                continue;
            }
            upload_file(main_sock, file);
        } else {
            ESP_LOGI(TAG, "Download");
            file = open_file("wb");
            if (file == NULL) {
                end_connect(&main_sock);
                continue;
            }
            download_file(main_sock, file);
        }

        fclose(file);
        end_connect(&main_sock);
    }
}

void app_main(void) {
    display_init();
    show_intro();

    nvs_flash_init();
    wifi_init();

    xTaskCreate(server_controller_task, "ServerController", 8192, NULL, 8, NULL);
}

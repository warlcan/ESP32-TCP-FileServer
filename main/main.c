#include "include/sd_card.h"
#include "include/wifi.h"
#include "include/display.h"

#include <sys/socket.h>

#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ssd1306.h"

#define PORT 12035
#define PACKET_SIZE 32
#define BUFFER_SD_SIZE 8192

static const char mount_point[] = "/sdcard";
static const char *TAG = "SD_CARD";
static bool is_FIN = false;
static char file_name[16] = {0};
static unsigned int max_packets = 0;

FILE *file;

static bool mount_sd(){
    esp_err_t ret = sd_card_controller(true, mount_point);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Mount: OK");
        send_display_command(1, 2, "MNT: Y");
        return true;
    } else {
        ESP_LOGE(TAG, "Mount failed: 0x%x", ret);
        send_display_command(1, 2, "MNT: ERROR");
        return false;
    }
}

static bool umount_sd(){
    esp_err_t ret = sd_card_controller(false, mount_point);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Umount: OK");
        send_display_command(1, 2, "MNT: N");
        return true;
    } else {
        ESP_LOGE(TAG, "Umount failed: 0x%x", ret);
        send_display_command(1, 2, "MNT: ERROR");
        return false;
    }
}


static void packet_service_type(char *buffer){
    memcpy(file_name, buffer + 1, sizeof(file_name));
    memcpy(&max_packets, buffer + 1 + sizeof(file_name), sizeof(max_packets));
}

static void packet_data_type(char *buffer){
    fwrite(buffer + 1, sizeof(char), PACKET_SIZE - 1, file);
}

static void packet_fin_type(){
    is_FIN = true;
}

static void parsing_packet(char *buffer){
        switch (buffer[0]){
        case 0xAA: // file name
            packet_service_type(buffer);
            break;
        case 0xDD: // data
            packet_data_type(buffer);
            break;
        case 0xFF: // fin flag
            packet_fin_type();
            break;
        default:
            ESP_LOGE(TAG, "INCORRECT TYPE BYTE: CHECK CLIENT");
            break;
        }
}


static void open_file(){
        char path[256] = {0};
        strcat(path, mount_point);
        strcat(path, "/");
        strcat(path, file_name);

        file = fopen(path, "wb");
        setvbuf(file, NULL, _IOFBF, BUFFER_SD_SIZE);
}

static int last_percent = -1;
static void percent_display_show(unsigned int counter) {
    int percent = (counter * 100) / max_packets;
    if (percent != last_percent){
        char temp[16];
        snprintf(temp, 16, "LOAD: %d%%", percent);
        send_display_command(1, 6, temp);
        last_percent = percent;
    }
}

static void filesize_display_show(){
    char temp_s[16];
    double file_size = (double)max_packets / 32;
    snprintf(temp_s, 16, "FILE: %.2f", file_size);
    send_display_command(1, 5, temp_s);
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
        char recv_buffer[PACKET_SIZE] = {0};
        unsigned int counter = 0;

        // accept connection
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        send_display_clear();
        draw_intro();
        ESP_LOGI(TAG, "Accept connect");
        send_display_command(1, 3, "CNT: Y");
        // mount sd
        if(!mount_sd()){
            continue;
        }
        // receive first(service) packet
        recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
        parsing_packet(recv_buffer);

        // open file
        open_file();
        filesize_display_show();
        send_display_command(1, 7, file_name);
        
        while (1) {
            int received_len = recv(client_sock, recv_buffer, sizeof(recv_buffer), 0);
            if (received_len <= 0) break;

            percent_display_show(counter);

            parsing_packet(recv_buffer);
            if(is_FIN){
                ESP_LOGI(TAG, "Fin packet");
                break;
            }
            counter++;
        }
        is_FIN = false;
        fclose(file);
        umount_sd();
        close(client_sock);
        send_display_command(1, 3, "CNT: N");
    }
}

void app_main(void) {
    display_init();
    draw_intro();

    nvs_flash_init();
    wifi_init();

    xTaskCreate(server_controller_task, "ServerController", 8192, NULL, 8, NULL);
}

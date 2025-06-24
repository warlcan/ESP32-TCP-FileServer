#include "include/p_upload.h"
#include "include/display.h"

static const char *TAG = "P_Upload";

static FILE* open_file(char* file_name){
    char path[256] = {0};
    snprintf(path, sizeof(path), "%s/%s", MOUNT_POINT, file_name);

    FILE* file = fopen(path, "wb");
    if (file == NULL){
        ESP_LOGE(TAG, "Error: File \"%s\" error", path);
        return NULL;
    }
    ESP_LOGI(TAG, "File open: %s", path);
    setvbuf(file, NULL, _IOFBF, BUFFER_UPLOAD_FILE_SIZE);
    return file;
}

void start_upload(int main_sock, char *file_name, uint32_t file_size){
    FILE* file = open_file(file_name);
    if (file == NULL) return;
    size_t counter = 0;
    size_t max_packets = (file_size + (CHUNK_SIZE - 2)) / (CHUNK_SIZE - 1);

    show_progress_status(0);

    while(1){
        char buffer[CHUNK_SIZE] = {0}; // resetting at each iteration

        int8_t received_len = recv(main_sock, buffer, CHUNK_SIZE, 0);
        if (received_len <= 0) break;

        if (counter % CALCULATING_PROGRESS_INTERVAL == 0){
            uint8_t load_progess = (counter * 100) / max_packets;
            show_progress_status(load_progess);
        }

        if (buffer[0] == DATA_PACKET_TYPE) {
            fwrite(buffer + 1, sizeof(char), CHUNK_SIZE - 1, file);
        } else ESP_LOGE(TAG, "Error: Bad data flag: %d", buffer[0]);

        counter++;
    }
    fclose(file);
    show_progress_status(100);
}
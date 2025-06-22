#include "include/p_upload.h"
#include "include/display.h"

static const char *TAG = "Upload";

static FILE* open_file(char* file_name){
    char path[256] = {0};
    strcat(path, MOUNT_POINT);
    strcat(path, "/");
    strcat(path, file_name);

    FILE* file = fopen(path, "rb");
    if (file == NULL){
        ESP_LOGE(TAG, "Error: File \"%s\" not found", path);
        return NULL;
    }
    ESP_LOGI(TAG, "File open: %s", path);
    setvbuf(file, NULL, _IOFBF, BUFFER_UPLOAD_FILE_SIZE);
    return file;
}

static size_t find_file_size(FILE* file){
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file_size;
}

void start_upload(int main_sock, char *file_name){
    FILE* file = open_file(file_name);
    if (file == NULL) return;
    size_t counter = 0;
    size_t file_size = find_file_size(file);
    size_t max_packets = file_size / CHUNK_SIZE;

    show_file_size(file_size);
    show_file_name(file_name);
    show_progress_status(0);

    while(1){
        char buffer[CHUNK_SIZE] = {0}; // resetting at each iteration

        buffer[0] = DATA_PACKET_TYPE;       
        int bytes_read = fread(buffer + 1, 1, CHUNK_SIZE, file);
        if (bytes_read <= 0) break;
        send(main_sock, buffer, CHUNK_SIZE, 0);

        if (counter % CALCULATING_PROGRESS_INTERVAL == 0){
            uint8_t load_progess = (counter * 100) / max_packets;
            show_progress_status(load_progess);
        }

        counter++;
    }
    fclose(file);
    show_progress_status(100);
}
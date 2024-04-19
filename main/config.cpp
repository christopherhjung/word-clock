
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "config.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ff.h"

#include "freertos/semphr.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "example";

cJSON* config = NULL;

void readFile(const char* path, char *buffer, int size){
    ESP_LOGI(TAG, "Reading file");
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fgets(buffer, size , f);
    fclose(f);
}

void writeFile(const char* path, const char *buffer){
    FILE *f = fopen(path, "w+");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, buffer);
    fclose(f);
}


void mount(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
            .base_path = "/spiffs",
            .partition_label = NULL,
            .max_files = 5,
            .format_if_mount_failed = true
    };


    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

char* loadFile(const char* path){
    FILE *f = fopen(path, "r");

    if(f != NULL){
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *string = (char*)malloc(fsize + 1);
        fread(string, fsize, 1, f);
        fclose(f);
        string[fsize] = 0;
        return string;
    }else{
        ESP_LOGI(TAG, "no file");
        return NULL;
    }
}

xSemaphoreHandle config_lock = 0;
void config_init(){
    config_lock = xSemaphoreCreateBinary();  // start of task
}

void config_load(){
    xSemaphoreTake(config_lock, 0);
    cJSON_Delete(config);
    char* json = loadFile("/spiffs/config.json");

    if(json != NULL){
        config = cJSON_Parse(json);
        free(json);
    }

    if(config == NULL){
        ESP_LOGI(TAG, "error json");
        config = cJSON_Parse("{}");
    }

    xSemaphoreGive(config_lock);
}

void set_or_replace_value(cJSON* json, const char *key, cJSON* value){
    cJSON* obj = cJSON_GetObjectItem(json, key);
    if(obj == NULL){
        cJSON_AddItemToObject(config, key, value);
    }else{
        cJSON_ReplaceItemInObject(config, key, value);
    }
}

void set_or_replace_bool(cJSON* json, const char *key, bool value){
    cJSON* cBool = cJSON_CreateBool(value);
    set_or_replace_value(json, key, cBool);
}

void set_or_replace_str(cJSON* json, const char *key, const char* value){
    cJSON* cStr = cJSON_CreateString(value);
    set_or_replace_value(json, key, cStr);
}

void set_or_replace_number(cJSON* json, const char *key, float value){
    cJSON* cNumber = cJSON_CreateNumber((double)value);
    set_or_replace_value(json, key, cNumber);
}

const char* get_str(cJSON* json, const char *key, const char *default_val){
    cJSON* entry = cJSON_GetObjectItem(json, key);
    if(entry == NULL){
        return default_val;
    }
    return entry->valuestring;
}

bool get_bool(cJSON* json, const char *key, bool default_val){
    cJSON* entry = cJSON_GetObjectItem(json, key);
    if(entry == NULL){
        return default_val;
    }
    return cJSON_IsTrue(entry);
}

float get_number(cJSON* json, const char *key, float default_val){
    cJSON* entry = cJSON_GetObjectItem(json, key);
    if(entry == NULL){
        return default_val;
    }
    return (float)entry->valuedouble;
}

template<class T>
void nextArrayElement(const cJSON *array, int* next_element, T* ref, bool (*fun)(const cJSON*, T*)){
    if(array != NULL){
        int size = cJSON_GetArraySize(array);

        for(int offset = 0 ; offset < size ; offset++){
            int current = (*next_element + offset ) % size;
            cJSON *network = cJSON_GetArrayItem(array, current);

            if(fun(network, ref)){
                *next_element = ( current + 1 ) % size;
                return;
            }
        }
    }

    *next_element = -1;
}

bool has_value(const cJSON* json, const char* value){
    return json->valuestring != NULL && strcmp(json->valuestring, value) == 0;
}

bool has_field_value(const cJSON* json, const char* field, const char* value){
    if(json != NULL){
        const cJSON *json_field = cJSON_GetObjectItem(json, field);

        return has_value(json_field, value);
    }

    return false;
}

bool copy_field_value(const cJSON* json, const char* field, void* target, int max_length){
    if(json != NULL){
        const char *field_value = cJSON_GetObjectItem(json, field)->valuestring;

        if(field_value != NULL){
            unsigned int host_length = strlen(field_value) + 1;
            if(host_length <= max_length){
                memcpy(target, field_value, host_length);
                return true;
            }
        }
    }

    return false;
}

bool fetch_wifi(const cJSON* network, wifi_config_t *wifi_config){
    if(has_field_value(network, "type", "wifi")){
        if(!copy_field_value(network, "ssid", wifi_config->sta.ssid, 32)){
            return false;
        }

        if(!copy_field_value(network, "password", wifi_config->sta.password, 64)){
            return false;
        }

        return true;
    }else{
        ESP_LOGI(TAG, "no wifi");
    }

    return false;
}

void setup_wifi(wifi_config_t *wifi_config, int* wifi_index){
    xSemaphoreTake(config_lock, 0);
    const cJSON *networks = cJSON_GetObjectItem(config, "networks");
    nextArrayElement(networks, wifi_index, wifi_config, fetch_wifi);
    xSemaphoreGive(config_lock);
}

void config_set_foreground_color(pixel_t pixel){
    char *rgb_str = (char*)malloc(17 * sizeof(char));
    snprintf(rgb_str, 17, "rgb(%d,%d,%d)", pixel.red, pixel.green, pixel.blue);
    xSemaphoreTake(config_lock, 0);
    set_or_replace_str(config, "foreground", rgb_str);
    xSemaphoreGive(config_lock);
}

pixel_t config_get_foreground_color(){
    xSemaphoreTake(config_lock, 0);
    const char* rgb_str = get_str(config, "foreground", "rgb(255,255,255)");
    pixel_t color = parse_pixel(rgb_str);
    xSemaphoreGive(config_lock);
    return color;
}

void config_set_background_color(pixel_t pixel){
    char *rgb_str = (char*)malloc(17 * sizeof(char));
    snprintf(rgb_str, 17, "rgb(%d,%d,%d)", pixel.red, pixel.green, pixel.blue);
    xSemaphoreTake(config_lock, 0);
    set_or_replace_str(config, "background", rgb_str);
    xSemaphoreGive(config_lock);
}

pixel_t config_get_background_color(){
    xSemaphoreTake(config_lock, 0);
    const char* rgb_str = get_str(config, "background", "rgb(0,0,0)");
    pixel_t color = parse_pixel(rgb_str);
    xSemaphoreGive(config_lock);
    return color;
}

void config_set_max_brightness(float max_brightness){
    xSemaphoreTake(config_lock, 0);
    set_or_replace_number(config, "brightness", max_brightness);
    xSemaphoreGive(config_lock);
}

float config_get_max_brightness(){
    xSemaphoreTake(config_lock, 0);
    float max_brightness = get_number(config, "brightness", 1.0);
    xSemaphoreGive(config_lock);
    return max_brightness;
}

void config_set_power_status(bool status){
    xSemaphoreTake(config_lock, 0);
    set_or_replace_bool(config, "power", status);
    xSemaphoreGive(config_lock);
}

bool config_get_power_status(){
    xSemaphoreTake(config_lock, 0);
    bool status = get_bool(config, "power", true);
    xSemaphoreGive(config_lock);
    return status;
}

void config_set_it_is_active(bool status){
    xSemaphoreTake(config_lock, 0);
    set_or_replace_bool(config, "it_is", status);
    xSemaphoreGive(config_lock);
}

bool config_get_it_is_active(){
    xSemaphoreTake(config_lock, 0);
    bool status = get_bool(config, "it_is", false);
    xSemaphoreGive(config_lock);
    return status;
}

void config_save(){
    xSemaphoreTake(config_lock, 0);
    ESP_LOGI(TAG, "Power Handler1");
    char *json = cJSON_PrintUnformatted(config);

    if(json != NULL){
        writeFile("/spiffs/config.json", json);
    }

    writeFile("/spiffs/config.json", json);
    xSemaphoreGive(config_lock);
    free(json);
}

void listFiles(const char* path){
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}
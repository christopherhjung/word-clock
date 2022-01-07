
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "cJSON.h"

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


void loadConfig(){
    cJSON_Delete(config);
    FILE *f = fopen("/spiffs/config.json", "rb");
    if(f != NULL){
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *string = (char*)malloc(fsize + 1);
        fread(string, fsize, 1, f);
        fclose(f);
        string[fsize] = 0;

        config = cJSON_Parse(string);
    }else{
        config = cJSON_Parse("{\"api\": \"test\"}");
    }
}

void setOrReplace(cJSON* json, const char *key, const char* value){
    cJSON* obj = cJSON_GetObjectItem(json, key);
    cJSON* cStr = cJSON_CreateString(value);
    if(obj == NULL){
        cJSON_AddItemToObject(config, key, cStr);
    }else{
        cJSON_ReplaceItemInObject(config, key, cStr);
    }
}

char* getString(cJSON* json, const char *key){
    cJSON* entry = cJSON_GetObjectItem(json, key);
    if(entry == NULL){
        return NULL;
    }
    return entry->valuestring;
}

void setVersion(const char *version){
    setOrReplace(config, "version", version);
}

const char* getVersion(){
    return getString(config, "version");
}

void setToken(const char *token){
    setOrReplace(config, "token", token);
}

const char* getToken(){
    return getString(config, "token");
}

void writeFile(const char* path, const char *buffer){
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, buffer);
    fclose(f);
}


void saveConfig(){
    char *json = cJSON_Print(config);
    writeFile("/spiffs/config.json", json);
    free(json);
}
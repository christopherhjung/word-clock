
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
#include "core.h"
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
void init_config(){
    config_lock = xSemaphoreCreateBinary();  // start of task
}

void loadConfig(){
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

bool fetchWifi(const cJSON* network, wifi_config_t *wifi_config){
    if(network != NULL){
        const cJSON *networkType = cJSON_GetObjectItem(network, "type");

        if(networkType != NULL && strcmp(networkType->valuestring, "wifi") == 0){
            const char *ssid = cJSON_GetObjectItem(network, "ssid")->valuestring;
            const char *password = cJSON_GetObjectItem(network, "password")->valuestring;
            unsigned int ssid_length = strlen(ssid) + 1;
            if(ssid_length <= 32){
                memcpy(wifi_config->sta.ssid, ssid, ssid_length);
            }else{
                return false;
            }

            unsigned int password_length = strlen(password) + 1;
            if(password_length <= 64){
                memcpy(wifi_config->sta.password, password, password_length);
            }else{
                return false;
            }

            return true;
        }else{
            ESP_LOGI(TAG, "no wifi");
        }
    }

    return false;
}

void setup_wifi(wifi_config_t *wifi_config, int* wifi_index){
    xSemaphoreTake(config_lock, 0);
    const cJSON *networks = cJSON_GetObjectItem(config, "networks");
    nextArrayElement(networks, wifi_index, wifi_config, fetchWifi);
    xSemaphoreGive(config_lock);
}

bool fetchServer(const cJSON* server, tcp_server_t *tcp_server){
    const cJSON *serverType = cJSON_GetObjectItem(server, "type");

    if(serverType != NULL && strcmp(serverType->valuestring, "tcp") == 0){
        const char *host = cJSON_GetObjectItem(server, "host")->valuestring;
        const char *port = cJSON_GetObjectItem(server, "port")->valuestring;

        unsigned int host_length = strlen(host) + 1;
        if(host_length <= 64){
            memcpy(tcp_server->host, host, host_length);
        }else{
            return false;
        }

        unsigned int port_length = strlen(port) + 1;
        if(port_length <= 8){
            memcpy(tcp_server->port, port, port_length);
        }else{
            return false;
        }

        return true;
    }

    return false;
}

void setup_server(tcp_server_t* server, int* server_index){
    xSemaphoreTake(config_lock, 0);
    const cJSON *servers = cJSON_GetObjectItem(config, "servers");
    nextArrayElement(servers, server_index, server, fetchServer);
    xSemaphoreGive(config_lock);
}

void setVersion(const char *version){
    xSemaphoreTake(config_lock, 0);
    setOrReplace(config, "version", version);
    xSemaphoreGive(config_lock);
}

const char* getVersion(){
    xSemaphoreTake(config_lock, 0);
    const char* version = getString(config, "version");
    xSemaphoreGive(config_lock);
    return version;
}

const char* getApi(){
    xSemaphoreTake(config_lock, 0);
    const char* api = getString(config, "api");
    xSemaphoreGive(config_lock);
    return api;
}

void setToken(const char *token){
    xSemaphoreTake(config_lock, 0);
    setOrReplace(config, "token", token);
    xSemaphoreGive(config_lock);
}

const char* getToken(){
    xSemaphoreTake(config_lock, 0);
    const char* token = getString(config, "token");
    xSemaphoreGive(config_lock);
    return token;
}

void saveConfig(){
    xSemaphoreTake(config_lock, 0);
    char *json = cJSON_Print(config);
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
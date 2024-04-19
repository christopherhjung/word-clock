//
// Created by Christopher Jung on 19.04.24.
//

#include "api.h"

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "../neopixel/display.h"
#include "../neopixel/renderer.h"

#include <esp_http_server.h>

static const char *TAG="api";

void log_req_host(httpd_req_t *req){
    size_t buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        char* buf = (char*)malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Api Request Host: %s", buf);
        }
        free(buf);
    }
}

bool api_get_param(httpd_req_t *req, const char* key, char* val, size_t val_size)
{
    bool success = false;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        char* buf = (char*)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            success = httpd_query_key_value(buf, key, val, val_size) == ESP_OK;
        }
        free(buf);
    }

    return success;
}

esp_err_t response(httpd_req_t *req, bool success){
    const char* resp_str = success ? "ok": "err";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

esp_err_t api_power_handler(httpd_req_t *req)
{
    char value[32];
    if(api_get_param(req, "status", value, 32)){
        bool next_status = strcmp(value, "on") == 0;
        display_set_power(next_status);
        return response(req, true);
    }else{
        return response(req, false);
    }
}

httpd_uri_t power_uri = {
        .uri       = "/power",
        .method    = HTTP_GET,
        .handler   = api_power_handler,
        .user_ctx  = (void*) 0
};

esp_err_t api_foreground_handler(httpd_req_t *req)
{
    char value[32];
    if(api_get_param(req, "color", value, 32)){
        pixel_t color = parse_pixel(value);
        renderer_set_foreground_color(color);
        return response(req, true);
    }else{
        return response(req, false);
    }
}

httpd_uri_t foreground_uri = {
        .uri       = "/foreground",
        .method    = HTTP_GET,
        .handler   = api_foreground_handler,
        .user_ctx  = (void*) 0
};

esp_err_t api_background_handler(httpd_req_t *req)
{
    char value[32];
    if(api_get_param(req, "color", value, 32)){
        pixel_t color = parse_pixel(value);
        renderer_set_background_color(color);
        return response(req, true);
    }else{
        return response(req, false);
    }
}

httpd_uri_t background_uri = {
        .uri       = "/background",
        .method    = HTTP_GET,
        .handler   = api_background_handler,
        .user_ctx  = (void*) 0
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 1024 * 8;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &power_uri);
        httpd_register_uri_handler(server, &foreground_uri);
        httpd_register_uri_handler(server, &background_uri);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static httpd_handle_t server = NULL;

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void rest_init()
{
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
}

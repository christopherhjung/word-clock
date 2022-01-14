/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <netdb.h>
#include <sys/socket.h>

#include "wifi_connect.h"
#include "../core.h"
#include "../config.h"

static const char *TAG = "tcp client";

[[noreturn]] static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;
    struct in_addr *addr;
    int s;

    tcp_server_t server;
    int current_server = 0;

    while(true) {
        if(wifi_wait()){
            current_server = 0;
        }

        setup_server(&server, &current_server);
        ESP_LOGI(TAG, "Try connect to. IP=%s:%s", server.host, server.port);

        int err = getaddrinfo(server.host, server.port, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        setDescriptor(s);

        initSiiam();

        ESP_LOGI(TAG, "ready");
        runSiiam();

        int error = 0;
        socklen_t len = sizeof (error);
        int retval = getsockopt (s, SOL_SOCKET, SO_ERROR, &error, &len);

        ESP_LOGI("core", "Error connection: %d %d" , error, retval);

        close(s);
        ESP_LOGI(TAG, "Starting again!");
    }
}
void runControlLink(){
    xTaskCreate(&http_get_task, "http_get_task" , 16384 , NULL , 5 , NULL ) ;
}

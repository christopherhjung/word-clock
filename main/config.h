#include "esp_wifi_types.h"

typedef struct {
    const char* host = NULL;
    const char* port = NULL;
} tcp_server_t;

void writeFile(const char* path, const char *buffer);

void readFile(const char* path, char *buffer, int size);

void mount(void);

void setVersion(const char *version);

const char* getVersion();

void setToken(const char* token);

const char* getToken();

const char* getApi();

char* loadFile(const char* path);

void loadConfig();

void saveConfig();

int setupWifi(wifi_config_t *wifi_config);

void listFiles(const char*);

tcp_server_t getServer();
#include "esp_wifi_types.h"

void writeFile(const char* path, const char *buffer);

void readFile(const char* path, char *buffer, int size);

void mount(void);

void setVersion(const char *version);

const char* getVersion();

void setToken(const char* token);

const char* getToken();

char* loadFile(const char* path);

void loadConfig();

void saveConfig();

int setupWifi(wifi_config_t *wifi_config);

void listFiles(const char*);
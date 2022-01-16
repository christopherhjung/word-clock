#include "esp_wifi_types.h"

typedef struct {
    char host[64];
    char port[8];
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

void listFiles(const char*);

void setup_wifi(wifi_config_t *wifi_config, int *wifi_index);

void setup_server(tcp_server_t* server, int *server_index );

void init_config();
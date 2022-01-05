void writeFile(const char* path, const char *buffer);

void readFile(const char* path, char *buffer, int size);

void mount(void);

void setVersion(const char *api);

char* getVersion();

void setToken(const char* token);

char* getToken();

void loadConfig();

void saveConfig();
void writeFile(const char* path, const char *buffer);

void readFile(const char* path, char *buffer, int size);

void mount(void);

void setVersion(const char *version);

const char* getVersion();

void setToken(const char* token);

const char* getToken();

void loadConfig();

void saveConfig();
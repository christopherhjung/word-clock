#include "esp_wifi_types.h"
#include "neopixel/display.h"

void mount(void);

void readFile(const char* path, char *buffer, int size);

void writeFile(const char* path, const char *buffer);

char* loadFile(const char* path);

void config_init();

void config_load();

void setup_wifi(wifi_config_t *wifi_config, int* wifi_index);


void config_set_foreground_color(pixel_t pixel);

pixel_t config_get_foreground_color();

void config_set_background_color(pixel_t pixel);

pixel_t config_get_background_color();

void config_set_max_brightness(float max_brightness);

float config_get_max_brightness();

void config_set_it_is_active(bool status);

bool config_get_it_is_active();

void config_set_power_status(bool status);

bool config_get_power_status();


void config_save();

void listFiles(const char* path);
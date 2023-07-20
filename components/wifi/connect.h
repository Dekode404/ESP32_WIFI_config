#ifndef connect_h
#define connect_h

#include "esp_err.h"
#include "nvs_flash.h"

#define AP_SSID "ESP32"
#define AP_PASS "12345678"

#define STATIC_HOME_PAGE "<!DOCTYPE html><html><body><h2>Set WiFi Credentials</h2><form action='/save_credentials'> <label for='ssid'>SSID:</label><br> <input type='text' id='ssid' name='ssid' required><br> <label for='password'>Password:</label><br> <input type='password' id='password' name='password' required><br><br> <input type='submit' value='Submit'> </form>  </body> </html>"
#define STATIC_SAVE_PAGE "<!DOCTYPE html><html><body><h2>WiFi Credentials saved</h2> </body> </html>"

#ifdef DEBUG_CODE
static const char *TAG = "SERVER";
#endif

// This struct is for the storing the WIFI credentials
typedef struct
{
    char WIFI_SSID[10];     // 10 byte storage for the SSID
    char WIFI_PASSWORD[20]; // 20 byte storage for the password

} WIFI_CREDENTIALS_t;

void wifi_init(void);
esp_err_t wifi_connect_sta(const char *ssid, const char *pass, int timeout);
void wifi_connect_ap(const char *ssid, const char *pass);
void wifi_disconnect(void);
void wifi_destroy_netif(void);

esp_err_t save_the_wifi_credentials_into_NVS(WIFI_CREDENTIALS_t *wifi_credentials);
esp_err_t read_the_wifi_credentials_from_NVS(WIFI_CREDENTIALS_t *wifi_credentials);

esp_err_t set_wifi_credentials_url(httpd_req_t *req);
esp_err_t save_wifi_credentials_url(httpd_req_t *req);

void init_web_server(void);

#endif
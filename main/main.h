#ifndef main_h
#define main_h

#define AP_SSID "ESP32"
#define AP_PASS "12345678"

#define STATIC_HOME_PAGE "<!DOCTYPE html><html><body><h2>Set WiFi Credentials</h2><form action='/save_credentials'> <label for='ssid'>SSID:</label><br> <input type='text' id='ssid' name='ssid' required><br> <label for='password'>Password:</label><br> <input type='password' id='password' name='password' required><br><br> <input type='submit' value='Submit'> </form>  </body> </html>"

#define STATIC_SAVE_PAGE "<!DOCTYPE html><html><body><h2>WiFi Credentials saved</h2> </body> </html>"

#endif
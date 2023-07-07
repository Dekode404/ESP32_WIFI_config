#ifndef main_h
#define main_h

const char set_wifi_credentials_HTML[512] = "<!DOCTYPE html><html><body><h2>Set WiFi Credentials</h2><form action='/save_credentials'> <label for='ssid'>SSID:</label><br> <input type='text' id='ssid' name='ssid'><br> <label for='password'>Password:</label><br> <input type='password' id='password' name='password'><br><br> <input type='submit' value='Submit'> </form>  </body> </html>";

const char save_wifi_credentials_HTML[128] = "<!DOCTYPE html><html><body><h2>WiFi Credentials saved</h2> </body> </html>";

#endif
// #include <Arduino.h>
// #include <WiFiManager.h>
// #include "network.h"

// String DoW[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
// String Months[] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// const char* NTP_SERVER = "ch.pool.ntp.org";
// const char& CONFIG_PORTAL_TIMEOUT = 180;  // seconds

// // Timezone config
// /* 
//   Enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
//   See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
//   based on https://github.com/SensorsIot/NTP-time-for-ESP8266-and-ESP32/blob/master/NTP_Example/NTP_Example.ino
// */
// const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";  // Switzerland
//                       // "EST5EDT,M3.2.0,M11.1.0" // New York

// // Time 
// tm timeinfo;
// time_t now;
// int hour = 0;
// int minute = 0;
// int second = 0;

// // Wifi
// WiFiManager wm;   // looking for credentials? don't need em! ... google "ESP32 WiFiManager"


// String getFormattedDate(){
//   char time_output[30];
//   strftime(time_output, 30, "%a  %d-%m-%y", &timeinfo);
//   return String(time_output);
// }

// String getFormattedTime(){
//   char time_output[30];
//   strftime(time_output, 30, "%H:%M:%S", &timeinfo);
//   return String(time_output);
// }

// void configModeCallback(WiFiManager *myWiFiManager) {
//   Serial.println("Entered config mode");
//   Serial.println(WiFi.softAPIP());
//   Serial.println(myWiFiManager->getConfigPortalSSID());
// }

// bool getNTPtime(int sec) {
//   if (WiFi.isConnected()) {
//     bool timeout = false;
//     bool date_is_valid = false;
//     long start = millis();

//     Serial.println(" updating:");
//     configTime(0, 0, NTP_SERVER);
//     setenv("TZ", TZ_INFO, 1);

//     do {
//       timeout = (millis() - start) > (1000 * sec);
//       time(&now);
//       localtime_r(&now, &timeinfo);
//       Serial.print(" . ");
//       date_is_valid = timeinfo.tm_year > (2016 - 1900);
//       delay(200);
      
//       // TODO: show animation

//     } while (!timeout && !date_is_valid);
    
//     if (!date_is_valid){
//       Serial.println("Error: Invalid date received!");
//       Serial.println(timeinfo.tm_year);
//       return false;  // the NTP call was not successful
//     } else if (timeout) {
//       Serial.println("Error: Timeout while trying to update the current time with NTP");
//       return false;
//     } else {
//       Serial.println("\n[ok] time updated: ");
//       Serial.print("System time is now:");
//       Serial.println(getFormattedTime());
//       Serial.println(getFormattedDate());
//       return true;
//     }
//   } else {
//     Serial.println("Error: Update time failed, no WiFi connection!");
//     return false;
//   }
// }

// boolean ConnectToWifi(boolean force_reset){
//   Serial.print("Connecting to WiFi");
//   WiFi.mode(WIFI_STA);
//   wm.setAPCallback(configModeCallback);

//   if (force_reset){
//     //wm.resetSettings();    // for testing
//     wm.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
//     wm.startConfigPortal("ESP32_DodecaRGB");
//     return false;
//   } else {
//     bool wifi_connected = wm.autoConnect("ESP32_DodecaRGB");
//     int t=0;
//     if (wifi_connected){
//       Serial.println();
//       Serial.println("WiFi connected");
//       Serial.print("IP address: ");
//       Serial.println(WiFi.localIP());
//       Serial.print("MAC address: ");
//       Serial.println(WiFi.macAddress());
//       Serial.print("RSSI: ");
//       Serial.print(WiFi.RSSI());
//       Serial.println("db");

//       delay(500);

//       return true;
//     } else {
//       Serial.println("ERROR: WiFi connect failure");
//       // update fastled display with error message
//       return false;
//     }
//   }
// }
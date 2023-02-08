#include <TFT_eSPI.h>
#include <SPI.h>
#include "Button2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "time.h"

const char* ssid = "SSID";
const char* password = "PASSWORD";
String haToken = "HOMEASSISTANTTOKEN";
const int motionPin1 = 13;
const int motionPin2 = 15; //Not Used

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5*60*60; // UTC -5 Timezone
const int   daylightOffset_sec = 3600;

#define BUTTON_1            35
#define BUTTON_2            0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
TFT_eSprite img = TFT_eSprite(&tft);
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

unsigned long lastMotion; //Keep track of last time motion was detected 
unsigned long firstDetectionTime; //The first timestamp when motion is detected (for the espLCDSleep)
unsigned long sleepDelay = 5000; //Delay before OLED TV screensaver (ms)
unsigned long powerDelay = sleepDelay + 3600000; //Delay before power off OLED TV (ms)
unsigned long espLCDSleep = 5000; //Delay before turning off LCD on ESP sensor when motion is first detected (ms)
int state = 0; // 0 Initialization | 1 Motion Detected (LCD On) | 2 Motion Detected (LCD Off) | 3 No Motion | 4 Screensaver On | 5 Power Off
volatile int motionState1;
volatile int motionState2;

boolean detectMotion = true; //Motion is detected (PIR or mmWave)

//action | 0 Screen ON | 1 Screen OFF | 2 Power
void rest_api_action(int action)
{
    WiFiClient client;
    HTTPClient http;
    int httpResponseCode;
    String url;
    
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("WiFi Disconnected");
      tft.fillScreen(TFT_RED);
      img.setTextColor(TFT_BLACK);
      tft.println("ERROR: WiFi Disconnected");
      delay(5000);
      ESP.restart();
    }

    switch (action) {
        case 0: // Screen On
          Serial.println("Sending Screen ON Request");
          url = "http://192.168.1.3:8123/api/services/webostv/command";
          http.begin(client, url);
          http.addHeader("Authorization", "Bearer " + haToken);
          http.addHeader("Content-Type", "application/json");
          httpResponseCode = http.POST("{\"entity_id\":\"media_player.lg_webos_smart_tv\",\"command\":\"com.webos.service.tvpower/power/turnOnScreen\"}");
          break;
        case 1: // Screen Off
          Serial.println("Sending Screen OFF Request");
          url = "http://192.168.1.3:8123/api/services/webostv/command";
          http.begin(client, url);
          http.addHeader("Authorization", "Bearer " + haToken);
          http.addHeader("Content-Type", "application/json");
          httpResponseCode = http.POST("{\"entity_id\":\"media_player.lg_webos_smart_tv\",\"command\":\"com.webos.service.tvpower/power/turnOffScreen\"}");
          break;
        case 2: // Power
          Serial.println("Sending Power Off Request");
          url = "http://192.168.1.3:8123/api/services/script/lg_and_lights_turn_off";
          http.begin(client, url);
          http.addHeader("Authorization", "Bearer " + haToken);
          http.addHeader("Content-Type", "application/json");
          httpResponseCode = http.POST("");
          break;
        
    }
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    
    http.end();
}

//For Debugging Purposes.
void button_init()
{
   
    btn1.setPressedHandler([](Button2 & b) {
        Serial.println("Right Button Pressed");
        
        //Do Something
    });

    btn2.setPressedHandler([](Button2 & b) {
        Serial.println("Left Button Pressed");
        
        //Do Something
    });
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

// //Legacy code that uses Polling, instead of Interupts to check the motion sensor
// boolean motion_sensor_scan()
// {
//     motionState1 = digitalRead(motionPin1);
//     motionState2 = digitalRead(motionPin2);
//     return motionState1 == HIGH || motionState2 == HIGH;
// }

void IRAM_ATTR motionISR1()
{
    motionState1 = digitalRead(motionPin1);
}

void IRAM_ATTR motionISR2()
{
    motionState2 = digitalRead(motionPin2);
}

void setup()
{
    
    Serial.begin(115200);
    Serial.println("Start");
    
    button_init();
    tft.init();
    img.createSprite(240,135);

    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 2);
    tft.setTextSize(1);

    tft.println("Booting...");

    WiFi.persistent(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    tft.println("Connecting to WiFi");
    int connectTime = millis();
    while(WiFi.status() != WL_CONNECTED) {
      delay(200);
      tft.print(".");
      if (millis() - connectTime >= 5000) { // On the rare occasion it gets stuck trying to connect, a reboot fixes it.
        ESP.restart();
      }
    }

    tft.println("IP Address: ");
    IPAddress ip = WiFi.localIP();    
    tft.println(ip);

    // Enable Arduino OTA updates
    ArduinoOTA.setHostname("oledautosleep-esp32");
    ArduinoOTA.setPassword("password");
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();    
    tft.println("Arduino OTA Enabled");

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    tft.println("Setting up Time Server");

    tft.println("Boot up complete!");
    delay(2000);

    pinMode(motionPin1, INPUT_PULLDOWN);  
    pinMode(motionPin2, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(motionPin1), motionISR1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(motionPin2), motionISR2, CHANGE);
    motionISR1(); //Initialize motionState1 value
    motionISR2(); //Initialize motionState2 value
    lastMotion = millis();
}

void loop()
{
    ArduinoOTA.handle();
    button_loop();
    // detectMotion = motion_sensor_scan(); // Legacy code for my Polling (vs Interupt) implementation.
    detectMotion = motionState1 == HIGH || motionState2 == HIGH;

    if (detectMotion) {
      lastMotion = millis();
    }

    if (!detectMotion && (millis() - lastMotion >= powerDelay)) {
      Serial.println("Time to power off the TV");
      state = 5;
      draw_screen();
      rest_api_action(2);
      delay(10*60*1000); //Wait 10 minutes for power off, if still not off try again. This assumes the ESP is connected to the OLED TV USB power source, such that it cuts power when the TV is Off.
       
    } else if (!detectMotion && (millis() - lastMotion >= sleepDelay)) {
      if (state != 4) {
        Serial.println("Turning Screensaver On");
        state = 4;
        rest_api_action(1);
      }
      draw_screen();

    } else {
      if (detectMotion) {
        if (state != 1 && state != 2) { //Not already in a motion detected state
          Serial.println("Motion Detected");
          state = 1; 
          firstDetectionTime = millis();
          rest_api_action(0);
        }
        if (state == 1 && (millis() - firstDetectionTime >= espLCDSleep)) { 
          state = 2;
          //Legacy Code: After a set time being in the Motion Detected state Turn LCD Off on ESP Sensor.
          // Serial.println("Turning OFF ESP32 LCD Backlight");
          // int r = digitalRead(TFT_BL);
          // digitalWrite(TFT_BL, !r);
          // tft.writecommand(TFT_DISPOFF);
        }
        draw_screen();

      } else { //No longer detecting motion but it hasn't been long enough to go to Screensaver or Power Off State.
        if (state == 2) { 
          //Legacy Code: LCD was previously off, turn it back on
          // Serial.println("No Motion Detected - Turning ON ESP32 LCD Backlight");
          // int r = digitalRead(TFT_BL);
          // digitalWrite(TFT_BL, !r);
          // tft.writecommand(TFT_DISPON);
        }
        state = 3;
        draw_screen();
      }
    }
}

void draw_screen() {
    
  img.fillRect(0,0,240,135,TFT_BLACK);
  img.setCursor(0, 0, 2);
  img.setTextColor(TFT_WHITE);
  img.setTextSize(2);

  switch (state) {
    case 0:
      break;
    case 1:
      draw_border(TFT_GREEN);
      img.setTextDatum(MC_DATUM);
      img.drawString("Motion Detected", 240/2, 135/2);
      break;
    case 2: //LCD Screensaver while actively on OLED TV
      struct tm timeinfo;
      if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        img.println("Issue trying to get time..");
        break;
      }
      img.setTextColor(TFT_DARKGREY);
      img.setCursor(15,10);
      img.println(&timeinfo, "%A");
      img.setCursor(15,40);
      img.println(&timeinfo, "%B %d");
      img.setTextSize(3);
      img.setCursor(15,75);
      img.println(&timeinfo, "%I:%M %p");
      break;
    case 3:
      draw_border(TFT_YELLOW);
      img.setCursor(15,10);
      img.println("No Motion");
      img.setCursor(15,40);
      img.println("Screensaver in:");
      img.setTextSize(3);
      img.setCursor(15,75);
      img.println((lastMotion + sleepDelay - millis())/1000 + 1);
      break;
    case 4:
      draw_border(TFT_RED);
      img.setCursor(15,10);
      img.println("Screensaver On");
      img.setCursor(15,40);
      img.println("Shut Down in:");
      img.setCursor(15,75);
      img.setTextSize(3);
      img.println((lastMotion + powerDelay - millis())/1000 + 1);
      break;
    case 5:
      draw_border(TFT_VIOLET);
      img.setTextDatum(MC_DATUM);
      img.drawString("Shutting Down TV", 240/2, 135/2);
      break;
  }

  img.pushSprite(0,0);

}

void draw_border(uint32_t color) {
  img.fillRect(0, 0, 240, 4, color);
  img.fillRect(0, 0, 4, 135, color);
  img.fillRect(0, 130, 240, 4, color);
  img.fillRect(235, 0, 4, 135, color);

}

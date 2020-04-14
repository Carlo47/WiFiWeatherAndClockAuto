/** 
  * Sketch     WiFiWeatherAndClockAuto.cpp
  * Author     2019-02-15 Ch. Geiser   
  * Purpose    Connects via WLAN to openWeatherMap.org and fetches the weather data 
  *            in JSON format, parses the strings with the JSON parser and displays 
  *            the data on the integrated OLED display of the ESP32 module.
  *            Date and Time are also retrieved from a time server.
  * Board      ESP32-Shield_WiFi / Bluetooth / OLED / 18650 Li-Ion
  * Reference  https://www.youtube.com/watch?v=zYfWcEgPi2g
  *            https://techtutorialsx.com/2018/03/17/esp32-arduino-getting-weather-data-from-api/
  *            https://www.hackster.io/hieromon-ikasamo/esp8266-esp32-connect-wifi-made-easy-d75f45
  *            https://openweathermap.org/guide
  * Remarks    To be able to access the owm-Api, you have to register and get a key.  
  */
#include <Arduino.h>
#include "WiFiWeatherAndClockAuto.h"

const String owmKey        = "2359ecb608ce6cf3b72a0d1b2bc3930a"; // Get your own key
const String cityCodeBaden = "2661646";
const String cityCodeBrugg = "2661374";
const String cityCodeZuoz  = "2657898";

// Initialize the OLED display using Wire library
#define I2C_ADDR  0x3c
#define PIN_SDA   5
#define PIN_SCL   4

struct tm rtcTime;
bool      setupOK;
OwmData   owmData;

WebServer     Server;
AutoConnect   Portal(Server);
AutoConnectConfig Config;
NtpClock      myNtpClock(NTP_SERVER_POOL, TZ_INFO, rtcTime);
Owm           myOwm(owmKey, cityCodeBaden, owmData);
SSD1306Wire   display(I2C_ADDR, PIN_SDA, PIN_SCL);


/**
  * Displays the weather data on the oled
  */
void displayOwmData(OwmData &owmdata)
{
  char buf[64];
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, owmdata.city.c_str());
  display.drawString(46, 0, owmdata.description.c_str());
  strftime(buf, 64, "Rise %H:%M", &owmdata.struct_sunrise);
  display.drawString(0, 12, buf);
  strftime(buf, 64, "Set %H:%M", &owmdata.struct_sunset);
  display.drawString(64, 12, buf);
  sprintf(buf, "%4.1f C  %2.0f %%  %4.0f hPa", owmdata.actTempC, owmdata.relHum, owmdata.preshPa);
  display.drawString(0, 24, buf); 
  sprintf(buf, "Wind  %3.0f m/s  %3d Grad", owmdata.windSpeed, owmdata.windDirection);
  display.drawString(0, 36, buf);
  getLocalTime(&rtcTime);
  strftime(buf, 64, "%F    %H:%M", &rtcTime);
  display.drawString(0, 48, buf);
  display.display();  
}

/**
  * Displays connection details of the WiFi connection
  * SSID, IP, MAC, RSSI
  */
void displayConnectionDetails()
{
  char buf[32];
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  sprintf(buf, "%s", WiFi.SSID().c_str());
  display.drawString(64,0, buf);
  sprintf(buf, "%s", WiFi.localIP().toString().c_str());
  display.setFont(ArialMT_Plain_10);
  display.drawString(64,18, buf);
  display.setFont(ArialMT_Plain_10);
  sprintf(buf, "%s", WiFi.macAddress().c_str());
  display.drawString(64,34, buf);
  sprintf(buf, "RSSI %d", WiFi.RSSI());
  display.drawString(64,50, buf);
  display.display();  
}

/**
  * Gets time and date every everySecs seconds from the RTC 
  * and displays the values on the oled
  */
void displayTime(struct tm &rtctime, unsigned long everySecs)
{
  char buf[40];
  static unsigned long msPrevious = millis();
  if (millis() > msPrevious + everySecs * 1000)
  {
    getLocalTime(&rtctime);
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "NTP-Clock");
    strftime(buf, 40, "%T", &rtctime); // Zeit  als hh:mm:ss
    display.drawString(64, 18, buf);
    strftime(buf, 40, "%F", &rtctime); // Datum als yyyy-mo-dd
    display.drawString(64, 36, buf);
    display.display();
  }
}


/**
  * Cyclically generates a task number from 0 to nbrOfTasks-1 
  * and returns the next one everySecs seconds. Is used to 
  * cyclically display either connection date, date and time or 
  * the weather data.
  */
int selectTask(int nbrOfTasks, int everySecs)
{
  static int taskNbr = 0;
  static unsigned long msPrevious = millis();
  if (millis() > (msPrevious + everySecs * 1000))
  {
    msPrevious = millis();
    taskNbr = ((taskNbr + 1) % nbrOfTasks);
  }
  return taskNbr;
}

void setup() 
{
  Serial.begin(COMPORT_SPEED);
  display.init();
  Config.autoReconnect = true;
  Portal.config(Config);

  if (Portal.begin()) 
  {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    myNtpClock.setVerbose(false);
    myNtpClock.setResyncInterval(6*60*60);  // Resync every 6 hours
    myNtpClock.setup();
    myOwm.setup();
    setupOK = true;   
  }
  else
    setupOK = false;
}

void loop()
{
  if (! setupOK)
    {
      display.clear();
      display.drawString(64, 0, "Restart");
      delay(5000);
      ESP.restart();
    }
  else
    {
      myNtpClock.loop();            // Look whether RTC must be synchronized with NTP
      myOwm.loop();                 // Get weather data 
      switch (selectTask(3, 10))    // Switch every 10 sec to display connectionDetails, 
      {                             // date / time and weather data
        case 0:
          displayConnectionDetails();
        break;
        case 1:
          displayTime(rtcTime, 1); // Display time every second
        break;
        case 2:
          displayOwmData(owmData);
        break;   
      }
    }
  delay(200);                      // Pass some time away 
}

/*
owmPayload to be parsed:
{
"coord":{"lon":8.31,"lat":47.47},
"weather":[{"id":800,"main":"Clear","description":"Klarer Himmel","icon":"01n"}],
"base":"stations",
"main":{"temp":1.99,"pressure":1035,"humidity":76,"temp_min":1,"temp_max":3},
"visibility":10000,
"wind":{"speed":0.5},
"clouds":{"all":0},
"dt":1550173800,
"sys":{"type":1,"id":6941,"message":0.0038,"country":"CH","sunrise":1550125987,"sunset":1550162962},
"id":2661646,
"name":"Baden",
"cod":200
}
 */

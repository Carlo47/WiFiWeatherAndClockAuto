/**
  * The "class NtpClock" synchronizes the RTC of the ESP32 with an NTP server. 
  * Before setup () and loop () are called, a WLAN internet connection must be 
  * established. The synchronization interval is set to once a day. However, 
  * the user can specify a different value. The current time is stored in the 
  * struct tm rtcTime data structure.
  */
#include "NtpClock.h"

extern const char* TZ_INFO;
extern const char* NTP_SERVER_POOL;

/**
  * setup () synchronizes the RTC with the NTP server. If the keepWLan flag 
  * is not set, the WLAN connection is closed.
  */
void NtpClock::setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  configTzTime(_tzInfo, _ntpPool);
  Serial.printf("RTC synchronized with %s\r\n", _ntpPool);
  Serial.printf("  for timezone: %s\r\n", _tzInfo);
  printLocalTime();
  if (! _keepWlan)
  {
    WiFi.disconnect(false); // WLAN-Connection no longer needed, RTC is set and runs 
                            // autonomously. Keep ssid and key for later reconnection.
                            // Hourly resynchronization by sntp.lib is no longer possible.      
    Serial.printf("Disconnect from WLAN. WiFi state: %d\r\n", WiFi.status());
  }
}

/**
  * loop () calls the synchronization with the NTP server periodically 
  * after the set number of seconds
  */
void NtpClock::loop() 
{
  if (millis() > (_msPrevious + _msResyncInterval))
  {
    _resyncRTC();
    _msPrevious = millis();
    if (_verbose) 
    {
      printLocalTime();
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle LED on each resync
    }
  }
}

/**
  * Sets the interval for the synchronization of the RTC with the NTP server
  */
void NtpClock::setResyncInterval(unsigned long secs)
{
  _msResyncInterval = secs * 1000;
}

/** 
  *  Aktiviert mehr Ausgaben auf den seriellen Monitor
  */
void NtpClock::setVerbose(bool v)
{
  _verbose = v;   
}

/**
  * Retrieves the local time from the RTC and outputs it 
  * to the serial monitor in various formats.
  */
void NtpClock::printLocalTime()
{
  getLocalTime(&_rtcTime);
  Serial.println(&_rtcTime, "%B %d %Y %H:%M:%S (%A)");  // January 15 2019 16:33:20 (Tuesday)
  Serial.println(&_rtcTime, "%c");                      // Tue Jan 15 16:33:20 2019
  Serial.println(&_rtcTime, "%F %T");                   // 2019-01-15 16:33:20
  Serial.println(&_rtcTime, "%F %T %W/%w %Z %z");       // 2019-01-15 16:51:18 02/2 MEZ +0100
                                                        //                     ^  ^  ^    ^ 1h Offset zu UTC
                                                        //                     ^  ^  ^ Zeitzone
                                                        //                     ^  ^ Wochentag (So = 0)
                                                        //                     ^ Woche (mit Montag als 1. Tag der Woche 1)   
  Serial.println();
}

/**
  * Sets the flag as to whether the WLAN connection should be maintained  
  * after the RTC synchronization with the NTP server or not.
  */
void NtpClock::keepWiFiConnection(bool keep)
{
  _keepWlan = keep;
}

/**
  * Restores the WiFi connection and synchronizes the RTC with the NTP server. 
  * If the keepWlan flag is not set, the  WLAN connection closed again and the
  * RTC runs autonomously. If the flag is set and the WLAN connection is 
  * established, the sntp.lib resynchronizes every hour.
  */
void NtpClock::_resyncRTC()
{
  byte retries = 0;
  if (_verbose)
  {
    Serial.printf("Resynchronizing clock with NTP. WiFi state: %d\r\n", WiFi.status());
  }
  while (!WiFi.isConnected() && retries < 10)
  {
    WiFi.reconnect();
    delay(200);
    retries++;
  }
  if (retries >= 10)
  {
    Serial.printf("RTC resynchronization failed after %d retries, try on next cycle\r\n", retries);
  }
  else
  {
    configTzTime(TZ_INFO, NTP_SERVER_POOL); // Synchronize RTC with NTP server
    if (_verbose)
      Serial.printf("RTC resyncronized with NTP after %d retries. WiFi state: %d\r\n", retries, WiFi.status());
    if (!_keepWlan)
    {
      WiFi.disconnect(false);
      if (_verbose)
        Serial.printf("WiFi disconnected. WiFi state: %d\r\n", WiFi.status());
    }
  }
}

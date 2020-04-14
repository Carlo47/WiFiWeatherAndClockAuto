#ifndef NTPCLOCK_H
#define NTPCLOCK_H
#include <WiFiMulti.h>

class NtpClock
{
  public:
    NtpClock(const char *ntpPool, const char *tzInfo, struct tm &rtcTime) :
      _ntpPool(ntpPool),
      _tzInfo(tzInfo),
      _rtcTime(rtcTime)
    { 
    }

    /**
      * setup () synchronizes the RTC and closes the WiFi connection if
      * keepWlan flag is not set
      */
    void setup();

    /**
      * loop () calls non-blocking and periodically the synchronization with 
      * the NTP server after the expiration of the set number of seconds
      */
    void loop();

    /**
      * Sets the interval for the synchronization of the RTC with the NTP server
      */
    void setResyncInterval(unsigned long secs);

    /** 
      * Enables more output on the serial monitor
      */
    void setVerbose(bool v);

    /**
      * Retrieves the local time from the RTC and outputs it  
      * to the serial monitor in various formats. 
      */
    void printLocalTime();

    /**
      * Sets the flag whether the WLAN connection to NTP server should be  
      * maintained or not after the RTC synchronization 
      */
    void keepWiFiConnection(bool keep);

  private:
    bool _verbose = false;
    bool _keepWlan = true;
    const char *_ntpPool;
    const char *_tzInfo;
    unsigned long _msResyncInterval = 24*60*60*1000; // Default-ReSyncIntervall 1 x täglich
    unsigned long _msPrevious = millis();
    struct tm &_rtcTime;

    /**
      * Restores the WiFi connection and synchronizes the RTC with the NTP server. 
      * Then the WiFi connection is closed and the RTC continues to run autonomously.
      */
    void _resyncRTC();
};

#endif
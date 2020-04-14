#include <SSD1306Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include "NtpClock.h"
#include "Owm.h"

#define COMPORT_SPEED     115200

const char* NTP_SERVER_POOL = "ch.pool.ntp.org";
const char* TZ_INFO         = "MEZ-1MESZ-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; 
const char* TZ_INFO1        = "EST5EDT4,M3.2.0/02:00:00,M11.1.0/02:00:00";

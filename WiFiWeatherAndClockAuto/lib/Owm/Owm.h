#ifndef OWM_H
#define OWM_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

typedef struct
  {
    String city;
    String country;
    String description;
    time_t datetime;
    time_t sunrise;
    time_t sunset;
    float actTempC;
    float minTempC;
    float maxTempC;
    float relHum;
    float preshPa;
    float windSpeed;
    int windDirection;
    tm struct_datetime;
    tm struct_sunrise;
    tm struct_sunset;
  } OwmData;

extern OwmData owmData;

class Owm
{
  public:
    Owm(String owmkey, String citycode, OwmData &owmdata) :
      _owmKey(owmkey),
      _cityCode(citycode),
      _owmData(owmdata)
    {
    }

    void setup();
    void loop();
    void printOwmData(OwmData &owmdata);

  private:
    void _parsePayload(OwmData &owmdata);
    void _composeOwmEndPoint();
    void _getOwmData(/*unsigned long everySecs*/);
    unsigned long _msPrevious = millis();
    unsigned long _everySecs = 60;
    String _owmKey;
    String _cityCode;
    OwmData &_owmData;
    String _owmEndpoint;
    String _owmPayload;
    HTTPClient _http;
};
#endif

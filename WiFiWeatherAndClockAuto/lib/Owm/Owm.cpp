#include "Owm.h"

/**
  * Decodes the weather data and fills the individual values ​​
  * into the OwmData structure
  */
void Owm::_parsePayload(OwmData &owmdata)
{
  char jsonArray[_owmPayload.length()+1];
  _owmPayload.toCharArray(jsonArray,sizeof(jsonArray)); 
  jsonArray[_owmPayload.length() + 1] = '\0';

  StaticJsonDocument<1024> jsonDoc;
  DeserializationError err = deserializeJson(jsonDoc, jsonArray);
  if (! err)
  {
    String city = jsonDoc["name"];                              // Ort
    owmdata.city = city;
    String country = jsonDoc["sys"]["country"];                 // Land
    owmdata.country = country;
    owmdata.datetime = jsonDoc["dt"];                           // date and time
    owmdata.sunrise = jsonDoc["sys"]["sunrise"];                // Sonnenaufgang UTC UNIX timestamp
    owmdata.sunset = jsonDoc["sys"]["sunset"];                  // Sonnenuntergang UTC UNIX timestamp
    owmdata.actTempC = jsonDoc["main"]["temp"];                 // °C aktuelle Temperatur
    owmdata.minTempC = jsonDoc["main"]["temp_min"];             // °C mögliche lokale Abweichung
    owmdata.maxTempC = jsonDoc["main"]["temp_max"];             // °C mögliche lokale Abweichung
    owmdata.relHum = jsonDoc["main"]["humidity"];               // Luftfeuchte %
    owmdata.preshPa = jsonDoc["main"]["pressure"];              // Luftdruck hPa
    String description = jsonDoc["weather"][0]["description"];  // Bewölkung
    owmdata.description = description;
    owmdata.windSpeed = jsonDoc["wind"]["speed"];               // m/s
    owmdata.windDirection = jsonDoc["wind"]["deg"];             // von Nord im Uhrzeigersinn
    gmtime_r(&owmdata.datetime, &owmdata.struct_datetime);
    gmtime_r(&owmdata.sunrise, &owmdata.struct_sunrise);
    gmtime_r(&owmdata.sunset, &owmdata.struct_sunset);
  }
  else
  {
    Serial.print("deserializeJson failed with code: ");
    Serial.println(err.c_str());
  }
}

/**
  * Outputs the weather data on the serial monitor
  */
void Owm::printOwmData(OwmData &owmdata)
{
  Serial.println(&owmdata.struct_datetime, "%d-%m-%Y %H:%M");
  Serial.printf("%s %s\t%s\r\nAufgang\t\t%02d:%02d UTC\r\nUntergang\t%02d:%02d UTC\r\nTemperatur\t%-4.1f°C\r\nLuftfeuchte\t%-2.0f%%\r\nLuftdruck\t%4.0f hPa\r\nWind\t\t%-3.0f m/s\r\nRichtung\t%d°\r\n",
                owmdata.city.c_str(), 
                owmdata.country.c_str(), 
                owmdata.description.c_str(), 
                owmdata.struct_sunrise.tm_hour, 
                owmdata.struct_sunrise.tm_min, 
                owmdata.struct_sunset.tm_hour, 
                owmdata.struct_sunset.tm_min, 
                owmdata.actTempC, 
                owmdata.relHum, 
                owmdata.preshPa, 
                owmdata.windSpeed, 
                owmdata.windDirection); 
}

/**
  * Constructs the URL to the weather data
  */
void Owm::_composeOwmEndPoint()
{
  _owmEndpoint = "http://api.openweathermap.org/data/2.5/weather?id=" + _cityCode + "&lang=de" + "&units=metric&APPID=" + _owmKey;
  Serial.printf("EP: %s\r\n", _owmEndpoint.c_str());
}

void Owm::_getOwmData(/*unsigned long everySecs*/)
{
  byte retries = 0;

  while (! WiFi.isConnected() && retries < 10)
  {
    WiFi.reconnect();
    delay(500);
    retries++;
    Serial.printf("WLAN disconnected, try to reconnect: %d\r\n", retries);
  }

  if(retries >= 10)     
  {
    Serial.println("WLAN connection lost, restarting");
    delay(5000);
    ESP.restart();
  }
  else
  {
    retries = 0;
    _http.begin(_owmEndpoint);    //Specify the URL
    if (_http.GET() > 0)
    { 
      _owmPayload = _http.getString();
      //Serial.println(httpCode);
      //Serial.println(owmPayload);
      _parsePayload(_owmData);
      //printOwmData(owmdata);
    }
    else
    {
      Serial.println("Error on HTTP request");
    }
    _http.end(); //Free the resources
  }
}

void Owm::setup()
{
  _composeOwmEndPoint();
  _getOwmData();
}  

void Owm::loop()
{
  if (millis() > _msPrevious + _everySecs * 1000)
  { 
    _msPrevious = millis();
    _getOwmData();
  }  
}
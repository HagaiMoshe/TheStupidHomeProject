/* boiler
 * version 3 - using ESP8266 over firebase
 */

// arduino ide does not support relative path.
#define DEBUG_PRINT
#include "C:\Users\Nachom\Desktop\stuff\code\theStupidHomeProject\espProjects\common.h"
#include <Time.h>
#include <TimeLib.h>
int disable_fg_check = 1;
typedef enum{
  dbStat_on,
  dbStat_off,
  dbStat_none,
  dbStat_err
}dbStat_t;

const int RELAY_PIN = 3; // RX - we use it as the relay control pin
//const int RELAY_PIN2 = 2;
const char* INTERNAL_DATABASE_NAME = "boilerInternal.json";
const char* DISPLAY_DATABASE_NAME = "boiler.json";
const int maxDisconnectedSecondsAllowed = 60 * 2;
const int maxBoilerOnSeconds = 60 * 30;
static String idToken = "";
// -----------------------------------------------------------------

dbStat_t checkDataBase()
{
  String readString = getValueFromDb(INTERNAL_DATABASE_NAME, idToken);

  //parse the response
  if(readString.indexOf("true") >=0){
    updateValueToDb(INTERNAL_DATABASE_NAME, "\"status\":\"NONE\"", idToken);
    return dbStat_on;
  }
  if(readString.indexOf("false") >=0){
    updateValueToDb(INTERNAL_DATABASE_NAME, "\"status\":\"NONE\"", idToken);
    return dbStat_off;
  }
  if(readString.indexOf("NONE") >=0)
    return dbStat_none;

  return dbStat_err; // server is unreacable.
}

void setBoiler(dbStat_t dbStat)
{
  static int boilerOnTimeStamp = {0};
  static int startDisconnectedTime = 0;
  const int currentTime = now();

  if(dbStat == dbStat_err){
    startDisconnectedTime = (startDisconnectedTime) ? startDisconnectedTime : currentTime;
    if(currentTime - startDisconnectedTime > maxDisconnectedSecondsAllowed){
      dbStat = dbStat_off;
      Serial.println(String("no network connection for ") + (currentTime - startDisconnectedTime) + "seconds");
    }
  }
  else
    startDisconnectedTime = 0;

  switch(dbStat){
    case dbStat_on:
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("setting boiler on");
      boilerOnTimeStamp = currentTime;
      updateValueToDb(DISPLAY_DATABASE_NAME, "\"isOn\":true", idToken);
      break;

    case dbStat_none:
      if(!boilerOnTimeStamp)
        break;
      if((currentTime - boilerOnTimeStamp) < maxBoilerOnSeconds)
        break;
      else
        Serial.println(String("timeout: boiler is on for ") + (currentTime - boilerOnTimeStamp) + "seconds");

    case dbStat_off:
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("setting boiler off");
      boilerOnTimeStamp = {0};
      updateValueToDb(DISPLAY_DATABASE_NAME, "\"isOn\":false", idToken);
      break;
  }
}

void setup(void)
{  
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY); //rx pin is used for relay control
  setupWifi();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  delay(1000);
  
  idToken = httpsAuthenticate();
  setBoiler(dbStat_off);
}// setup


void loop() {
  if(WiFi.status() != WL_CONNECTED)
    connectToWifi();

  dbStat_t dbStat = checkDataBase();
  setBoiler(dbStat);
  delay(MILLISEC_TO_SEC * 2);
}// loop

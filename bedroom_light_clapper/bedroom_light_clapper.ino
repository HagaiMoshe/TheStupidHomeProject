/* turn on the bed light using hands clapping
 */

#include <Time.h>
#include <TimeLib.h>

//KY-038

const int configModeTime = 7;
const int configModeGnd = 6;

const int relayControrl2 = 5;
const int relayControrl1 = 4;

//const int sensorA0 = 5;
//const int sensorGnd = 4;
const int senosrVcc = 2;
const int sensorD0 = 3;

const unsigned long sonicPulseLength = 20;// microseconds
const unsigned long minDelayBetweenClaps = 100; //millisonconds
const unsigned long maxDelayBetweenClaps = 300; //millisonconds



void setup() {
  Serial.begin(115200);
  delay(300);

  //pinMode(sensorA0, INPUT);
  //pinMode(sensorGnd, OUTPUT);
  //digitalWrite(sensorGnd, LOW);
  pinMode(senosrVcc, OUTPUT);
  digitalWrite(senosrVcc, HIGH);
  
  pinMode(sensorD0, INPUT);
  
  pinMode(relayControrl1, OUTPUT);
  digitalWrite(relayControrl1, LOW);
  pinMode(relayControrl2, OUTPUT);
  digitalWrite(relayControrl2, LOW);

  //pinMode(configVcc, OUTPUT);
  //digitalWrite(configVcc, HIGH);
  pinMode(configModeGnd, OUTPUT);
  digitalWrite(configModeGnd, LOW);
  pinMode(configModeTime, INPUT_PULLUP);

  Serial.println("started");
}


static inline bool detectShortPulse()
{
  if(digitalRead(sensorD0)== 1){
    delay(sonicPulseLength);
    if(digitalRead(sensorD0)== 0)
      return true;
  }
  return false;
}


static inline bool detectClap()
{
  // identify first clap
  if(false == detectShortPulse())
    return false;

  // detect the delay between the two claps
  unsigned long startTime = millis();
  while((millis() - startTime) < minDelayBetweenClaps)
    if(digitalRead(sensorD0)== 1)
      return false;

  // detect the second clap
  startTime = millis();
  while((millis() - startTime) < maxDelayBetweenClaps)
    if(true == detectShortPulse())
      return true;

  return false;
}


void loop() {
  static boolean LEDStatus=false; 
  if(detectClap() == true){

    if(LEDStatus==false){
        LEDStatus=true;
        digitalWrite(relayControrl1,HIGH);
        digitalWrite(relayControrl2,HIGH);
        Serial.println("relay on");
    }
    else{
        LEDStatus=false;
        digitalWrite(relayControrl1,LOW);
        digitalWrite(relayControrl2,LOW);
        Serial.println("relay off");
    }
  }
}

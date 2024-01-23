/* proximity sensor for automaticaly turning on the hallway light
 * using the HC-SR04 sensor
 */

#include <Time.h>
#include <TimeLib.h>



const int configModeTime = 9;
const int configModeDistance = 8;
const int configModeVcc = 7;
const int SignalPin1 = 6;
const int SignalPin2 = 5;
const int SR04_vccPin = 4;
const int SR04TrigPin = 3;
const int SR04EchoPin = 2;


//const int SR04Gnd = 7;


const int trigerLength = 20;// microseconds
unsigned long EchoLength = 10000;// microseconds
int timeOn = 40; // seconds

typedef enum
{
  OFF = 0,
  ON = 1,
}e_ON_OFF;

void setup() {
  Serial.begin(115200);
  delay(300);
 
  pinMode(SignalPin1, OUTPUT);
  pinMode(SignalPin2, OUTPUT);
  pinMode(SR04_vccPin, OUTPUT);
  pinMode(SR04TrigPin, OUTPUT);
  pinMode(SR04EchoPin, INPUT);
  //pinMode(SR04Gnd, INPUT);

  pinMode(configModeVcc, OUTPUT);
  pinMode(configModeDistance, INPUT_PULLUP);
  pinMode(configModeTime, INPUT_PULLUP);
  
  digitalWrite(SR04_vccPin, HIGH);
  digitalWrite(configModeVcc, HIGH);

  Serial.println("started");
}

static inline void MultyDelay(unsigned int microseconds)
{
  unsigned int milliseconds = microseconds / 1000;
  microseconds = microseconds - (milliseconds*1000);

  delayMicroseconds(microseconds);
  delay(milliseconds);
}

static inline void setSignal(bool state)
{
  Serial.print("change mode: ");
  Serial.println(state);
  digitalWrite(SignalPin1, state);
  digitalWrite(SignalPin2, state);
}

static inline void sendTriger()
{
  digitalWrite(SR04TrigPin, HIGH);
  delayMicroseconds(trigerLength);
  digitalWrite(SR04TrigPin, LOW);
}

static inline e_ON_OFF ReadEcho()
{
  while(LOW == digitalRead(SR04EchoPin));
  MultyDelay(EchoLength);
  return (HIGH == digitalRead(SR04EchoPin)) ? ON:OFF;  
}

void configDistance()
{
  while(LOW == digitalRead(configModeDistance));
  delay(5000); //time to set the wanted distance

  // start the measurment
  sendTriger();
  while(LOW == digitalRead(SR04EchoPin));
  const unsigned long startEchoTime = micros();
  while(HIGH == digitalRead(SR04EchoPin));
  EchoLength = micros() - startEchoTime;

  Serial.print("distance config to ");
  Serial.println(EchoLength);
}

void configTime()
{
  const time_t startTime = now();
  while(LOW == digitalRead(configModeTime));
  timeOn = second(now()) - second(startTime);
  delay(1000);// allow disconnect the gpio safely
  Serial.print("distance time to ");
  Serial.println(timeOn);
}

void configMode()
{
  if(LOW == digitalRead(configModeDistance))
    configDistance(); // start distance configuration.
  
  if(LOW == digitalRead(configModeTime))
    configTime(); // start 'on time' configuration
}

static time_t startTime = {0};
void loop() {
  delay(100);

  configMode();
  
  sendTriger();
  if(OFF == ReadEcho())
  {
    startTime = now();
    setSignal(ON);
  }

  if( (startTime != (time_t){0}) &&((now() - startTime) > timeOn)){
    setSignal(OFF);
    startTime = {0};
  }
}



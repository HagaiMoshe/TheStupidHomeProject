/* control the Air condition
 * using ESP8266 over FireBase
 */

#include <IRremoteESP8266.h>
#include <IRsend.h>
int disable_fg_check = 1;
#define DEBUG_PRINT
// arduino ide does not support relative path.
#include "C:\Users\ITZIK\Desktop\stuff\code\theStupidHomeProject\espProjects\common.h"

const char* displayDataBaseName = "livingAir.json";
const char* internalDataBaseName = "livingAirInternal.json";

//ir codes
unsigned short statusCheckCode[] = {860, 860, 860, 860, 1700, 860, 860, 860, 860, 860, 860, 860, 860, 860, 860, 1700, 1700, 860, 860, 860, 860, 860, 860};
unsigned short offCode[] = {7828, 4288, 1588, 544, 492, 1632, 496, 1628, 496, 1636, 492, 1636, 492, 1636, 492, 1648, 492, 1636, 1588, 540, 496, 1636, 492, 1636, 496, 1636, 492, 1632, 492, 1636, 1588, 540, 492, 1636, 496, 1628, 1588, 540, 496, 1632, 492, 1636, 1588, 540, 1588, 536, 496, 1636, 492, 1636, 496, 1636, 492, 1636, 492, 1636, 496, 1628, 496, 1632, 496, 1632, 496, 1632, 492, 1636, 492, 1632, 496, 1636, 492, 1636, 496, 1632, 496, 1632, 496, 1632, 492, 1636, 496, 1636, 492, 1636, 492, 1636, 496, 1636, 492, 1636, 492, 1636, 492, 1636, 1588, 540, 1588, 544, 492, 1636, 492, 1636, 496, 1632, 496, 1632, 496, 1636, 492, 1636, 1588, 544, 1588, 544, 1588, 544, 1588, 540, 496, 1636, 496, 1636, 496, 1636, 1588, 540, 496, 1632, 496, 1636, 1588, 30392, 7828, 4288, 1588, 544, 492, 1636, 496, 1628, 496, 1636, 496, 1640, 496, 1636, 496, 1636, 496, 1636, 1588, 544, 496, 1636, 496, 1636, 492, 1640, 492, 1640, 492, 1640, 1588, 544, 492, 1636, 496, 1640, 1584, 548, 492, 1640, 492, 1640, 1588, 540, 1588, 544, 496, 1632, 496, 1640, 492, 1640, 492, 1640, 492, 1636, 496, 1636, 496, 1640, 492, 1636, 496, 1636, 496, 1636, 496, 1640, 492, 1636, 496, 1636, 496, 1636, 496, 1636, 496, 1640, 496, 1636, 492, 1640, 496, 1632, 496, 1636, 488, 1652, 492, 1640, 492, 1644, 492, 1640, 1584, 548, 1584, 548, 492, 1636, 492, 1640, 492, 1640, 492, 1640, 496, 1636, 496, 1636, 1588, 548, 1584, 548, 1584, 544, 1588, 544, 492, 1640, 492, 1640, 492, 1644, 1584, 544, 496, 1636, 496, 1640, 1584};
unsigned short coldCode[] = {7804, 4316, 1592, 540, 472, 1656, 468, 1660, 472, 1660, 472, 1660, 472, 1660, 468, 1664, 468, 1660, 1564, 568, 472, 1660, 472, 1660, 472, 1660, 472, 1656, 472, 1660, 1564, 568, 496, 1636, 496, 1636, 468, 1660, 1592, 540, 1592, 540, 472, 1660, 1564, 568, 472, 1656, 472, 1660, 472, 1656, 472, 1656, 472, 1660, 472, 1660, 472, 1656, 472, 1664, 472, 1660, 468, 1664, 472, 1660, 492, 1636, 472, 1660, 496, 1636, 472, 1656, 472, 1660, 472, 1660, 468, 1664, 496, 1632, 472, 1664, 472, 1660, 472, 1660, 1564, 568, 1564, 564, 472, 1660, 468, 1664, 472, 1656, 472, 1660, 472, 1660, 472, 1660, 496, 1628, 472, 1664, 1588, 540, 1564, 572, 1564, 564, 1564, 568, 496, 1636, 496, 1636, 472, 1660, 1564, 568, 496, 1636, 468, 1664, 1564, 30416, 7804, 4312, 1564, 568, 496, 1628, 496, 1640, 472, 1656, 472, 1660, 496, 1636, 472, 1660, 468, 1656, 1564, 572, 472, 1660, 496, 1636, 472, 1660, 496, 1636, 476, 1656, 1588, 544, 468, 1664, 468, 1660, 472, 1660, 1560, 568, 1588, 544, 472, 1660, 1564, 560, 472, 1664, 468, 1660, 472, 1660, 472, 1656, 472, 1660, 496, 1636, 472, 1660, 472, 1660, 496, 1636, 472, 1656, 476, 1656, 472, 1664, 492, 1636, 472, 1660, 472, 1660, 472, 1660, 472, 1660, 468, 1664, 468, 1660, 472, 1660, 472, 1660, 496, 1632, 1588, 544, 1564, 568, 472, 1660, 468, 1664, 492, 1636, 472, 1660, 496, 1636, 472, 1660, 472, 1660, 468, 1664, 1564, 564, 1560, 572, 1564, 568, 1564, 564, 472, 1660, 472, 1660, 496, 1636, 1588, 544, 472, 1660, 496, 1636, 1564};
unsigned short heatCode[] = {7828, 4300, 1588, 540, 496, 1632, 496, 1628, 496, 1632, 496, 1636, 496, 1628, 496, 1636, 496, 1636, 1588, 544, 496, 1632, 524, 1620, 496, 1628, 496, 1636, 496, 1636, 1588, 536, 496, 1636, 496, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1628, 1592, 544, 496, 1636, 492, 1636, 496, 1632, 496, 1632, 496, 1632, 500, 1632, 496, 1632, 496, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1632, 496, 1632, 500, 1632, 496, 1632, 496, 1636, 492, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1636, 492, 1636, 1588, 540, 1592, 536, 496, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1636, 492, 1628, 496, 1636, 492, 1636, 1588, 540, 1588, 540, 1592, 540, 1588, 540, 1592, 540, 492, 1636, 1588, 540, 496, 1632, 496, 1636, 496, 1632, 1588, 30388, 7832, 4288, 1588, 540, 492, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1636, 492, 1636, 496, 1632, 1588, 544, 492, 1636, 496, 1632, 496, 1636, 492, 1636, 496, 1632, 1588, 544, 492, 1636, 496, 1632, 496, 1632, 496, 1636, 496, 1632, 496, 1632, 1588, 544, 496, 1632, 496, 1632, 500, 1632, 496, 1632, 496, 1636, 496, 1632, 496, 1632, 496, 1636, 496, 1632, 496, 1632, 496, 1632, 500, 1632, 496, 1632, 496, 1632, 496, 1632, 496, 1632, 496, 1636, 496, 1632, 496, 1636, 496, 1632, 496, 1632, 496, 1632, 1588, 540, 1588, 544, 496, 1632, 496, 1632, 496, 1636, 492, 1636, 496, 1632, 496, 1632, 500, 1632, 496, 1632, 1592, 540, 1588, 540, 1588, 540, 1588, 544, 1588, 540, 496, 1632, 1592, 540, 496, 1632, 496, 1632, 496, 1636, 1588};
// gpio
const uint16_t IR_LED = 3;  // ESP8266 GPIO3 (RX)
const int RECV_PIN = 2;
const int BUILT_IN_PIN = 1; //gpio1 is the esp8266-01 built in led

// ir
const unsigned char irSsendRetry = 5;
const unsigned short statusCheckCodeLength = sizeof(statusCheckCode) / sizeof(statusCheckCode[0]);
volatile unsigned long irBuffer[statusCheckCodeLength + 2]; //stores timings - volatile because changed by ISR
const unsigned short maxRecvLength = sizeof(irBuffer) / sizeof(irBuffer[0]);
volatile unsigned short x = 0; //Pointer thru irBuffer - volatile because changed by ISR

// html code
const char HTML_OFF[]="\"status\":\"OFF\"";
const char HTML_COLD[]="\"status\":\"COLD\"";
const char HTML_HOT[]="\"status\":\"HOT\"";
const char HTML_NONE[]="\"status\":\"NONE\"";

IRsend irsend(IR_LED);
String idToken = "";


typedef enum
{
  LAir_NONE,
  LAir_OFF,
  LAir_COLD,
  LAir_HOT
}AIR_STATUS;

// -----------------------------------------------------------------
void rxIR_Interrupt_Handler() {
  if(x < maxRecvLength)
    irBuffer[x++] = micros(); //just continually record the time-stamp of signal transitions
}


bool readAirConditionStatus()
{
  bool isIrMatchFound = true;

  if(statusCheckCodeLength != x)
    isIrMatchFound = false;
    
  for(int i = 1; i < statusCheckCodeLength; i++){
    int sample = irBuffer[i] - irBuffer[i - 1];
    if(abs(sample - statusCheckCode[i]) > 50){
      isIrMatchFound = false;
      break;
    } 
  }

  x=0;
  return isIrMatchFound;
}

bool isAirConditionOff()
{
  attachInterrupt(digitalPinToInterrupt(RECV_PIN), rxIR_Interrupt_Handler, CHANGE);//re-enable ISR for receiving IR signal
  irsend.sendRaw(statusCheckCode, sizeof(statusCheckCode)/sizeof(statusCheckCode[0]), 38);
  detachInterrupt(digitalPinToInterrupt(RECV_PIN));

  return readAirConditionStatus();
}


AIR_STATUS checkDataBase()
{
  AIR_STATUS acStatus = LAir_NONE;
  String readString = getValueFromDb(internalDataBaseName, idToken);

  //parse the response
  if(readString.indexOf(HTML_OFF) >=0)
    acStatus = LAir_OFF;
  else if(readString.indexOf(HTML_COLD) >=0)
    acStatus = LAir_COLD;
  else if(readString.indexOf(HTML_HOT) >=0)
    acStatus = LAir_HOT;

  if(LAir_OFF == acStatus || LAir_COLD == acStatus || LAir_HOT == acStatus)
      httpsUpdateDataBase(internalDataBaseName, HTML_NONE, idToken);
  return acStatus;
}

static inline void sendSingleIrCode(unsigned short *code, unsigned int len)
{
  for(int i = 0; i < irSsendRetry; i++)
    irsend.sendRaw(code, len, 38);
}

void sendIrCode(AIR_STATUS stat)
{
  switch(stat){
    case LAir_OFF:
      httpsUpdateDataBase(displayDataBaseName, HTML_OFF, idToken);
      Serial.println("sending off code");
      sendSingleIrCode(offCode, sizeof(offCode)/sizeof(offCode[0]));
      break;
    case LAir_COLD:
      httpsUpdateDataBase(displayDataBaseName, HTML_COLD, idToken);
      Serial.println("sending cold code");
      sendSingleIrCode(coldCode, sizeof(coldCode)/sizeof(coldCode[0]));
      break;
    case LAir_HOT:
      httpsUpdateDataBase(displayDataBaseName, HTML_HOT, idToken);
      Serial.println("sending hot code");
      sendSingleIrCode(heatCode, sizeof(heatCode)/sizeof(heatCode[0]));
      break;

    case LAir_NONE:
    default:
      return; //return without blinking the led
  }
  delay(50); // delay between ir sends
}


void setup(void)
{  
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY); //rx pin is used for the ir transmit
  setupWifi();
  
  pinMode(RECV_PIN, INPUT);
  
  delay(1000);
  connectToWifi();
  Serial.println("air remote started");
}// setup


void loop() {

  if(WiFi.status() != WL_CONNECTED)
    ESP.restart();

  const AIR_STATUS LAir_status = checkDataBase();
  sendIrCode(LAir_status);

  delay(MILLISEC_TO_SEC * 2);
}// loop

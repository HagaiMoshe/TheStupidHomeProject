/* A simple control for turning the TV on and for preventing it from sleeping
 */

#include <IRsend.h>

#define DEBUG_PRINT
// arduino ide does not support relative path.
#include "C:\Users\MyPC\Desktop\stuff\code\theStupidHomeProject\common.h"

//ir codes
unsigned short volumeUpCode[] = {9020, 4456, 592, 540, 588, 540, 592, 540, 588, 540, 592, 544, 584, 540, 592, 1644, 588, 540, 592, 1644, 588, 1644, 592, 1644, 588, 1644, 592, 1644, 588, 1644, 588, 544, 588, 1644, 592, 540, 588, 1644, 592, 540, 588, 1644, 592, 1644, 588, 540, 592, 540, 588, 540, 592, 1644, 588, 540, 592, 1644, 588, 540, 592, 540, 588, 1644, 540, 1696, 588, 1644, 588, 39776, 9004, 2220, 592, 95820, 9020, 2224, 588};
unsigned short volumeDownCode[] = {9020, 4456, 592, 540, 588, 540, 592, 540, 588, 540, 592, 544, 584, 540, 592, 1644, 588, 540, 592, 1644, 588, 1644, 592, 1644, 588, 1644, 592, 1644, 588, 1644, 588, 544, 588, 1644, 592, 540, 588, 1644, 592, 540, 588, 1644, 592, 1644, 588, 540, 592, 540, 588, 540, 592, 1644, 588, 540, 592, 1644, 588, 540, 592, 540, 588, 1644, 540, 590, 588, 1644, 588, 39776, 9004, 2220, 592, 95820, 9020, 2224, 588};
unsigned short powerOnCode[] = {9024, 4456, 592, 536, 592, 540, 592, 536, 592, 540, 592, 536, 592, 540, 592, 1640, 592, 540, 592, 1640, 592, 1644, 592, 1640, 592, 1644, 592, 1640, 592, 1644, 592, 536, 592, 1644, 592, 536, 592, 1644, 588, 540, 592, 540, 592, 1640, 592, 540, 592, 536, 592, 540, 588, 1644, 592, 540, 592, 1640, 592, 1644, 592, 536, 592, 1644, 592, 1640, 592, 1644, 592, 39752, 9024, 2220, 592};
// gpio
const uint16_t IR_LED = 3;  // ESP8266 GPIO3 (RX)
const int RECV_PIN = 2;
const int BUILT_IN_PIN = 1; //gpio1 is the esp8266-01 built in led

// configuration
const unsigned char irSsendRetry = 3;
const unsigned int powerOnPushTime = 2000; //milliseconds
const unsigned int timeBetweenPushes = 500; //milliseconds

IRsend irsend(IR_LED);


// -----------------------------------------------------------------
static inline void sendSingleIrCode(unsigned short *code, unsigned int len)
{
  for(int i = 0; i < irSsendRetry; i++)
    irsend.sendRaw(code, len, 38);
  delay(50);// delay between ir sends
}


void setup(void)
{  
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY); //rx pin is used for the ir transmit

  pinMode(RECV_PIN, INPUT_PULLUP);
  
  delay(1000);
  debugPrint("tv remote started");
}// setup


void loop() {
  static unsigned int curruentMillis = 0;

  if(digitalRead(RECV_PIN) == LOW){
    curruentMillis = millis();
    
    debugPrint("toggeling the volume");
    sendSingleIrCode(volumeUpCode, sizeof(volumeUpCode)/sizeof(volumeUpCode[0]));
	delay(10);
    sendSingleIrCode(volumeDownCode, sizeof(volumeDownCode)/sizeof(volumeDownCode[0]));

    while(digitalRead(RECV_PIN) == LOW);
    if(millis() - curruentMillis > powerOnPushTime){
      debugPrint("sending powerOnCode");
      sendSingleIrCode(powerOnCode, sizeof(powerOnCode)/sizeof(powerOnCode[0]));
    }
      
    delay(timeBetweenPushes);
  } //if digitalRead
}// loop

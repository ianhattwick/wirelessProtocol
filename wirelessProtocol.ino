#include <EEPROM.h>
#include <ADXL345.h>
/*
Updated Rib firmware for use with wirelessProtocol idmil spec.
November 4 2015


Wireless protocol:
Code for formatting serial data for wireless transmission and for receiving and parsing wireless data. 
All data is sent through the built-in UART port, pins 0+1 on an Arduino or Atmega 328, etc. 

data arrays are:
serialOutputBuffer[64]
serialInputBuffer[127]

both can be resized according to your needs.

*/
/*Serial Protocol variables
 *
 *
 *
 *
 */

/*ID number for this device - saved in EEPROM typically, or can be hard-coded. 
A function for changing and saving the device ID is included here, as is a function to set the device ID on setup.
*/


#include <Wire.h>  
#include <SPI.h>
#define XBEE_SLEEP_PIN 2
#define accel_ID 0
#define SPI_ID 1
#define SPI_SS 10

ADXL345 adxl345;
int led = 5;
int led2 = 6;
int main_batt=2;
int led_batt=1;

/****/

byte device_type;
byte device_led;

/*****/

/***polling****/

byte polled = 0;
long polled_millis=0;
int poll_enable=0;
int poll_interval=20; //time between polls
uint32_t pollTimer=0;
uint16_t pollInterval=4000;

byte counter = 0;
int accelData[3] = {
  0, 0, 0};
  int prev_accelData[3] = {
  0, 0, 0};


byte CAPdata = 0;
byte prevCAPdata = 0;
  byte numCaps=0;
  byte thisCap=0;
  byte tempCap = 0;
  byte padOrder[]={0,1,7,6,2,3,5,4};
long ID_timer=0;
 
/***********LED variables***/
int sleep_level = 50;
int wake_level = 255;
int touch_level = 255;
long led_millis= 0; 
int led_interval = 25;
int led_threshold;
int led_offset=8;
int led_fade = 50;

int ledEvent_flag=0;
int ledEvent_value=0;
long ledEvent_millis=0;


/***********Low-battery variables***/

byte MC_led_status = 0;
byte LED_led_status = 0;
int batt_threshold1 = 512;
int batt_threshold2 = 512;
byte batt_LED = 8;
int batt_level[] = {0,0};
int led1_out, led2_out;

/***********initialization variables***/
int current_accel=0;
int input_val; //placeholder

/************oscillator variables***********************/
int led_brightness = 0;
int max_accel=0;
int accelFilter = 0;

int bdata = 0;
int accel_axes[] = {5,6,7};


int i;
int test=0;


int ledcurve[]= {0, 0, 0, 0, 0, 2, 5, 6, 7, 11, 15, 23, 26, 32, 35, 37,
                 42, 47, 50, 51, 56, 57, 59, 59, 62, 63, 64, 65, 65, 66, 68, 69,
                 70, 71, 72, 74, 74, 76, 76, 78, 79, 79, 80, 80, 81, 82, 83, 83,
                 83, 83, 84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 85, 86, 87, 87,
                 87, 88, 88, 88, 88, 88, 88, 89, 89, 90, 90, 90, 91, 91, 92, 92,
                 92, 93, 93, 94, 94, 95, 95, 95, 96, 97, 98, 98, 98, 99, 99, 99,
                 100, 100, 101, 101, 101, 101, 101, 101, 102, 102, 102, 102, 103, 103, 103, 104,
                 104, 104, 104, 104, 105, 105, 105, 106, 106, 106, 106, 106, 106, 106, 107, 108,
                 108, 109, 110, 110, 111, 111, 111, 112, 113, 114, 114, 114, 114, 115, 115, 116,
                 116, 117, 118, 119, 119, 120, 121, 122, 123, 124, 125, 125, 125, 125, 126, 126,
                 126, 126, 127, 127, 127, 127, 127, 127, 128, 129, 130, 131, 132, 133, 133, 134,
                 135, 136, 138, 138, 139, 139, 140, 142, 142, 143, 144, 144, 145, 146, 147, 149,
                 151, 152, 153, 154, 155, 156, 157, 157, 158, 164, 166, 169, 170, 171, 172, 176,
                 177, 178, 180, 183, 184, 187, 188, 189, 191, 191, 195, 197, 200, 201, 203, 204,
                 205, 207, 209, 211, 213, 214, 214, 216, 216, 217, 219, 221, 226, 226, 229, 233,
                 235, 236, 239, 242, 242, 244, 245, 245, 246, 248, 249, 250, 254, 254, 254, 254};
                 
/*
WIRELESS PROTOCOL VARIABLES
*/
byte deviceID;

byte inPktSize=0; //number of data bytes in incoming packet
uint16_t pktTime=0; //timestamp - unimplemented
byte inPktTime=0; //unimplemented
byte msgType=0; //message type  

/*
packet type indicates whether 
*/
byte ackFlag=0; //0:no acknowledgement needed, 1:ask for acknowledgement
byte pktNumber=0; //0-127 sequence number of packet

byte serialOutputBuffer[64]; //buffer for serial messages
byte bufferIndex=0;
uint32_t streamTimer=0;
uint16_t streamInterval=10;

//constants for SLIP encoding 
const byte escByte = 219; //1101 1011
const byte beginMsg = 164; //1010 0100
const byte endMsg = 192; // 1100 0000

//message types
const byte sensors=11;
const byte errorMsg = 12;
const byte acknowledgement=13; 
const byte testMessage=254;

//variables for acknowledging packet receipt
byte recPktNum=0;
byte packetReceiveFlag=0;

//variables for polling enable
byte pollEnable=0;
uint32_t pollEnableTimer=0; //check to see if to continue polling
uint16_t pollEnableInterval = 5000; //rate of poll timeout

byte specialChar[] = {
  escByte, //escape character
  beginMsg, //begin message packet
  endMsg, //end character
};



/******************************
SEUP
******************************/
void setup(){
  Serial.begin(57600);
  readDeviceID();
}

/******************************
LOOP
******************************/




void loop(){
  checkSerial();

}


/******************************
EEPROM functions
******************************/

void readDeviceID()
{
  deviceID = EEPROM.read(0);
}

void setDeviceID(byte val)
{
  EEPROM.write(0,val); //write serial data to device ID
  deviceID=val;
}

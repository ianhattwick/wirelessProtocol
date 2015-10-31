#include <EEPROM.h>

/*
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

long pollTimer=0;
int pollInterval=25; //rate of data sent
byte sampleStream=0;

void loop(){
  checkSerial();
  
  if((millis()>pollTimer+pollInterval)&&(pollEnable==1)){ //pollEnable must be set. . .
    pollTimer=millis();
    sampleStream++;
    prepStreamingData();
  }
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

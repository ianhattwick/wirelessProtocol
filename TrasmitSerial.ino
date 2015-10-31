/*
This code contains all of the serial sending functions
the most important are:

prepStreamingData() - prepares data for streaming.
outputSerialData() - actually sends the data. 

data sent format:
begin msg / device ID / packet number / message type + data packets / packet size /   packet timer  / end msg
*/

void prepStreamingData()
{
    //write functions here to buffer data
  bufferSensors(); //example
  ackFlag=0; //no acknowledgement needed
   outputSerialData(); //this function trasmits buffered data to wireless device after all data has been buffered
  /*
  Polling means the device will send data continuously at the <pollInterval> ms rate. 
  If a message from the host device is not received after a delay of <pollEnableInterval> ms then polling is disabled.
  This protects the host device from a flood of serial data when the host is not ready to receive it.
  Sometimes flooding the host device will cause your PC to freeze. . . 
  */
    if(millis()>(pollEnableTimer+pollEnableInterval)){
      //bufferSensors();
      pollEnable=0;
    }
}

/*
An example of buffering data
*/
void bufferSensors()
{
  bufferByte(sensors); //buffer message type first
  //buffer bytes
  for (int i=0;i<2;i++) {  
   bufferByte(sampleStream);
  }
  
  //buffer an int
  //for(int i=3;i<10;i++) bufferInt(outIMU[i]); 
}


void sendErrorMessage(byte error){
  //unimplemented
  ackFlag=0;
  bufferByte(errorMsg);
  bufferByte(error);
  outputSerialData();
}

void sendAckMessage(byte packetNumber){
  bufferByte(acknowledgement);
  bufferByte(packetNumber);
  ackFlag=0;
  outputSerialData();
}



/*
functions for writing data to the serial output buffer <serialOutputBuffer[]>
*/
void bufferByte(byte input)
{
  serialOutputBuffer[bufferIndex]=input;
  bufferIndex+=1;
}

void bufferInt(int output)
{
  serialOutputBuffer[bufferIndex]=((byte)(output >> 8));
  bufferIndex=bufferIndex+1;
  serialOutputBuffer[bufferIndex]=((byte)(output & 0xFF));
  bufferIndex=bufferIndex+1;
}



/***************************************************************************
outputSerialData is the function that actually transmits the serial data.

#TODO device ID, msgType and pktNumber are not SLIP encoded
****************************************************************************/

void outputSerialData()
{
  Serial.write(beginMsg); //SLIP start message
  
  slipOut(deviceID); //device ID
  byte byte2 = (ackFlag<<7) | pktNumber; //2nd byte - acknowledge flag and number
  pktNumber=(pktNumber+1)%128;
  slipOut(byte2);

  for(int i=0;i<bufferIndex;i++) {      //message type + data. . . 
    slipOut(serialOutputBuffer[i]);
  }

  slipOut(bufferIndex);
  bufferIndex=0; //reset buffer to have 0 elements

  //set time elapsed since last packet was sent. Seems redundant here. . . .
  pktTime=  (millis()-streamTimer);
  slipOut(pktTime);
  streamTimer=millis();

  Serial.write(endMsg); //SLIP end message
}

/*
escapes special characters
*/
void slipOut(byte input){
    //check to see if input is a special character
    for(int i=0;i<(sizeof(specialChar));i++) {
      if(input==specialChar[i]) { //if it is, escape it
        Serial.write(escByte);
      }
    }
   Serial.write(input); //write data byte
}


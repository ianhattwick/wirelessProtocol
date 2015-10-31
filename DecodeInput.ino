/*
This code does the following:
1) Removes SLIP encoding from incoming serial stream
2) Places de-encoded serial data in a buffer
3) Processes data according to the data protocol, does rudimentary error checking, sends acknowledgement and error messages
4) Processes the incoming messages

Serial input data format:
 device ID / packet number / #bytes / data packets
 */
 
// <static> variables declared outside a function are limited in scope to this file
static byte serialInputBuffer[128]; //

void checkSerial()
{
  //static variables declared in a function are function-scope, but persistent. 
  static int inputIndex=0;
  static int esc_flag=0;

  while (Serial.available()>0) {
    byte rawInput=Serial.read();

    if (esc_flag == 1) {                 // if prev byte was esc automatically write current byte to buffer
      serialInputBuffer[inputIndex] = rawInput;
      esc_flag = 0;
      inputIndex++;
    }  
    else if (rawInput == beginMsg) { // get ready for new input data
      inputIndex=0;
    } 
    else if (rawInput == escByte) { // next byte is good data
      esc_flag = 1;
    } 
    
    else if (rawInput == endMsg) {//end byte
      if((serialInputBuffer[0]==deviceID)||(serialInputBuffer[0]==255)) { //if incoming message is addressed to device or global ID 255
         byte receiveError=checkPktForErrors(inputIndex);
         if(receiveError==0) processSerialInput(inputIndex);
         inputIndex=0;
        }
      }

    else {
      serialInputBuffer[inputIndex]=rawInput;
      inputIndex++;
    }
    if(inputIndex>63)inputIndex=0;
  }
}

/*
processSerialInput
Processes incoming serial packets according to their message type
always make sure to increment i for each data byte belonging to a message
*/
void processSerialInput(byte msgLength)
{
  for (int i=2;i<msgLength-2;i++) {
    switch (serialInputBuffer[i]) { //serialInputBuffer[i] is the message type
    
      case 1:
      //an example case statement for messageType 1.
      /*
      byte testVal=serialInputBuffer[i+1]; //i+1 is the first data byte after the message type
      byte testVal2=serialInputBuffer[i+2];
      byte testVal=3serialInputBuffer[i+3];
      myTestFunction(testVal,testVal2,testVal3);
      i+=3; //always increment i for every data byte read.
      */
      break;
      
      case 11:
      //a received data stream and enable sending streaming data
      pollEnable=1;
      pollEnableTimer=millis();
      break;
      
      case 12:
      //an error message saying that the last package received by the host was corrupted
      break;
      
      case 13:
      //an acknowledgement message with the packet number of the message asking for acknowledgement
      break;
    
      //other functions here
    
      case 121: //ascii 'y', set deviceID remotely from host computer
        if (serialInputBuffer[i+1]==97) {
          if (serialInputBuffer[i+2]==105) {
            if (serialInputBuffer[i+3]==111) {
              if (serialInputBuffer[i+4]==98) setDeviceID(deviceID=serialInputBuffer[i+5]);
            }
          }
        }
        i+=5; //we increment by five because there are five data bytes
        break;
    
      case 122: //ascii 'z', turn power off from host computer
        if (serialInputBuffer[i+1]==97) {
          if (serialInputBuffer[i+2]==105) {
            if (serialInputBuffer[i+3]==111) {
              if (serialInputBuffer[i+4]==97) {
                //write function to turn power off here
              }
            }
          }
        }
        i+=4;
        break;
        
        case 253:
        for(int j=0;j<10;j++) bufferByte(j);
        outputSerialData();
        break;
        
        case 254: //test case, mirrors input back to sender
          bufferByte(254);
          while(i<msgLength) 
          {
            i+=1;
            bufferByte(serialInputBuffer[i]);
          }
          outputSerialData();
        break;
        }
    }
}

/*
checkPktForErrors()
Checks to see if the package is either not the expected size or is a duplicate. 
If either case is true the received data is not processed.
This function needs some work . . .
*/
byte checkPktForErrors(byte msgLength)
{
  byte incomingAckFlag=serialInputBuffer[1]>>7; //MSB is ackFlag
  byte recPktNum=serialInputBuffer[1] & 127; //packet number filters out MSB
  byte tempSize=serialInputBuffer[msgLength-2]; //packet size
  static byte prevPktNum=127;
  packetReceiveFlag=1;

  if((msgLength) != tempSize) { //packet not expected size
    sendErrorMessage(0);
    return(1);
  } else if (prevPktNum==recPktNum) { //packet is a duplicate
    //ignore packet
    sendErrorMessage(1);
    return(1);
  } else { //packet is expected size and not a duplicate
    prevPktNum=recPktNum;
    if(incomingAckFlag==1) sendAckMessage(recPktNum); //send acknowledgement of packet number, if required
    return(0);
  }
}





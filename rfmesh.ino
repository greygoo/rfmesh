// Define max size per sent packet
#define DEBUG true
#define RH_MESH_MAX_MESSAGE_LEN 50

#include <RHMesh.h>
#include <RH_RF22.h>
#include <SPI.h>
#include "rfmesh.h"

// set default address to 0;
int ID = 0;

// Array to hold messages,
uint8_t inBytes[RH_RF22_MAX_MESSAGE_LEN];
int i = 0;
uint8_t dest;

// Instance of radio driver
RH_RF22 driver;
// Class to manage messages
RHMesh manager(driver, ID);

void setup() {
  Serial.begin(9600);
  if ( !manager.init() ) {
    Serial.println("RH22 init failed");
  }
}

// Buffer for received messages
uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];

void loop() {
  // check for data on serial port
  if ( Serial.available() > 0 ) {
    inBytes[i] = Serial.read();
    if ( DEBUG ) Serial.println((char*)inBytes);

    if ( inBytes[i] == 10 || i == RH_RF22_MAX_MESSAGE_LEN ) {
      // Copy serial buffer array to output as casting it to char breaks the sendtoWait call
      inBytes[i+1] = '\0';
      uint8_t output[RH_RF22_MAX_MESSAGE_LEN];
      memcpy(output, inBytes, i+1);
      output[i+1] = '\0';

      // Enter command mode if leading character in serial buffer array is "/" 
      if ( inBytes[0] == 47 ) {
        command(inBytes[1]);
      }
      else {
        // send mode
        Serial.print(ID);
        Serial.print("->");
        Serial.print(dest);
        Serial.print(": ");
        Serial.print((char*)output);

        rfsend();
      }
      i = 0;
    }
    else {
      i++; 
    }

  }
  rfreceive();
}


void rfsend() {
  if ( manager.sendtoWait(inBytes, sizeof(inBytes), dest) && DEBUG ) {
    Serial.println("DEBUG: Message sent successfully");
  }
  else {
    Serial.println("SendtoWait failed");
  }
}


void rfreceive() {
  if ( manager.available() ) {
    if ( DEBUG ) Serial.println("DEBUG: Got something in the rf buffer");
    
    uint8_t len = sizeof(buf);
    uint8_t from;

    if ( manager.recvfromAck(buf, &len, &from) ) {
      Serial.print(from);
      Serial.print(" ");
      Serial.println((char*)buf);
    } 
    else {
      Serial.println("fetching from RF device buffer failed");
    }  
  }
}


void setdestination(){
  uint16_t tmp = retrieveAddress(inBytes);
  if ( DEBUG ) Serial.println(tmp);
  if ( tmp >= 0 && tmp <= 255 ) {
    dest = tmp;
  }
  if ( DEBUG ) {
    Serial.print("Setting destination address to: ");
    Serial.println(dest);
  }
}


void setID() {
 uint8_t tmp = retrieveAddress(inBytes);
 if ( DEBUG ) Serial.println(tmp);
 if ( tmp >= 0 && tmp <= 255 ) {
   ID = tmp;
 } 
 if ( DEBUG ) {
    Serial.print("Setting ID to: ");
    Serial.println(ID);
 }
}


uint16_t retrieveAddress(uint8_t asciiArray[]) {
  uint16_t address;

  if ( isInteger( asciiArray[2]) && isInteger(asciiArray[3]) && isInteger(asciiArray[4]) ) {
    address = (( asciiArray[2] - 48 ) * 100) + (( asciiArray[3] - 48 ) * 10) + asciiArray[4] - 48;
    if ( DEBUG ) Serial.println(address);
    return address;
  } 
  if ( isInteger( asciiArray[2]) && isInteger(asciiArray[3]) && !(isInteger(asciiArray[4])) ) {
    address = (( asciiArray[2] - 48 ) * 10) + asciiArray[3] - 48;
    if ( DEBUG ) Serial.println(address);
    return address;
  }
  if ( isInteger( asciiArray[2]) && !(isInteger(asciiArray[3])) && !(isInteger(asciiArray[4])) ) {
    address = asciiArray[2] - 48;
    if ( DEBUG ) Serial.println(address);
    return address;
  }
  Serial.println("Not a valid address");
  return 256;
}


bool isInteger(uint8_t var) {
  if ( DEBUG ) Serial.println(var);

  if ( var >= 48 && var < 58 ) {
    return true;
  }
  return false;
}


void command(uint8_t command) {
  // command mode
  if ( DEBUG ) Serial.println("entering command mode");

  switch ( command ) {
  case 48:
    setdestination();
    break;
  case 49:
    setID();
    break;
  default:
    Serial.println("Not a valid command");
    break;
  }
}


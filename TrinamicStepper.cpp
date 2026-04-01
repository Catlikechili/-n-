// TrinamicStepper.cpp
#include "TrinamicStepper.h"

  // Constructor
TrinamicStepper::TrinamicStepper(HardwareSerial &serial, long baudRate): serialPort(serial), tmclStatus(0), tmclValue(0), tmclByteIndex(0)  {
  // Initialize control pins
  pinMode(DE_PIN, OUTPUT);
  pinMode(RE_PIN, OUTPUT);
  // Start in receive mode
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
  serialPort.begin(BAUD_RATE, SERIAL_8N1, RO_PIN, DI_PIN);    //Hardware Serial of ESP32
  //serialPort.begin(baudRate);
}

// Function to calculate checksum for TMCL commands
uint8_t TrinamicStepper::calculateChecksum(uint8_t *data, uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; i++) {
    checksum += data[i];
  }
  return checksum;
}

// Function to send a TMCL command
bool TrinamicStepper::sendCommand(uint8_t moduleAddress, uint8_t command, uint8_t type, uint8_t motor, int32_t value) {
  uint8_t packet[9];
  packet[0] = moduleAddress;  // Module address for multi-motor control
  packet[1] = command;
  packet[2] = type; // Type: not used for ROR
  packet[3] = motor; // Motor ID
  packet[4] = (value >> 24) & 0xFF; // Value high byte (velocity)
  packet[5] = (value >> 16) & 0xFF;
  packet[6] = (value >> 8) & 0xFF;
  packet[7] = value & 0xFF; // Value low byte (velocity = 500)
  packet[8] = calculateChecksum(packet, 8); // Checksum (will be calculated)
  tmclByteIndex = 0;// start receive a new message packet
  while (serialPort.available() > 0) serialPort.read(); // delete all the incoming buffer
  this->tmclStatus = 0;
  this->tmclValue = 0;
  // Enable transmit mode
  digitalWrite(DE_PIN, HIGH);
  digitalWrite(RE_PIN, HIGH);
  for (uint8_t i = 0; i < TMCL_PACKET_LENGTH; i++)  serialPort.write(packet[i]);
  serialPort.flush();  // Wait until transmission is complete 
  //delayMicroseconds(10);  // Ensure full transmission before switching
  //Switch back to receive mode
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
  return true;
}

// Function to read response from TMCL module
bool TrinamicStepper::readResponse(void) {
 unsigned long currentMillis, startMillis = millis();
  do {
        while (serialPort.available() && (tmclByteIndex < TMCL_PACKET_LENGTH)) {
            tmclPacket[tmclByteIndex] = serialPort.read();
            tmclByteIndex ++;
        }
        currentMillis = millis();
  } while ((tmclByteIndex < TMCL_PACKET_LENGTH) && ((currentMillis - startMillis) <  50));
  
  if ((tmclByteIndex == TMCL_PACKET_LENGTH) && (calculateChecksum(tmclPacket, 8) == tmclPacket[8]) ) {
        this->tmclStatus = tmclPacket[2];
        this->tmclValue = (tmclPacket[4] << 24) | (tmclPacket[5] << 16) | (tmclPacket[6] << 8) | tmclPacket[7];
        tmclByteIndex = 0;// start a new message packet
        return (this->tmclStatus == TMCL_REPLY_ACK);
  } 
  return false;
}

// Function to get Status from TMCL feedback
uint8_t TrinamicStepper::getStatus(void) {
        return tmclStatus;
}

// Function to get Value from TMCL feedback
int32_t TrinamicStepper::getValue(void) {
        return tmclValue;
}

// Function to rotate motor right at given velocity
bool TrinamicStepper::rotateRight(uint8_t moduleAddress, int velocity, bool readBack) {
  if(velocity >= 0)
    sendCommand(moduleAddress, TMCL_CMD_ROR, 0, 0, velocity);
  else 
    sendCommand(moduleAddress, TMCL_CMD_ROL, 0, 0, - velocity);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to rotate motor left at given velocity
bool TrinamicStepper::rotateLeft(uint8_t moduleAddress, int velocity, bool readBack) {
   if(velocity >= 0)
    sendCommand(moduleAddress, TMCL_CMD_ROL, 0, 0, velocity);
   else 
    sendCommand(moduleAddress, TMCL_CMD_ROR, 0, 0, - velocity);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to stop motor
bool TrinamicStepper::stopMotor(uint8_t moduleAddress, bool readBack) {
  sendCommand(moduleAddress, TMCL_CMD_MST, 0, 0, 0);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to set zero position
bool TrinamicStepper::setZeroPosition(uint8_t moduleAddress, bool readBack) {
	sendCommand(moduleAddress, TMCL_CMD_SAP, 0, 0, 0);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to set encoder zero position
bool TrinamicStepper::setEncoderZeroPosition(uint8_t moduleAddress, bool readBack) {
	sendCommand(moduleAddress, TMCL_CMD_SAP, 1, 0, 0);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to set maximum position speed
bool TrinamicStepper::setMaxPositionSpeed(uint8_t moduleAddress, int32_t speed, bool readBack) {
	sendCommand(moduleAddress, TMCL_CMD_SAP, 4, 0, speed);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to set maximum acceleration
bool TrinamicStepper::setMaxAcceleration(uint8_t moduleAddress, int32_t acceleration, bool readBack) {
	sendCommand(moduleAddress, TMCL_CMD_SAP, 5, 0, acceleration);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to move to an absolute position
bool TrinamicStepper::moveToPosition(uint8_t moduleAddress, uint8_t moveMode, int32_t position, bool readBack) {
	sendCommand(moduleAddress, TMCL_CMD_MVP, moveMode, 0, position);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to move to an absolute position
bool TrinamicStepper::moveAbsolute(uint8_t moduleAddress, int32_t position, bool readBack) {
	moveToPosition(moduleAddress, 0, position);
  if(readBack) return this -> readResponse();
  return true;
}

// Function to move relative to the current position
bool TrinamicStepper::moveRelative(uint8_t moduleAddress, int32_t distance, bool readBack) {
	moveToPosition(moduleAddress, 1, distance);
  if(readBack) return this -> readResponse();
  return true;
}

// Function implementations in TrinamicStepper.cpp
bool TrinamicStepper::getMaxPositionSpeed(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 4, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::getMaxAcceleration(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 5, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::getActualPosition(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 1, 0, 0);
  return this -> readResponse();
}
bool TrinamicStepper::getActualSpeed(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 3, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::getEncoderPosition(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 209, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::getActualCurrent(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 6, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::getActualLoad(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_GAP, 31, 0, 0);
  return this -> readResponse();
}

bool TrinamicStepper::DisableRightSwitchStop(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_SAP, 12, 0, 1);
  return  this -> readResponse();
}

bool TrinamicStepper::DisableLeftSwitchStop(uint8_t moduleAddress) {
  sendCommand(moduleAddress, TMCL_CMD_SAP, 13, 0, 1);
  return this -> readResponse();
}


// Destructor
TrinamicStepper::~TrinamicStepper() {
	serialPort.end();
}

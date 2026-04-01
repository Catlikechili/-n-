// TrinamicStepper.h

#ifndef TRINAMICSTEPPER_H
#define TRINAMICSTEPPER_H

#include <Arduino.h>

// TMCL Command Definitions
#define TMCL_CMD_ROR  1
#define TMCL_CMD_ROL  2
#define TMCL_CMD_MST  3
#define TMCL_CMD_MVP  4
#define TMCL_CMD_SAP  5 //set axis parameter
#define TMCL_CMD_GAP  6 //get axis parameter
#define TMCL_CMD_STAP 7
#define TMCL_CMD_RSAP 8
#define TMCL_CMD_RFS  13
#define TMCL_CMD_SIO  14
#define TMCL_CMD_GIO  15
#define TMCL_REPLY_ACK 100
#define TMCL_PACKET_LENGTH 9 // TMCL Packet Length
#define BAUD_RATE 38400      // Communication baud rate

//Hardware Serial 2 pins
//#define RXD2 16
//#define TXD2 17
// Define RS485 pins
#define DI_PIN 17     // DI pin (TX) for RS485
#define RO_PIN 16     // RO pin (RX) for RS485
#define DE_PIN 18     // Driver Enable pin
#define RE_PIN 19     // Receiver Enable pin

class TrinamicStepper {
	public:
	// Constructor
	TrinamicStepper(HardwareSerial &serial, long baudRate = 38400);
	// Destructor
	~TrinamicStepper();
	private:
	HardwareSerial &serialPort;
  uint8_t tmclPacket[TMCL_PACKET_LENGTH];
  uint8_t tmclByteIndex;
  uint8_t tmclStatus;
  int32_t tmclValue;
  bool sendCommand(uint8_t moduleAddress, uint8_t command, uint8_t type, uint8_t motor, int32_t value);// Send TMCL command
  bool readResponse(void);// Read response from TMCL module
  uint8_t calculateChecksum(uint8_t *data, uint8_t length);// Calculate checksum
  public:
  // Read feedback functions
  uint8_t getStatus(void);// Read Status response from TMCL module
  int32_t getValue(void);// Read Value response from TMCL module
  // Control functions
  bool rotateRight(uint8_t moduleAddress, int velocity, bool readBack = false);// Rotate motor right
  bool rotateLeft(uint8_t moduleAddress, int velocity, bool readBack = false);// Rotate motor left
  bool stopMotor(uint8_t moduleAddress, bool readBack = false);// Stop motor
  bool setZeroPosition(uint8_t moduleAddress, bool readBack = false);// Set zero position
  bool setEncoderZeroPosition(uint8_t moduleAddress, bool readBack = false);// Set encoder zero position
  bool setMaxPositionSpeed(uint8_t moduleAddress, int32_t speed, bool readBack = false);// Set maximum position speed
  bool setMaxAcceleration(uint8_t moduleAddress, int32_t acceleration, bool readBack = false);// Set maximum acceleration
  bool moveToPosition(uint8_t moduleAddress, uint8_t moveMode, int32_t position, bool readBack = false);// Move to absolute position
  bool moveAbsolute(uint8_t moduleAddress, int32_t position, bool readBack = false);// Move absolute
  bool moveRelative(uint8_t moduleAddress, int32_t distance, bool readBack = false);// Move relative
  // Get functions 
  bool getMaxPositionSpeed(uint8_t moduleAddress);// Get maximum position speed
  bool getMaxAcceleration(uint8_t moduleAddress);// Get maximum acceleration
  bool getActualPosition(uint8_t moduleAddress); 
  bool getActualSpeed(uint8_t moduleAddress);
  bool getEncoderPosition(uint8_t moduleAddress);
  bool getActualCurrent(uint8_t moduleAddress);
  bool getActualLoad(uint8_t moduleAddress);
  bool DisableRightSwitchStop(uint8_t moduleAddress);
  bool DisableLeftSwitchStop(uint8_t moduleAddress);
};

#endif // TRINAMICSTEPPER_H

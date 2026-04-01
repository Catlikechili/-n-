/* 
* DataPacket.h
*
* Created: 1/27/2025 5:20:51 PM
* Author: duong
*/
#include <stdlib.h>

#ifndef __DATAPACKET_H__
#define __DATAPACKET_H__

#define NUM_DIGITS  10 // Number of Digits Length
#define ARRAY_ELEMENT 4 // Number Packet Length

class DataPacket
{
//variables
private:
	int32_t dataValue[ARRAY_ELEMENT];
	char buffer[NUM_DIGITS];
	int  cnt;
	int  num;  
	void clear(void);
//functions
public:
	DataPacket();
	~DataPacket();
	bool Push(char c);
	int GetValue(int num);


}; //DataPacket

#endif //__DATAPACKET_H__


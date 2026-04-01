/* 
* DataPacket.cpp
*
* Created: 1/27/2025 5:20:50 PM
* Author: duong
*/


#include "DataPacket.h"

// default constructor
DataPacket::DataPacket()
{
	cnt = 0;
	num = 0;
} //DataPacket

// default destructor
DataPacket::~DataPacket()
{
} //~DataPacket

bool DataPacket::Push(char c)
{
	switch(c)
	{
		case '>':        // new message
			cnt = 0;
			num = 0;
      clear();
			break;
		case ',':       // next data
			dataValue[num] =  atoi(buffer);
			cnt = 0;
			num ++;
			clear(); 
			break;
		case '\n':       // finish
			dataValue[num] =  atoi(buffer);
			cnt = 0;
			num = 0;
			clear();
			return true;
			break;
		default:
    if((c <='9' && c >= '0') || ( ((c == '-') || ( c == '+')) && (cnt == 0) ))
		buffer[cnt++] = c;
	}
	return false;
}

int DataPacket::GetValue(int num)
{
	if(num > ARRAY_ELEMENT) return 0;
	return dataValue[num];
}

void DataPacket::clear(void)
{
  for(int i = 0;i < NUM_DIGITS;i++) 
  buffer[i] ='\0'; 
}





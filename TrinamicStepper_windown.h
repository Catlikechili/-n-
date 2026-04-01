#pragma once
#include <windows.h>
#include <iostream>
#include <cstdint>

class TrinamicStepper {
private:
    HANDLE hSerial = INVALID_HANDLE_VALUE;

    unsigned char calcChecksum(unsigned char *buf) {
        unsigned int sum = 0;
        for (int i = 0; i < 8; i++)
            sum += buf[i];
        return sum & 0xFF;
    }

    bool sendTMCL(unsigned char addr, unsigned char cmd, unsigned char type,
                  unsigned char motor, int32_t value) {
        unsigned char frame[9];

        frame[0] = addr;
        frame[1] = cmd;
        frame[2] = type;
        frame[3] = motor;

        frame[4] = (value >> 24) & 0xFF;
        frame[5] = (value >> 16) & 0xFF;
        frame[6] = (value >> 8) & 0xFF;
        frame[7] = value & 0xFF;

        frame[8] = calcChecksum(frame);

        DWORD bytesWritten;
        if (!WriteFile(hSerial, frame, 9, &bytesWritten, NULL))
            return false;

        return (bytesWritten == 9);
    }

    bool readTMCL(int32_t &valueOut, uint8_t &statusOut) {
        unsigned char resp[9];
        DWORD bytesRead;

        if (!ReadFile(hSerial, resp, 9, &bytesRead, NULL))
            return false;

        if (bytesRead != 9)
            return false;

        valueOut = (resp[4] << 24) |
                   (resp[5] << 16) |
                   (resp[6] << 8)  |
                    resp[7];

        statusOut = resp[2];
        return true;
    }

public:
    // ================= INIT =================
    bool init(const char* port, int baudrate) {
        hSerial = CreateFile(port,
            GENERIC_READ | GENERIC_WRITE,
            0, 0, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, 0);

        if (hSerial == INVALID_HANDLE_VALUE)
            return false;

        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

        DCB dcb = {0};
        dcb.DCBlength = sizeof(dcb);

        if (!GetCommState(hSerial, &dcb))
            return false;

        dcb.BaudRate = baudrate;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity   = NOPARITY;

        if (!SetCommState(hSerial, &dcb))
            return false;

        COMMTIMEOUTS t = {0};
        t.ReadIntervalTimeout = 50;
        t.ReadTotalTimeoutConstant = 100;
        t.ReadTotalTimeoutMultiplier = 10;
        SetCommTimeouts(hSerial, &t);

        return true;
    }

    void close() {
        if (hSerial != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(hSerial);
            PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
        }
    }

    // ================= COMMANDS =================
    bool rotateRight(uint8_t addr, int speed, bool readBack = true) {
        sendTMCL(addr, (speed >= 0) ? 1 : 2, 0, 0, abs(speed));
        if (!readBack) return true;

        int32_t val; uint8_t st;
        return readTMCL(val, st);
    }

    bool rotateLeft(uint8_t addr, int speed, bool readBack = true) {
        sendTMCL(addr, (speed >= 0) ? 2 : 1, 0, 0, abs(speed));
        if (!readBack) return true;

        int32_t val; uint8_t st;
        return readTMCL(val, st);
    }

    bool stop(uint8_t addr, bool readBack = true) {
        sendTMCL(addr, 3, 0, 0, 0);
        if (!readBack) return true;

        int32_t val; uint8_t st;
        return readTMCL(val, st);
    }
    bool getCurrentPosition(uint8_t addr, int32_t &posOut) {
    // GAP type 1 = Actual Position
    sendTMCL(addr, 6, 1, 0, 0);

    uint8_t status;
    if (!readTMCL(posOut, status))
        return false;

    return true;
}
    bool moveTo(uint8_t addr, uint8_t mode, int32_t pos, bool readBack = true) {
        sendTMCL(addr, 4, mode, 0, pos);
        if (!readBack) return true;

        int32_t val; uint8_t st;
        return readTMCL(val, st);
    }
    
};
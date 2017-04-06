/*
  Microchip_EEPROM.h - Library for flashing Microchip 11AA EEPROM code.
  Created by Pavlo Ponomarov, April 3, 2017.
  Released into the public domain.
*/
#ifndef Microchip_EEPROM_h
#define Microchip_EEPROM_h

#include "Arduino.h"
#include "TimerOne.h"
#include <String.h>

#define TSTBY 605 // StandBy-Pulse 600µs + 5µs offset
#define TSS 15 // StartHeader setup time 10µs + 5µs offset
#define THDR 10 // StartHeader low pulse time 5µs + 5µs offset
#define TE 100 // BitPeriod time 10µs + 5µs offset
#define TE_half TE/2 // BitPeriod time 10µs + 5µs offset
#define TE_quarter TE/4 // BitPeriod time 10µs + 5µs offset
#define THDL TE*10 // BitPeriod time 10µs + 5µs offset
#define TWC 5005 // BitPeriod time 10000µs + 5µs offset

#define UNIO_STARTHEADER 0x55
#define UNIO_READ        0x03
#define UNIO_CRRD        0x06
#define UNIO_WRITE       0x6c
#define UNIO_WREN        0x96
#define UNIO_WRDI        0x91
#define UNIO_RDSR        0x05
#define UNIO_WRSR        0x6e
#define UNIO_ERAL        0x6d
#define UNIO_SETAL       0x67

#define SAK 1
#define noSAK 0

#define UNIO_device_address       0xA0
//#define UNIO_device_address       0xA1

#define CMD_START_HEADER 0x00
#define CMD_MAK 0x10
#define CMD_ISSAK 0x20
#define CMD_ADDRESS 0x30
#define CMD_READ 0x40
#define CMD_WRITE 0x50
#define CMD_WRITE_EN 0x60
#define CMD_RDSR 0x70
#define CMD_ERAL 0x80
#define CMD_TRANSMIT 0x90

class Microchip_EEPROM
{
  public:
    Microchip_EEPROM(int pin);
    void pulseStandBy();
    void pulseTss();
	void pulseTWC();
	void pulseStartHeader();
	void pulseAddress();
	bool MAK();
	bool noMAK();
	bool isSAK();
	void transmitByte(char value);
	bool connect();
	bool nextCmd();
	int readAddress(char addr, char *array);
	int writeAddress(char addr, char *array);
	int isWriteEnabled();
	byte readByte();
	//char *getLog();
	bool enableWrite();
	bool disableWrite();
	bool isReadyToWrite();
  private:
    int _pin;
	int fstEdgeSample;
	int sndEdgeSample;
	int sndEdgeSample_last;
	//void log(int command, int value);
	void bitOne();
	void bitZero();
	void pulseTHDR();
	char *getCommand(int command, int value);
	String parse32byte(char *bytes1, char *bytes2);
	String parse16byte(char *bytes);
	int transmitAddress(char addr);
	int getBit(char value, int bit);
};

#endif

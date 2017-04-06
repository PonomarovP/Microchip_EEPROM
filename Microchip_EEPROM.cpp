/*
 Microchip_EEPROM.h - Library for flashing Microchip 11AA EEPROM code.
 Created by Pavlo Ponomarov, April 3, 2017.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Microchip_EEPROM.h"

Microchip_EEPROM::Microchip_EEPROM(int pin) {
	_pin = pin;
	fstEdgeSample = 0;
	sndEdgeSample = 0;
	sndEdgeSample_last = 0;
	//_log_cnt = 0;
}

/*For half period set signal LOW and for other half HIGH*/
void Microchip_EEPROM::bitOne() {
	digitalWrite(_pin, LOW);
	delayMicroseconds(TE_half); // Trimmed-Bit-Period
	digitalWrite(_pin, HIGH);
	delayMicroseconds(TE_half); // Trimmed-Bit-Period
}

/*For half period set signal HIGH and for other half LOW*/
void Microchip_EEPROM::bitZero() {
	digitalWrite(_pin, HIGH);
	delayMicroseconds(TE_half); // Trimmed-Bit-Period
	digitalWrite(_pin, LOW);
	delayMicroseconds(TE_half); // Trimmed-Bit-Period
}

void Microchip_EEPROM::pulseTHDR() {
	digitalWrite(_pin, LOW); // High to Low transition for THDR
	pinMode(_pin, OUTPUT);
	delayMicroseconds(THDR); // THDR Time
}

void Microchip_EEPROM::pulseStandBy() {
	digitalWrite(_pin, HIGH); // Write high digital signal
	delayMicroseconds(TSTBY); // let standby-pulse time pass
}

void Microchip_EEPROM::pulseTss() {
	delayMicroseconds(TSS); // TSS Time
}

void Microchip_EEPROM::pulseTWC() {
	digitalWrite(_pin, HIGH); // High to Low transition for THDR
	pinMode(_pin, OUTPUT);
	delayMicroseconds(TWC); // TWC Time
}

void Microchip_EEPROM::pulseStartHeader() {
	pulseTHDR();
	transmitByte(UNIO_STARTHEADER);
}

void Microchip_EEPROM::pulseAddress() {
	transmitByte(UNIO_device_address);
}

bool Microchip_EEPROM::MAK() {
	bitOne();
	return true;
}

bool Microchip_EEPROM::noMAK() {
	bitZero();
	return false;
}

bool Microchip_EEPROM::isSAK() {
	pinMode(_pin, INPUT);
	delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
	fstEdgeSample = digitalRead(_pin);
	delayMicroseconds(TE_half); // Trimmed-Bit-Period
	sndEdgeSample = digitalRead(_pin);
	delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
	digitalWrite(_pin, sndEdgeSample); // Write output to actual Value before switching pinMode to OUTPUT
	pinMode(_pin, OUTPUT);

	if (fstEdgeSample == LOW && sndEdgeSample == HIGH) {
		return true;
	} else {
		return false;
	}
}

void Microchip_EEPROM::transmitByte(char value) {
	if (value & 0x80)
		bitOne();
	else
		bitZero();
	if (value & 0x40)
		bitOne();
	else
		bitZero();
	if (value & 0x20)
		bitOne();
	else
		bitZero();
	if (value & 0x10)
		bitOne();
	else
		bitZero();
	if (value & 0x08)
		bitOne();
	else
		bitZero();
	if (value & 0x04)
		bitOne();
	else
		bitZero();
	if (value & 0x02)
		bitOne();
	else
		bitZero();
	if (value & 0x01)
		bitOne();
	else
		bitZero();
}

byte Microchip_EEPROM::readByte() {
	byte fstEdgeSample_byte = 0b00000000;
	byte sndEdgeSample_byte = 0b00000000;
	byte output_read_byte = 0b00000000;

	pinMode(_pin, INPUT);
	for (int cnt_read = 8; cnt_read > 0; cnt_read--) {
		delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
		bitWrite(fstEdgeSample_byte, cnt_read - 1, digitalRead(_pin));
		delayMicroseconds(TE_half); // Trimmed-Bit-Period
		bitWrite(sndEdgeSample_byte, cnt_read - 1, digitalRead(_pin));
		if (cnt_read == 7) {
			sndEdgeSample_last = digitalRead(_pin);
		} else {
		}
		delayMicroseconds(TE_quarter); // Trimmed-Bit-Period
	}

	digitalWrite(_pin, sndEdgeSample_last); // Write output to actual Value before switching pinMode to OUTPUT
	pinMode(_pin, OUTPUT);

	return output_read_byte = sndEdgeSample_byte;
}

bool Microchip_EEPROM::connect() {
	digitalWrite(_pin, HIGH); //set OUTPUT to high before pinMode to start in Standby
	pinMode(_pin, OUTPUT);
	delay(1000);
	digitalWrite(_pin, LOW);
	delayMicroseconds(TE); // Trimmed-Bit-Period
	pulseStandBy();
	nextCmd();
}

bool Microchip_EEPROM::nextCmd() {
	pulseTss();
	pulseStartHeader();
	MAK();
	isSAK();
	pulseAddress();
	MAK();
	return isSAK();
}

int Microchip_EEPROM::transmitAddress(char addr) {
	transmitByte(addr >> 16);
	MAK();
	if (isSAK() == false) {
		return 0;
	}
	transmitByte(addr & 0xFF);
	MAK();
	if (isSAK() == false) {
		return 0;
	}
	return 1;
}

int Microchip_EEPROM::readAddress(char addr, char *array) {
	transmitByte(UNIO_READ);
	MAK();
	if (isSAK() == false) {
		return 0;
	}
	if (!transmitAddress(addr))
		return 0;
	int i = 0;
	for (i = 0; i < 16; i++) {
		array[i] = readByte();
		if (i == 15)
			noMAK();
		else
			MAK();
		if (isSAK() == false) {
			return 0;
		}
	}
	return 1;
}

int Microchip_EEPROM::isWriteEnabled() {
	transmitByte(UNIO_RDSR);
	MAK();
	if (isSAK() == false) {
		return -1;
	}
	char rdsr_byte = readByte();
	noMAK();
	if (isSAK() == false) {
		return -2;
	}
	if (getBit(rdsr_byte, 1) == 0) {
		return 0;
	}
	return 1;
}

bool Microchip_EEPROM::enableWrite() {
	transmitByte(UNIO_WREN);
	noMAK();
	if (isSAK() == false) {
		return false;
	}
	return true;
}

bool Microchip_EEPROM::disableWrite() {
	transmitByte(UNIO_WRDI);
	noMAK();
	if (isSAK() == false) {
		return false;
	}
	return true;
}

int Microchip_EEPROM::writeAddress(char addr, char *array) {
	transmitByte(UNIO_WRITE);
	MAK();
	if (isSAK() == false) {
		return 0;
	}
	if (!transmitAddress(addr))
		return 0;
	int i = 0;
	for (i = 0; i < 16; i++) {
		transmitByte(array[i]);
		if (i == 15)
			noMAK();
		else
			MAK();
		if (isSAK() == false) {
			return 0;
		}
	}
	pulseTWC();
	return 1;
}

bool Microchip_EEPROM::isReadyToWrite() {
	transmitByte(UNIO_RDSR);
		MAK();
		if (isSAK() == false) {
			return -1;
		}
		char rdsr_byte = readByte();
		noMAK();
		if (isSAK() == false) {
			return -2;
		}
		if (getBit(rdsr_byte, 0) == 1) {
			return 0;
		}
		return 1;
}

int Microchip_EEPROM::getBit(char a, int bit) {
	return ((a >> bit) & 0x01);
}

#include "Microchip_EEPROM.h"

Microchip_EEPROM c(6);
char arr[16];

void setup() {
  Serial.begin(115200);
  Serial.println("Usage:");
  Serial.println("h - to connect to Arduino");
  Serial.println("Hello - to test connection");
  Serial.println("write - to write to EEPROM at address 0x00");
  Serial.println("read - to read from EEPROM at address 0x00");
  Serial.println("Waiting for client..(h)");
  while (!GUIConnected()) { //Loop until GUI client is connected
    delay(100);
  }
  readEEPROM();
}

void loop() {
  if (Serial.available()) {
    String command;
    command = Serial.readStringUntil('\n');
    if (command.equals("h")) Serial.println("h");
    if (command.equals("Hello")) Serial.println("Hello yourself!");
    if (command.equals("read")) {
      readEEPROM();
    }
    if (command.equals("write")) {
      Serial.println("Awaiting input:");
      while (!Serial.available());
      String line = Serial.readStringUntil('\n');
      if (line.length() <= 16) {
        Serial.println("Got it");
        char buf[16];
        line.toCharArray(buf, 16);
        writeEEPROM(buf);
      } else
        Serial.println("Error: String is longer than 16 symbols");
    }
  }
}

void readEEPROM() {
  if (c.connect()) {
    int result = c.readAddress(0x00, arr);
    Serial.println("Connected");
    if (result) {
      printArray(arr);
    }
  } else {
    Serial.println("readEEPROM: Connection failed!");
  }
}

void writeEEPROM(char *arr) {
  if (c.connect()) {
    c.enableWrite();
  } else {
    Serial.println("writeEEPROM: Connection failed!");
  }
  if (c.connect()) {
    int result = c.writeAddress(0x00, arr);
    if (result) {
      Serial.println("Write succeded!");
    } else {
      Serial.println("Write failed!");
    }
  }else {
    Serial.println("writeEEPROM: Connection failed!");
  }
  if (c.connect()) {
    c.disableWrite();
  }else {
    Serial.println("writeEEPROM: Connection failed!");
  }
}

void writeEnabled() {
  if (c.connect()) {
    int result = c.isWriteEnabled();
    switch (result) {
      case 1: Serial.println("Write enabled"); break;
      case 0: Serial.println("Write disabled"); break;
      case -1: Serial.println("RDSR: transmit problem"); break;
      case -2: Serial.println("RDSR: read problem"); break;
    }
  } else {
    Serial.println("writeEnabled: Connection failed!");
  }
}

void printArray(char *arr) {
  for (int i = 0; i < 16; i++) {
    Serial.print(char(arr[i]));
  }
  Serial.println();
}

bool GUIConnected() {
  if (Serial.available()) {
    if (Serial.read() == 'h') {
      Serial.println("h");
      return true;
    }
  }
  return false;
}

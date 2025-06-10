// download library: https://github.com/smarmengol/Modbus-Master-Slave-for-Arduino/releases/tag/Release1

#include <ModbusRtu.h>

// Assign the Arduino pin connected to RE-DE on the RS-485 transceiver
#define TXEN 4

// Data array for Modbus network sharing (now 8 registers)
// We'll have register 0 increment by 1 each loop.
uint16_t au16data[3] = {0, 0, 0};

/**
 *  Modbus object declaration
 *  u8id       : node‐ID (0 = master, 1..247 = slave)
 *  port       : serial port
 *  u8txenpin  : 0 for RS-232/USB-FTDI, or any pin >1 for RS-485
 */
Modbus slave(1, Serial, TXEN); // This is slave @1 on RS-485

void setup() {
  Serial.begin(9600);  // Baud rate 9600
  slave.start();
  au16data[0] = 1250; // mA
  au16data[1] = 1400;
  au16data[2] = 1600;
}

void loop() {
  // Poll the Modbus network, sharing all registers
  if (au16data[0] < 1800)
    au16data[0]++;
  else
    au16data[0] -= 200;

  slave.poll(au16data, 3);
  delay(500);
}
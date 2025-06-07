#include <ModbusRTUSlave.h>

#define SLAVE_ID 1
#define DE A4
#define RE A5

uint16_t holdingRegisters[3] = { 1, 2, 3 };

ModbusRTUSlave slave(Serial);

void setup() {
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  
  Serial.begin(9600);
  
  slave.begin(SLAVE_ID, 9600, SERIAL_8N1);
  slave.configureHoldingRegisters(holdingRegisters, 3);
}

void loop() {
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  
  if (slave.poll()) {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(2);
    delay(2);
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
  }

  Serial.print("Holding Registers: ");
  for (int i = 0; i < 3; i++) {
    Serial.print("R");
    Serial.print(i);
    Serial.print("=");
    Serial.print(holdingRegisters[i]);
    Serial.print("  ");
  }
  Serial.println();

  delay(1000);
}

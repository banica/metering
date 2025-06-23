#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusRTUSlave.h>

// Define the Modbus slave ID and the pin for RS485 direction control
#define SLAVE_ID 1
#define DE_RE 2

// Modbus slave object and register array
ModbusRTUSlave slave(Serial);
uint16_t registers[8];

// --- Peripherals and Global Variables ---

// Part 1 & 2: Coin slot servo
Servo servo_11;

// Part 3: Window servo
Servo servo_10;

// Part 4: Alarm function
void sound_alarm() // sound alarm function
{
  digitalWrite(13, HIGH);
  tone(3, 532);
  delay(125);
  delay(200);
  digitalWrite(13, LOW);
  noTone(3);
  delay(200);
}

// Part 5 & 6: Fan control
void control_fan(int speedPin, int speed)  // fan operation function
{
  if (speed <= 0)
  {
    analogWrite(speedPin, 0);
  }
  else if (speed > 255)
  {
    analogWrite(speedPin, 255);
  }
  else
  {
    analogWrite(speedPin, speed);
  }
}

// DHT Sensor
DHT dht12(12, 11);

// Part 7: Keypad/Door lock variables and functions
int red;
int green;
int timeCounter;
String password;
int door;
boolean doorAccess;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo_9;

void play_key_tone()
{
  tone(3, 349);
  delay(125);
  delay(100);
  noTone(3);
  delay(100);
}

void confirm_password()
{
  if (green == 0 && red != 0)
  {
    delay(100);
    red = digitalRead(5);
    if (green == 0 && red != 0)
    {
      if (!doorAccess)
      {
        if (password == "--")
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass");
          lcd.setCursor(10, 1);
          lcd.print("OK");
          servo_9.write(180);
          delay(0);
          door = 0;
          doorAccess = true;
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass:");
          lcd.setCursor(10, 1);
          lcd.print("Err");
          tone(3, 165);
          delay(125);
          delay(500);
          noTone(3);
          delay(200);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(10, 1);
          lcd.print("Again");
          lcd.setCursor(0, 1);
          lcd.print("Password:");
          play_key_tone();
        }
      }
      else // doorAccess == true
      {
        doorAccess = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smart Home");
        servo_9.write(110);
        delay(0);
      }
      password = "";
    }
  }
}

void setup()
{
  // --- Modbus Setup ---
  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);
  Serial.begin(9600);
  slave.begin(SLAVE_ID, 9600, SERIAL_8N1);
  slave.configureHoldingRegisters(registers, 8);

  // --- Initialize Pins & Peripherals ---
  // Part 1: Light & Motion
  pinMode(A1, INPUT);
  pinMode(A4, INPUT);
  pinMode(7, OUTPUT);

  // Part 2: Coin Slot
  pinMode(A0, INPUT);
  servo_11.attach(11);

  // Part 3: Rain Sensor & Window
  pinMode(A2, INPUT);
  servo_10.attach(10);

  // Part 4: Soil Moisture & Alarm
  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(A3, INPUT);

  // Part 5: Flame Sensor & Fan
  pinMode(8, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);

  // Part 6: Temp/Humidity
  dht12.begin();

  // Part 7: Door Lock System
  red = 1;
  green = 1;
  timeCounter = 0;
  password = "";
  door = 0;
  doorAccess = false;
  lcd.init();
  lcd.backlight();
  servo_9.attach(9);
  lcd.setCursor(0, 0);
  lcd.print("Smart Home");
  servo_9.write(110);
  pinMode(5, INPUT);
  pinMode(4, INPUT);
}

void loop()
{
  // Poll for Modbus requests
  digitalWrite(DE_RE, LOW);
  if (slave.poll()) {
    digitalWrite(DE_RE, HIGH);
    // A small delay might be needed for the transceiver to switch
    delay(10); 
    digitalWrite(DE_RE, LOW);
  }

  /* ---------- READ ALL SENSORS ---------- */
  // Note on Pin 2: This pin is used for DE/RE on the RS485 transceiver.
  // Reading from it as 'ambientSound' will likely not work as expected and may cause communication issues.
  // Consider moving the sound sensor to an unused pin.
  int   lightLevel      = analogRead(A1);
  int   ambientSound    = digitalRead(2); // POTENTIAL CONFLICT WITH DE_RE PIN
  int   coinSensor      = digitalRead(A0);
  int   rainfall        = analogRead(A2);
  int   soilMoisture    = analogRead(A3);
  int   flameDetected   = digitalRead(8);
  float temperatureC    = dht12.readTemperature();
  float humidityPct     = dht12.readHumidity();
  int   motionSensor    = analogRead(A4); // From original code, assigned to registers[1]

  /* ---------- UPDATE MODBUS REGISTERS ---------- */
  // This makes the sensor data available to the Modbus master.
  // Note: float values from DHT sensor are converted to integers here.
  registers[0] = lightLevel;
  registers[1] = motionSensor;
  registers[2] = coinSensor;
  registers[3] = rainfall;
  registers[4] = soilMoisture;
  registers[5] = flameDetected;
  registers[6] = temperatureC;
  registers[7] = humidityPct;
  
  /* ---------- PRINT TO SERIAL MONITOR ---------- */
  Serial.println(F("----- Sensor Readings -----"));
  Serial.print(F("Light level: "));     Serial.println(lightLevel);
  Serial.print(F("Ambient sound: "));   Serial.println(ambientSound);
  Serial.print(F("Coin sensor: "));     Serial.println(coinSensor);
  Serial.print(F("Rain level: "));      Serial.println(rainfall);
  Serial.print(F("Soil moisture: "));   Serial.println(soilMoisture);
  Serial.print(F("Flame detected: "));  Serial.println(flameDetected);
  Serial.print(F("Temperature (C): ")); Serial.println(temperatureC);
  Serial.print(F("Humidity (%): "));    Serial.println(humidityPct);
  Serial.println();

  // --- Main Program Logic (uses values from the 'registers' array) ---

  // Part 1: Light & Motion controlled LED
  if (registers[0] > 500)
  {
    if (registers[1] == 1)
    {
      digitalWrite(7, HIGH); // turn on red LED
      delay(6000);
      digitalWrite(7, LOW);  // turn off red LED
    }
  }
  else
  {
    digitalWrite(7, LOW);
  }

  // Part 2: Coin-operated servo
  if (registers[2] == 0) // coin inserted reads 0
  {
    servo_11.write(180);
  }
  else if (registers[2] == 1)
  {
    servo_11.write(90);
  }

  // Part 3: Rain-controlled window servo
  if (registers[3] > 100)
  {
    servo_10.write(90); // close window
  }
  else
  {
    servo_10.write(0);  // keep window open
  }

  // Part 4: Soil moisture alarm
  if (registers[4] < 50)
  {
    sound_alarm();
  }
  else
  {
    digitalWrite(13, LOW);
    noTone(3);
  }

  // Part 5: Flame-controlled fan
  if (registers[5] == 1) // if light sensor detects radiation
  {
    control_fan(6, 120); // fan on pin 6 at speed 120/255
  }
  else
  {
    control_fan(6, 0);
  }

  // Part 6: Temperature-controlled fan
  if (registers[6] > 30)
  {
    control_fan(6, 60);
  }
  else
  {
    control_fan(6, 0);
  }

  // Part 7: Keypad and door lock logic
  green = digitalRead(4);
  red = digitalRead(5);  // 1 if button not pressed, 0 if pressed
  if (green != 0 && red == 0)
  {
    delay(100);
    green = digitalRead(4);
    while (green != 0 && red == 0)
    {
      red = digitalRead(5);
      timeCounter++;
      delay(100);
    }
  }
  if (timeCounter > 1 && timeCounter < 5)
  {
    play_key_tone();
    password += ".";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
  }
  if (timeCounter > 5)
  {
    play_key_tone();
    password += "-";
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
  }
  confirm_password();
  timeCounter = 0;
}

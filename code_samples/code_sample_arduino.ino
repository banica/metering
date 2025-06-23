#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- SECTION 1: light & sound ----------
volatile int lightLevel;          // light sensor (A1)
volatile int ambientSound;        // sound sensor (D2)

// ---------- SECTION 2: coin acceptor ----------
int coinSensor;                   // coin sensor (A0)
Servo coinServo;                  // servo on pin 11

// ---------- SECTION 3: rain ----------
int rainfall;                     // rain sensor (A2)
Servo windowServo;                // servo on pin 10

// ---------- SECTION 4: soil moisture & alarm ----------
int soilMoisture;                 // soil moisture sensor (A3)
void soilAlarm()                  // soil moisture alarm
{
  digitalWrite(13, HIGH);
  tone(3, 532);
  delay(125);
  noTone(3);
  delay(200);
}

// ---------- SECTION 5: flame & fan ----------
int flameDetected;                // flame sensor (D8)
void fanControl(int speedPin, int speed)  // fan control helper
{
  if (speed <= 0) {
    analogWrite(speedPin, 0);
  } else if (speed > 255) {
    analogWrite(speedPin, 255);
  } else {
    analogWrite(speedPin, speed);
  }
}

// ---------- SECTION 6: DHT12 ----------
int temperatureC;
int humidityPct;
DHT dht12(12, 11);   // (dataPin, DHT11)

// ---------- SECTION 7: keypad & door ----------
int redBtn;
int greenBtn;
int timeCounter;
String password;
boolean doorAccess;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo doorServo;      // servo on pin 9

void playKeyTone()
{
  tone(3, 349);
  delay(125);
  noTone(3);
  delay(100);
}

void confirmPassword()
{
  if (greenBtn == 0 && redBtn != 0) {
    delay(100);
    redBtn = digitalRead(5);
    if (greenBtn == 0 && redBtn != 0) {
      if (!doorAccess) {
        if (password == "--") {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass OK");
          doorServo.write(180);  // open
          doorAccess = true;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass Err");
          tone(3, 165);
          delay(125);
          noTone(3);
          delay(200);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Again");
          playKeyTone();
        }
      } else {               // door currently open
        doorAccess = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smart Home");
        doorServo.write(90); // close
      }
      password = "";
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Smart Home system starting..."));

  // SECTION 1
  pinMode(A1, INPUT);    // light sensor
  pinMode(2, INPUT);     // sound sensor
  pinMode(7, OUTPUT);    // alert LED

  // SECTION 2
  pinMode(A0, INPUT);    // coin sensor
  coinServo.attach(11);
  coinServo.write(90);

  // SECTION 3
  pinMode(A2, INPUT);    // rain sensor
  windowServo.attach(10);
  windowServo.write(0);

  // SECTION 4
  pinMode(A3, INPUT);    // soil sensor
  pinMode(13, OUTPUT);   // alarm LED/buzzer
  pinMode(3, OUTPUT);    // buzzer tone pin

  // SECTION 5
  pinMode(8, INPUT);     // flame sensor
  pinMode(6, OUTPUT);    // fan PWM
  digitalWrite(6, LOW);

  // SECTION 6
  dht12.begin();

  // SECTION 7
  pinMode(5, INPUT);     // red button
  pinMode(4, INPUT);     // green button
  doorServo.attach(9);
  doorServo.write(90);   // closed
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home");
}

void loop()
{
  /* ---------- READ SENSORS ---------- */
  lightLevel    = analogRead(A1);
  ambientSound  = digitalRead(2);
  coinSensor    = digitalRead(A0);
  rainfall      = analogRead(A2);
  soilMoisture  = analogRead(A3);
  flameDetected = digitalRead(8);
  temperatureC  = dht12.readTemperature();
  humidityPct   = dht12.readHumidity();

  /* ---------- SERIAL MONITOR ---------- */
  Serial.println(F("----- Sensor Readings -----"));
  Serial.print(F("Light level: ")); Serial.println(lightLevel);
  Serial.print(F("Ambient sound: ")); Serial.println(ambientSound);
  Serial.print(F("Coin sensor: ")); Serial.println(coinSensor);
  Serial.print(F("Rain level: ")); Serial.println(rainfall);
  Serial.print(F("Soil moisture: ")); Serial.println(soilMoisture);
  Serial.print(F("Flame detected: ")); Serial.println(flameDetected);
  Serial.print(F("Temperature (C): ")); Serial.println(temperatureC);
  Serial.print(F("Humidity (%): ")); Serial.println(humidityPct);
  Serial.println();

  /* ---------- ACTUATION LOGIC ---------- */

  // SECTION 1: Light & sound to LED alert
  if (lightLevel > 500 && ambientSound == HIGH) {
    digitalWrite(7, HIGH);
    delay(6000);
    digitalWrite(7, LOW);
  } else {
    digitalWrite(7, LOW);
  }

  // SECTION 2: Coin acceptor
  if (coinSensor == LOW) {      // coin inserted
    coinServo.write(180);
  } else {
    coinServo.write(90);
  }

  // SECTION 3: Rain → window
  if (rainfall > 100) {
    windowServo.write(90);      // close window
  } else {
    windowServo.write(0);       // open window
  }

  // SECTION 4: Soil moisture → alarm
  if (soilMoisture < 50) {
    soilAlarm();
  } else {
    digitalWrite(13, LOW);
    noTone(3);
  }

  // SECTION 5: Flame → fan
  fanControl(6, flameDetected == HIGH ? 120 : 0);

  // SECTION 6: Temperature-based fan (optional)
  /* if (temperatureC > 30) {
       fanControl(6, 60);
     } else {
       fanControl(6, 0);
     } */

  // SECTION 7: Keypad / buttons
  greenBtn = digitalRead(4);
  redBtn   = digitalRead(5);
  if (greenBtn != 0 && redBtn == 0) {
    delay(100);
    greenBtn = digitalRead(4);
    while (greenBtn != 0 && redBtn == 0) {
      redBtn = digitalRead(5);
      timeCounter++;
      delay(100);
    }
  }

  if (timeCounter > 1 && timeCounter < 5) { // short press = dot
    playKeyTone();
    password += '.';
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
  } else if (timeCounter >= 5) { // long press = dash
    playKeyTone();
    password += '-';
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
  }

  confirmPassword();
  timeCounter = 0;

  delay(200);  // ease serial output readability
}

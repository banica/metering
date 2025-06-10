#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusRtu.h>
 
#define SLAVE_ID 1
#define DE_RE 2
Modbus slave;

uint16_t registers[8];
 
// input variables part 2
Servo servo_11;
 
// input variables part 3
Servo servo_10;
 
// input variables and functions part 4
void sound_alarm() // sound alarm function
{
  digitalWrite(13, HIGH);
  tone(3, 532);
  // delay(125);
  // delay(200);
  digitalWrite(13, LOW);
  noTone(3);
  // delay(200);
}
 
// input variables and functions part 5
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
 
// input variables part 6
DHT dht12(12, 11);
 
// input variables and functions part 7
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
  // delay(125);
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
          /*
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass");
          lcd.setCursor(10, 1);
          lcd.print("OK");
          servo_9.write(180);
          door = 0;
          */
          doorAccess = true;          
        }
        else
        {
          /*
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(0, 1);
          lcd.print("Pass:");
          lcd.setCursor(10, 1);
          lcd.print("Err");
          tone(3, 165);
          // delay(125);
          // delay(500);
          noTone(3);
          // delay(200);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Smart Home");
          lcd.setCursor(10, 1);
          lcd.print("Again");
          lcd.setCursor(0, 1);
          lcd.print("Password:");
          */
          play_key_tone();
        }
      }
      else // doorAccess == true
      {
        doorAccess = false;
        /*
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smart Home");
        */
        servo_9.write(110);
      }
      password = "";
    }
  }
}
 
void setup()
{
  pinMode(DE_RE, OUTPUT);
 
  Serial.begin(9600);
  slave = Modbus(SLAVE_ID, 0, DE_RE);
  slave.begin(9600);
 
  // initialize variables part 1  
  pinMode(A1, INPUT);
  pinMode(A4, INPUT);
  pinMode(7, OUTPUT);
 
  // initialize variables part 2
  pinMode(A0, INPUT);
  servo_11.attach(11);
 
  // initialize variables part 3
  pinMode(A2, INPUT);
  servo_10.attach(10);
 
  // initialize variables part 4
  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(A3, INPUT);
 
  // initialize variables part 5
  pinMode(8, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
 
  // initialize variables part 6
  dht12.begin();
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
 
  // initialize variables part 7
  pinMode(3, OUTPUT);
  red = 1;
  green = 1;
  timeCounter = 0;
  password = "";
  door = 0;
  doorAccess = false;
  /*
  lcd.init();
  lcd.backlight();
  servo_9.attach(9);
  lcd.setCursor(0, 0);
  lcd.print("Smart Home");
  servo_9.write(110);
  */
  pinMode(5, INPUT);
  pinMode(4, INPUT);
}
 
void loop()
{
  // main program part 1
  registers[0] = analogRead(A1);
  registers[1] = analogRead(A4);
  if (registers[0] > 500)
  {
    if (registers[1] == 1)
    {
      digitalWrite(7, HIGH); // turn on red LED
      digitalWrite(7, LOW);  // turn off red LED
    }
  }
  else
  {
    digitalWrite(7, LOW);
  }
 
  // main program part 2
  registers[2] = digitalRead(A0); // coin inserted reads 0
  if (registers[2] == 0)
  {
    servo_11.write(180);
  }
  else if (registers[2] == 1)
  {
    servo_11.write(90);
  }
 
  // main program part 3
  registers[3] = analogRead(A2);
  if (registers[3] > 100)
  {
    servo_10.write(90); // close window
  }
  else
  {
    servo_10.write(0);  // keep window open
  }
 
  // main program part 4
  registers[4] = analogRead(A3);
  if (registers[4] < 50)
  {
    sound_alarm();
  }
  else
  {
    digitalWrite(13, LOW);
    noTone(3);
  }
 
  // main program part 5
  registers[5] = digitalRead(8);
  if (registers[5] == 1) // if light sensor detects radiation
  {
    control_fan(6, 120); // fan on pin 6 at speed 120/255
  }
  else
  {
    control_fan(6, 0);
  }
 
  // main program part 6
  registers[6] = dht12.readTemperature(); // degrees Celsius
  registers[7] = dht12.readHumidity();    // percent
  if (registers[6] > 30)
  {
    control_fan(6, 60);
  }
  else
  {
    control_fan(6, 0);
  }
 
  // main program part 7
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
    /*
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
    */
  }
  if (timeCounter > 5)
  {
    play_key_tone();
    password += "-";
    /*
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smart Home");
    lcd.setCursor(0, 1);
    lcd.print("Password:");
    lcd.setCursor(10, 1);
    lcd.print(password);
    */
  }
  confirm_password();
  timeCounter = 0;

  slave.poll(registers, sizeof(registers) / sizeof(registers[0]));
  delay(500);
}

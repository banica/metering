#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusRTUSlave.h>
 
#define SLAVE_ID 1
#define DE_RE 2
 
ModbusRTUSlave slave(Serial);
uint16_t registers[8];
 
// variabile de intrare partea 2
Servo servo_11;
 
// variabile de intrare partea 3
Servo servo_10;
 
// variabile de intrare si functii partea 4
void alarma_sol() // functie alarma sunet
{
  digitalWrite(13,HIGH);
  tone(3, 532);
  delay(125);
  delay(200);
  digitalWrite(13,LOW);
  noTone(3);
  delay(200);
}
 
// variabile de intrare si functii partea 5
void ventilator (int speedpin, int speed)  //functie functionare ventilator
{
  if (speed <= 0)
  {
    analogWrite(speedpin, 0);
  }
  else if (speed > 255)
  {
    analogWrite(speedpin, 255);
  }
  else
  {
    analogWrite(speedpin, speed);
  }
}
 
// variabile de intrare partea 6
DHT dht12(12, 11);
 
// variabile de intrare si functii partea 7
int rosu;
int verde;
int timp;
String parola;
int usa;
boolean acces_usa;
LiquidCrystal_I2C mylcd(0x27,16,2);
Servo servo_9;
void cheie()
{
  tone(3,349);
  delay(125);
  delay(100);
  noTone(3);
  delay(100);
}
void confirmare_parola()
{
  if (0 == verde && 0 != rosu)
  {
    delay(100);
    rosu = digitalRead(5);
    if (0 == verde && 0 != rosu)
    {
      if (false == acces_usa)
      {
        if (parola == "--")
        {
          mylcd.clear();
          mylcd.setCursor(1-1, 1-1);
          mylcd.print("Casa Inteligenta");
          mylcd.setCursor(1-1, 2-1);
          mylcd.print("Pass");
          mylcd.setCursor(11-1, 2-1);
          mylcd.print("OK");
          servo_9.write(180);
          delay(0);
          usa = 0;
          acces_usa = true;          
        }
        else
        {
          mylcd.clear();
          mylcd.setCursor(1-1, 1-1);
          mylcd.print("Casa Inteligenta");
          mylcd.setCursor(1-1, 2-1);
          mylcd.print("Pass:");
          mylcd.setCursor(11-1, 2-1);
          mylcd.print("Err");
          tone(3, 165);
          delay(125);
          delay(500);
          noTone(3);
          delay(200);
          mylcd.clear();
          mylcd.setCursor(1-1, 1-1);
          mylcd.print("Casa Inteligenta");
          mylcd.setCursor(11-1, 2-1);
          mylcd.print("Iar");
          mylcd.setCursor(1-1, 2-1);
          mylcd.print("Parola:");
          cheie();
        }
      }
   
       else if (true == acces_usa)
       {
         acces_usa = false;
         mylcd.clear();
         mylcd.setCursor(1-1, 1-1);
         mylcd.print("Casa Inteligenta");
         servo_9.write(110);
         delay(0);
       }
    parola = "";
  }
}
}
 
void setup()
{
  pinMode(DE_RE, OUTPUT);
 
  Serial.begin(9600);
  slave.begin(SLAVE_ID, 9600, SERIAL_8N1);
  slave.configureHoldingRegisters(registers, 8);
 
// initializare variabile partea 1  
  pinMode(A1, INPUT);
  pinMode(A4, INPUT);
  pinMode(7, OUTPUT);
 
// initializare variabile partea 2
  pinMode (A0, INPUT);
  servo_11.attach(11);
 
// initializare variabile partea 3
  pinMode(A2, INPUT);
  servo_10.attach(10);
 
// initializare variabile partea 4
  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(A3, INPUT);
 
// initializare variabile partea 5
  pinMode(8, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
 
// initializare variabile partea 6
  dht12.begin();
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
 
// initializare variabile partea 7
  pinMode(3, OUTPUT);
  rosu = 1;
  verde = 1;
  timp = 0;
  parola = "";
  usa = 0;
  acces_usa = false;
  mylcd.init();
  mylcd.backlight();
  servo_9.attach(9);
  mylcd.setCursor(1-1, 1-1);
  mylcd.print("Casa Inteligenta");
  servo_9.write(110);
  pinMode(5, INPUT);
  pinMode(4, INPUT);
}
 
void loop()
{
  digitalWrite(DE_RE, LOW);  
 
  if (slave.poll()) {
digitalWrite(DE_RE, HIGH);
delay(10);
digitalWrite(DE_RE, LOW);
  }
 
// program principal partea 1
    registers[0] = analogRead(A1);
    registers[1] = analogRead(A4);
    if (registers[0] > 500)
    {
      if (registers[1] == 1)
      {
        digitalWrite(7,HIGH); // porneste LED-ul rosu
        delay(6000);
        digitalWrite(7,LOW); // taie tensiunea de alimentare a LED-ului
      }
    }
    else
    {
      digitalWrite(7,LOW);
    }
 
// program principal partea 2
    registers[2] = digitalRead(A0); // moneda introdusa afiseaza 0
    if (registers[2] == 0)
    {
      servo_11.write(180);
    }
    else if (registers[2] == 1)
    {
      servo_11.write(90);
    }
 
// program principal partea 3
    registers[3] = analogRead(A2);
    if (100 < registers[3])
    {
      servo_10.write(90); // inchide fereastra
    }
    else
    {
      servo_10.write(0); // pastreaza deschisa fereastra
    }
 
// program principal partea 4
    registers[4] = analogRead(A3);
    if (50 > registers[4])
    {
      alarma_sol();
    }
    else
    {
      digitalWrite(13,LOW);
      noTone(3);
    }
 
// program principal partea 5
    registers[5] = digitalRead(8);
    if (1 == registers[5]) // daca senzorul sesizeaza radiatie luminoasa
    {
      ventilator (6, 120); // ventilatorul functioneaza pe pinul digital 6 la viteza 120/255
    }
    else
    {
      ventilator (6, 0);
    }
 
// program principal partea 6
    registers[6] = dht12.readTemperature(); // grade Celsius
 
    registers[7] = dht12.readHumidity(); // in %
    if (30 < registers[6])
    {
      ventilator (6, 60);
    }
    else
    {
      ventilator (6, 0);
    }
 
// program principal partea 7
    verde = digitalRead(4);
    rosu = digitalRead(5);  //1 daca butonul nu e apasat, 0 daca este apasat
    if (0 != verde && 0 == rosu)
    {
      delay(100);
      verde = digitalRead(4);
      while (0 != verde && 0 == rosu)
      {
        rosu = digitalRead(5);
        timp = timp + 1;
        delay(100);
      }
    }
    if (1 < timp && 5 > timp)
    {
      cheie();
      parola = String(parola) + String(".");
      mylcd.clear();
      mylcd.setCursor(1-1, 1-1);
      mylcd.print("Casa Inteligenta");
      mylcd.setCursor(1-1, 2-1);
      mylcd.print("Parola:");
      mylcd.setCursor(11-1, 2-1);
      mylcd.print(parola);
    }
    if (5 < timp)
    {
      cheie();
      parola = String(parola) + String("-");
      mylcd.clear();
      mylcd.setCursor(1-1, 1-1);
      mylcd.print("Casa Inteligenta");
      mylcd.setCursor(1-1, 2-1);
      mylcd.print("Parola:");
      mylcd.setCursor(11-1, 2-1);
      mylcd.print(parola);
    }
  confirmare_parola();
  timp = 0;
}


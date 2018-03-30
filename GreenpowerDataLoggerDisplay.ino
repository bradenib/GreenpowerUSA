/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 7, 5, 4, 3, 6);  // the last pin had a default (sample program) value of 2, but had to be changed to 6 due to conflict with interrupt
const int chipSelect = 10;
unsigned long trigger_time=0;
unsigned long last_trigger_time=0;
unsigned long time_interval;
float speedo=0;
float voltage;
float current;
int rawCurrent=0;
int currentZero;
int triggerLED=9;
int writeLED=8;
int currentPIN=A2;
int voltagePIN=A1;
byte places1=1;
byte places0=0;

void setup() {
  //initialize LCD
  lcd.begin(16, 2);

  //welcome driver
  lcd.print("Welcome to car 123.");
  delay(1000);
  lcd.clear();
  
  //Zero the current sensor
  lcd.print("Zeroing the");
  lcd.setCursor(0,1);
  lcd.print("current sensor");
  delay(500);
  currentZero=analogRead(currentPIN);
  lcd.clear();
  delay(500);
  lcd.setCursor(15,1);
  lcd.blink();
  delay(2000);
  lcd.clear();
  
  //Initialize pin functions
  pinMode(writeLED, OUTPUT);
  pinMode(triggerLED, OUTPUT);

  //attach interrupt so that every time the magnet passes the sensor the SpeedCalc function will update the speed (speedo)
  attachInterrupt(0,SpeedCalc,FALLING);  //attach speed sensor to digital pin 2

  // see if the card is present and can be initialized, and communicate SD status to LCD
  if (!SD.begin(chipSelect)) {
    lcd.print("Card failed!");
  }
  else {
    lcd.print("Card good.");
  }
  delay(2000);
  lcd.clear();

  lcd.print("Ready to roll.");
  delay(1000);
  lcd.clear();


  
}

void loop() {
  //debugging LEDs for checking if SD is writting and if the magnetic sensor is triggering for speed
  digitalWrite(writeLED,LOW);
  digitalWrite(triggerLED,LOW);

  //refresh rate for the speed display
  delay(200); 

  //a timeout in case there hasn't been an interrupt (wheel turn) in a long time
  if (millis()-last_trigger_time>2000){  
    speedo=0;
  }

  //measuring analog sensors and converting
  rawCurrent=analogRead(currentPIN);
  current=(float(analogRead(currentPIN)-currentZero))/1023*5/0.0098;  //difference from zero converted to voltage, then current with 40mV per amp, according to manufacturer
  if (current<0.7){
    current=0.00;
  }
  voltage=float(analogRead(voltagePIN))/1023*5*6.1;  // conversion from 10 bit to measured voltage (0-5V), then up again due to voltage divider circuit

  //sending readings to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Spd  Pwr  Vlt");
  lcd.setCursor(0,1);
  lcd.print(speedo,places1);
  lcd.setCursor(5,1);
  lcd.print(current*voltage,places0);
  lcd.setCursor(10,1);
  lcd.print(voltage,places1);

  // make a string for assembling the data to log:
  String dataString = String(millis()/1000)+","+String(speedo)+","+String(voltage)+","+String(current);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    digitalWrite(writeLED,HIGH);
    lcd.setCursor(15,1);
    lcd.blink();
    dataFile.close();
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    lcd.noBlink();
  }
}

void SpeedCalc(){
  trigger_time=millis();
  digitalWrite(triggerLED,HIGH);
  time_interval=trigger_time-last_trigger_time;
  if (time_interval>100){  //time delay debounce (stops multiple triggers at once)
    speedo=1000.0*0.5/time_interval;
  }
  last_trigger_time=trigger_time;
}










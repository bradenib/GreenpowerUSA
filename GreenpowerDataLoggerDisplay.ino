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
int time_interval=3000;  //anything large, so that it starts with a speed of zero
float speedo=0;
float voltage;
int current;
int currentZero = 517;
int currentPIN=A2;
int voltagePIN=A1;
byte places1=1;
byte places0=0;
long int rev=0;
float charge=0.0;
float eff=0.0;
const int numReadings = 100;
long int readings[numReadings];
int readIndex=0;
long int total = 0;

void setup() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading]=0;
  }
  //initialize LCD
  lcd.begin(16, 2);
  //Zero the current sensor
  //currentZero=analogRead(currentPIN);
  
  //attach interrupt so that every time the magnet passes the sensor the time_period function will update the period of rotation (time_interval)
  attachInterrupt(0,time_period,FALLING);  //attach speed sensor to digital pin 2
  
  // see if the card is present and can be initialized, and communicate SD status to LCD
  if (!SD.begin(chipSelect)) {
    lcd.print("Card failed!");
  }
  delay(2000);
  lcd.clear();
}

void loop() {
  //refresh rate for the speed display
  delay(200); 

  //measuring speed from time interval and wheel circ.
  if (millis()-last_trigger_time>2000){  //set minimum speed to show zero
    speedo=0;
  } else{
    speedo=1000*1.57/time_interval;
  }

  //measuring and calculating voltage
  voltage=float(analogRead(voltagePIN))/1023*5*6.1;  // conversion from 10 bit to measured voltage (0-5V), then up again due to voltage divider circuit

  //sending readings to LCD
  lcd.clear();
  lcd.setCursor(0,0);
  if (millis()>1500000){    //timer for driver change notification
    lcd.print("Head to the pit!");
  }
  else{
    lcd.print("spd  eff  vlt");
  }
  lcd.setCursor(0,1);
  lcd.print(speedo,1);
  lcd.setCursor(5,1);
  lcd.print(eff,0);
  lcd.setCursor(10,1);
  lcd.print(voltage);

  

  // make a string for assembling the data to log:
  String dataString = String(millis())+","+String(rev)+","+String(time_interval)+","+String(speedo)+","+String(voltage)+","+String(current)+","+String(charge)+","+String(eff);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
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

void time_period(){
  trigger_time=millis();
  if (trigger_time-last_trigger_time>100){            //ignores short time intervals caused by noise triggering the interrupt
    time_interval=trigger_time-last_trigger_time;
    last_trigger_time=trigger_time;
    rev=rev+1;  //adding up the cumulative distance traveled in revolutions
    current=analogRead(currentPIN)-currentZero;  //difference from zero converted to voltage, then current with 40mV per amp, according to manufacturer
    if (current<2){
      current=0;
    }
    
    charge=charge+float((current*time_interval))/1000;
    total=total-readings[readIndex];
    readings[readIndex]=time_interval*current/100;
    total=total+readings[readIndex];
    readIndex=readIndex+1;
    if (readIndex>=numReadings){
      readIndex=0;
    }
    eff=float(total)/float(numReadings);
   
  }

}










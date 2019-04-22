/*
GreenPower Data Logger
Marc Braden, Cara Li, Jofred Gonzalez

Circuit available at https://github.com/bradenib/GreenpowerUSA/blob/master/GreenpowerDatalogger.pdf

The data logger collects data from a variety of sensors (magnetic switch to detect wheel rotation, battery voltage, current sensor in series with battery,
and temperature sensor on motor case).  Raw analog values are converted appropriately using calibrations.  Speed (and a measure of efficiency) are calculated
when a hardware interrupt from the magnetic switch triggers the excecution of the time_period() function, at which point the time interval between rotations
can be determined.  The rest of the measurements are made every time the loop cycles.  Selected data are displayed on an LCD and saved to a SD card.

Hardware
ACS758 current sensor, expected 50 amp bidirectional 40mV/amp, but based on limited calibration it looks like a 200 amp bidirectional, 10mV/amp (9.8mV/amp measured)

 */

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

//Setting up the LCD
LiquidCrystal lcd(8, 7, 5, 4, 3, 6);  // last pin had a default of 2, but had to be changed to 6 due to conflict with interrupt from magnetic switch (interrupts only available on 2 and 3)
byte places1=1;  //determines displayed precision on LCD
byte places0=0;

//Setting up the SD card
const int chipSelect = 10;  // pin 10 is used to control the SD card chip

//Initiallizing measurement and calculation variables and constants
unsigned long trigger_time=0;  //the internal clock time (in milliseconds) when the magnetic switch is triggered
unsigned long last_trigger_time=0;  //the last time the switch was triggered
int time_interval=3000;  //anything large, so that it starts with a speed of zero, updated with difference after first use
long int rev=0;  //counts revolutions of the wheel
float speedo=0; //the speed of the cart

int voltagePIN=A1;
float voltage;

int rawCurrent;
float current;
int currentZero; //the sensor has a zero offset error, so its value is measured when first turned on, and this value taken as zero
int currentPIN=A2;  //assignment of sensor pins
float charge=0.000001;  //charge (ideally in Coulombs transfered through the battery) = current*time.  Ideally, the battery can transfer the same total charge from use to use.

float temp;
int tempPIN=A0;

/*float eff=0;  //efficiency, described below
const int numReadings = 100;  //the size of the "moving window" used to smooth out the efficiency readings, which aren't very useful with an on/off power switch otherwise
long int readings[numReadings];  //an array that stores the selected "window" of measurements
int readIndex=0;  //a particular element of the "window" array
long int total = 0;  //the current sum of the elements in the "window", which when divided by numReadings gives the average
*/

//The runs once.  Some things need to be included in this section.
void setup() {
  /*for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading]=0;
  }
*/
  //Serial.begin(9600);
  
  //initialize LCD
  lcd.begin(16, 2);
  //Zero the current sensor
  currentZero=analogRead(currentPIN);
  
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

  //measuring and calculating temperature
  temp=0.113*analogRead(tempPIN)-34;

  lcd.setCursor(0,0);
  //sending readings to LCD
  lcd.clear();
  //if (millis()>1500000){    //timer for driver change notification
  //  lcd.print("Head to the pit!");
  //}
  //else{
  lcd.print("spd tp vlt   cur");
  //}
  lcd.setCursor(0,1);
  lcd.print(speedo,1);
  lcd.setCursor(4,1);
  lcd.print(temp,0);
  lcd.setCursor(7,1);
  lcd.print((voltage));
  lcd.setCursor(13,1);
  lcd.print(current, places1);

  

  // make a string for assembling the data to log:
  String dataString = String(millis())+","+String(rev)+","+String(time_interval)+","+String(speedo)+","+String(voltage)+","+String(current)+","+String(charge)+","+String(temp);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    lcd.setCursor(15,0);
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

    //time interval calculations
    time_interval=trigger_time-last_trigger_time;
    last_trigger_time=trigger_time;
    
    //counting revolutions
    rev=rev+1;  //adding up the cumulative distance traveled in revolutions
    
    //Current and charge use calculations
    rawCurrent=analogRead(currentPIN)-currentZero;  
    if (rawCurrent<2){  //avoiding errors in small currents due to signal drift
      current=0;
    }
    else{
      current=float(rawCurrent)/1024*5*1000/11.5;   // 1024 possible values from 10-bit ADC over a 5 V range, converted to mV for the 11.5 mV/A conversion from calibration
    }
    charge=charge+(current*time_interval)/1000/3600;  // 1000 milliseconds per second, 3600 seconds per hour

    //efficiency calculations
    /*total=total-readings[readIndex];
    readings[readIndex]=time_interval*current/100;
    total=total+readings[readIndex];
    readIndex=readIndex+1;
    if (readIndex>=numReadings){
      readIndex=0;
    }
    eff=float(total)/float(numReadings);
    */
   
  }

}










/*
Continuous writing of culumlative value to SD, but starts by looking in SD for where it left off
Fixed length records
One record per line
Resets the value if a button is pressed during start.

NOTE:  writing to the SD card is slow.  The convention is to open and close the file every loop.  Closing the file actually writes the data to the file, otherwise it is all held in a buffer, writing to the SD when it gets full.  "Flush" or close when done, or to make sure it makes it on the SD.
Things could be sped up by forcing it to write to SD less frequently, like every 10 loops, or every 10 s (time%10==0)?
The buffer is automatically written to the card every 512 bytes (taking 7-8 ms), but the function Flush or Close will properly edit the file to reflect the changes, otherwise normal methods would show that no new data was added.
Flushing takes 8-10 ms, same for close, which just makes it impossible to do further writes to the SD until opened.
This program gets 12-14 ms per loop.  It starts with a quick record 
EDITS
04/22/2019 - 
* Added robust (recovers well from resets ability to add multiple entries using formatted character arrays concatenated.  The main problem is that a reset event can mess up the counting.  The "size" will no longer be in increments of the bytesPerLine.  Be safe - seek (move the pointer) to the beginning of the last complete entry by subtracting anything beyond a complete entry.
* Added a button to delete the text file, mostly for troubleshooting.  Remove for final version?
04/25/2019
*creating functions to simplify the code
04/25/2019
*adding variables to log, instead of const char

 */

#include <SPI.h>
#include <SD.h>

File myFile;
float oldTime=0;  //The start time for the counter, assuming there is no old data
int rate=1000;  //delay time (millis) to control the measurement rate
int bytesPerMeasure=9;  //The number of characters (including negative and decimal) required to represent the largest values expected, and each measurement will have the same length
int bytesPerLine = (bytesPerMeasure)*3+2+2; //3 equal, constant width measurements, add 2 for the two commas, add 2 for the CR and LF
float measureA=12345.678;  //sample measurements
float measureB=257;
const String fileName="test.txt";
int resetPin=18;  //Using A4 as a digital pin.  Pins for buttons to press during startup to reset the time to zero or delete the existing file
int deletePin=19;  //Using A5 as a digital pin

void setup() {
  Serial.begin(9600);
  pinMode(deletePin,INPUT_PULLUP);  //take advantage of the internal pullup resistor to avoid having to use an external one.  This means the logic is reversed.  It will read low when the botton connecting to ground is pressed.
  pinMode(resetPin,INPUT_PULLUP);
  
  //Check for SD card
  if (!SD.begin(10)) {  
    Serial.println("SD initialization failed!");
    return;
  }
  
  //If the file exists but the file delete button is pressed, delete the file
  if (SD.exists(fileName) && !digitalRead(deletePin)){  //see note above for pinMode to explain the !
    SD.remove(fileName);    
  }

  // If no file exists, make a new one.  If it does, recover the first entry from the last line and set up the SD pointer to the correct position in the file to continue logging
  if (SD.exists(fileName) && digitalRead(resetPin)) { //If there's old data in an existing file and the reset pin is not being pressed (with internal pullup, it reads high when not grounded), otherwise "oldTime" =0
    oldTime=recoverLog(fileName,bytesPerLine,bytesPerMeasure);
  } else{
    myFile = SD.open(fileName, FILE_WRITE);
    Serial.println("time zeroed");  //time is zeroed since the old time is not recovered and oldTime was initiallized as zero, and when logging starts it is appended to the old file
  }
}

void loop() {
  // if the file opened okay, write to it:
  if (myFile) {
    appendToLog(myFile,bytesPerLine,bytesPerMeasure,oldTime,measureA,measureB);
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
  delay(rate);  //controls the rate of data collection/SD writing
}




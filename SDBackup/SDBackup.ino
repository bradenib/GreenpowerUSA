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
 */

#include <SPI.h>
#include <SD.h>

File myFile;
int bytesPerLine = 31;
float thisTime=0;
float oldTime=0;

const char counterChar[10]="      134";  //Need to make array sizes large enough to hold the end of array character (0?), otherwise the neighboring memory just bleeds in!
const char counter2Char[10]="      135";
int resetPin=9;
int deletePin=8;

void setup() {
  // Open serial communications and wait for port to open:
  
  Serial.begin(9600);
  pinMode(deletePin,INPUT_PULLUP);
  

  if (!SD.begin(10)) {
    Serial.println("SD initialization failed!");
    return;
  }
  // If no file exists, make a new one.  If it does, get the last value
  if (SD.exists("test.txt") && !digitalRead(deletePin)){
    SD.remove("test.txt");    
  }
  delay(100);
  if (SD.exists("test.txt") && !digitalRead(resetPin)) { //If there's old data in an existing file and the reset pin is not being pressed, otherwise "oldTime" =0
    myFile = SD.open("test.txt", FILE_WRITE);
    // read from last line - the data on the SD is one long line of data, with \n\r characters to represent new lines
    long int total=myFile.size();
    myFile.seek(((total-total%bytesPerLine)/bytesPerLine-1)*bytesPerLine);  //Moves the pointer to the beginning of the last line.  Modulus used to remove any incomplete lines.  size() returns the number of bytes.  Each character is a byte.
    Serial.println("File exists.  Getting last data on line "+String((total-total%bytesPerLine)/bytesPerLine));
    char lineToRead[10];   
    for (int i=0;i<9;i++){  //read the bytes from the beginningn of the last line until the known bytes in a line
      lineToRead[i]=myFile.read();   //read() worked byte by byte
    }
    lineToRead[9]="\0";  //Adding a null terminator
    myFile.seek(((total-total%bytesPerLine)/bytesPerLine-1)*bytesPerLine);  //Man, this took a while to figure out.  Send the pointer back to the correct position for later writing
    Serial.println(lineToRead);
    oldTime=atof(lineToRead);  //converts the character string to a floating point number
    Serial.println(oldTime,3); //a number after a comma allows you to specify how many places after the decimal are shown.  The default is 2.
    delay(500);
    // close the file:
  } else{
    myFile = SD.open("test.txt", FILE_WRITE);
    Serial.println("file created");
  }
}

void loop() {
  // if the file opened okay, write to it:
  if (myFile) {
    char lineToWrite[31];
    thisTime=millis()/1000.0+oldTime;  //converts from milli to seconds, adds on oldTime (recovered from SD) if there was an old time, otherwise oldTime = 0
    dtostrf(thisTime,9,3,lineToWrite);  //Double to formatted string, takes thisTime, puts it in character array "c" with 9 total characters, 3 after the decimal
    strcat(lineToWrite,",");
    strcat(lineToWrite,counterChar);
    strcat(lineToWrite,",");
    strcat(lineToWrite,counter2Char);
    myFile.println(lineToWrite);
    Serial.println(lineToWrite); 
    // close the file:
    myFile.flush(); 
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  delay(1000);  //controls the rate of data collection/SD writing
}



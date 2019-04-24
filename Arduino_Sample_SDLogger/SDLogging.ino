/*
 * Continuous writing of culumlative value to SD, but starts by looking in SD for where it left off
Fixed length records
One record per line
NOTE:  writing to the SD card is slow.  The convention is to open and close the file every loop.  Closing the file actually writes the data to the file, otherwise it is all held in a buffer, writing to the SD when it gets full.  "Flush" or close when done, or to make sure it makes it on the SD.
Things could be sped up by forcing it to write to SD less frequently, like every 10 loops, or every 10 s (time%10==0)?
The buffer is automatically written to the card every 512 bytes (taking 7-8 ms), but the function Flush or Close will properly edit the file to reflect the changes, otherwise normal methods would show that no new data was added.
Flushing takes 8-10 ms, same for close, which just makes it impossible to do further writes to the SD until opened.
This program gets 12-14 ms per loop.  It starts with a quick record 
 */




//Finds the last complete line in a fixed length (bytesPerLine) record, reads the first bytesToRecover from that line, sends the "pointer" to the end of the line to continue writing (overwrites a partial line)
float recoverLog(String fileName, int bytesPerLine, int bytesPerMeasure){
    myFile = SD.open(fileName, FILE_WRITE);
    // read from last line - the data on the SD is one long line of data, with \n\r characters to represent new lines
    long int total=myFile.size();
    myFile.seek(((total-total%bytesPerLine)/bytesPerLine-1)*bytesPerLine);  //Moves the pointer to the beginning of the last line.  Modulus used to remove any incomplete lines.  size() returns the number of bytes.  Each character is a byte.
    char valueToRead[(bytesPerMeasure+1)];  //add 1 to include the null ending - the n byte number requires an additional character at the end
    for (int i=0;i<(bytesPerMeasure);i++){  //read the bytes from the beginning of the last line until the known bytes in the first value (the one to be recovered)
      valueToRead[i]=myFile.read();   //read() works byte by byte, building up the character string
    }
    valueToRead[(bytesPerMeasure)]="\0";  //Adding a null terminator to the last position
    myFile.seek(((total-total%bytesPerLine)/bytesPerLine)*bytesPerLine);  //Man, this took a while to figure out.  Send the pointer back to the correct position for later writing (the end of the file)
    return atof(valueToRead);  //converts the character string to a floating point number and returns the value;
}

//Appends a line of characters to an open SD file.  The line is made up of 3 fixed length floats, the first of which is recovered from previous entries for continuous logging
void appendToLog(File myFile, int bytesPerLine,int bytesPerMeasure, float recoveredData,float x,float y){
    char lineToWrite[bytesPerLine];
    char buff[bytesPerMeasure+1]; //add a spot for a null terminator
    float newData=millis()/1000.0+recoveredData;  //converts from milli to seconds, adds on recoveredData (recovered from SD) if there was any, otherwise recoveredData must be sent zero
    dtostrf(newData,bytesPerMeasure,3,lineToWrite);  //Double to formatted string, takes newData, puts it in character array "lineToWrite" with bytesPerMeasure total characters, 3 after the decimal
    strcat(lineToWrite,",");  //concatenates lineToWrite and a comma
    dtostrf(x,bytesPerMeasure,3,buff);
    strcat(lineToWrite,buff);
    strcat(lineToWrite,",");
    dtostrf(y,bytesPerMeasure,3,buff);
    strcat(lineToWrite,buff);
    myFile.println(lineToWrite);
    Serial.println(lineToWrite); 
    // close the file:
    myFile.flush(); //writes the memory buffer to the SD card
}

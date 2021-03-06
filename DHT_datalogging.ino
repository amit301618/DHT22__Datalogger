//Libraries
#include <DHT.h>
#include <DHT_U.h>
#include "SD.h"
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  1000 // mills between entries. 
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht (DHTPIN, DHTTYPE) ;

//Variables declaration

float hum;
float temp;

RTC_DS1307 RTC; // define the Real Time Clock object
const int chipSelect = 10;// for the data logging shield, we use digital pin 10 for the SD cs line
File myfile;// the logging file

void setup()
{
  Serial.begin(9600);
  initSDcard();// initialize the SD card
  createFile();// create a new file
  initRTC(); // initialize the RTC
  dht.begin(); //initialize the DHT sensor
  myfile.println("millis,stamp,date,time,Humidity, Temperature");
  pinMode(2, INPUT);

}

void loop()
{
  DateTime now;
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));// delay for the amount of time we want between readings
  uint32_t m = millis(); // log milliseconds since starting
  myfile.print(m);           // milliseconds since start
  myfile.print(", ");
  now = RTC.now();// fetch the time
  // log time
  myfile.print(now.unixtime()); // seconds since 2000
  myfile.print(", ");
  myfile.print(now.year(), DEC);
  myfile.print("/");
  myfile.print(now.month(), DEC);
  myfile.print("/");
  myfile.print(now.day(), DEC);
  myfile.print(" ");
  myfile.print(", ");
  myfile.print(now.hour(), DEC);
  myfile.print(":");
  myfile.print(now.minute(), DEC);
  myfile.print(":");
  myfile.print(now.second(), DEC);

  //Read data and store it to variable
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  myfile.print(", ");
  myfile.print(hum);
  myfile.print(", ");
  myfile.println(temp);

  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  myfile.flush();

}

void initSDcard()
{
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void createFile()
{
  //file name must be in 8.3 format (name length at most 8 characters, follwed by a '.' and then a three character extension.
  char filename[] = "MLOG00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      myfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void initRTC()
{
  Wire.begin();
  if (!RTC.begin()) {
    myfile.println("RTC failed");
  }
}

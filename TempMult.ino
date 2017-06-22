
/**************************************************************************/
/*!
This is a demo for the Adafruit MCP9808 breakout
----> http://www.adafruit.com/products/1782
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!
*/
/**************************************************************************/

#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor0 = Adafruit_MCP9808();
Adafruit_MCP9808 tempsensor1 = Adafruit_MCP9808();
Adafruit_MCP9808 tempsensor2 = Adafruit_MCP9808();
Adafruit_MCP9808 tempsensor3 = Adafruit_MCP9808();
#define HIGH_FREQ 700
#define LOW_FREQ 200
#define UPPER_TEMP_THRESH 28
String blank  = "";
String tab = "     ";
const int buzzerPin = 15;
const int doorPin = 3; 
float temps[] = {0,0,0,0};
void playSound(int cNum, int frequency);

void initialize();
void getTemps();

void setup() {
  Serial.begin(9600);
  Serial.println("MCP9808 demo");
    pinMode(buzzerPin, OUTPUT);
    pinMode(doorPin, INPUT); 
    digitalWrite(doorPin, HIGH);
  
  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
initialize();
}

void loop() {

if(digitalRead(doorPin0) == HIGH){playSound(1, LOW_FREQ); }
if(digitalRead(doorPin1) == HIGH){playSound(2, LOW_FREQ); }
if(digitalRead(doorPin2) == HIGH){playSound(3, LOW_FREQ); }
if(digitalRead(doorPin3) == HIGH){playSound(4, LOW_FREQ); }
  getTemps();

if(temps[0]> UPPER_TEMP_THRESH) {
  playSound(1 ,HIGH_FREQ);
  Serial.println("FIRE IN CABINET 1");
}
if(temps[1]> UPPER_TEMP_THRESH) {
  playSound(2 ,HIGH_FREQ);
 Serial.println("FIRE IN CABINET 2");
 }
if(temps[2]> UPPER_TEMP_THRESH){
   Serial.println("FIRE IN CABINET 3");
   playSound(3 ,HIGH_FREQ);
}
if(temps[3]> UPPER_TEMP_THRESH){
  playSound(4 ,LOW_FREQ);
   Serial.println("FIRE IN CABINET 4");
}

  // Read and print out the temperature, then convert to *F
  Serial.print(tab + "Sensor 0: " + temps[0] + "*C" + tab + "Sensor 1: " + temps[1]+ "*C" + tab + "Sensor 2: " + temps[2]+ "*C" + tab + "Sensor 3: " + temps[3]+ "*C" );
            Serial.println("");
  //Serial.println("Shutdown MCP9808.... ");
  //tempsensor.shutdown(); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  
  delay(5000);
}

void initialize() {
    if (!tempsensor0.begin(0x18)) {
    Serial.println("Couldn't find Sensor 0");
  }
  if (!tempsensor1.begin(0x19)) {
    Serial.println("Couldn't find Sensor 1");
  }
    if (!tempsensor2.begin(0x1A)) {
    Serial.println("Couldn't find Sensor 2");
  }
    if (!tempsensor3.begin(0x1C)) {
    Serial.println("Couldn't find Sensor 3");
  }
}

void playSound(int cNum, int frequency) {

  for(int k = 0; k<cNum; k++) {
    tone(buzzerPin, frequency, 500);
    delay(1000);
    tone(buzzerPin, 0, 400);
    }    
  }
void getTemps() {
temps[0] = tempsensor0.readTempC();
temps[1] = tempsensor1.readTempC();
temps[2] = tempsensor2.readTempC();
temps[3] = tempsensor3.readTempC();
  }
  


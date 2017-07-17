
//<-------------Included Libraries---------------->
#include <Wire.h>
#include "Adafruit_MCP9808.h"
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <SD.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
//<----------------------------------------------->


/* ************************************************************************
 * ***              Hot Room Temperature Monitor                        ***
 * ************************************************************************
 * Built using parts of:
 * Super Graphing Data Logger by Everett Robinson, December 2016 http://everettsprojects.com
 * 
 * 
 * 
 * Authors:
 * Ejnar Arechavala - github.com/ejnarvala
 * Serena Liu - github.com/serliu
 * 
 * 
 * 
 */


//<-------------STATIC DEFINIITONS----------------->
#define UPPER_TEMP_THRESH 28
#define LOWER_TEMP_THRESH 18
#define UPPER_GAS_THRESH 400
#define LOWER_GAS_THRESH 100
#define MEASURE_INTERVAL_NORMAL 30000
#define MEASURE_INTERVAL_EMERGENCY 5000
#define DAYS_BETWEEN_NEW_LOG 1 //how often to make a new log
#define ADDR_COUNT 1 //address for where day count will be stored in EEPROM
#define ADDR_DAY 0 //address for where day number " "
#define NUM_SENSORS 2
//<------------------------------------------------>


//<-----------------IP SETTINGS-------------------->
byte mac[] = { 0x8A, 0x7F, 0xA7, 0x2F, 0x8D, 0xE0 };  
IPAddress ip(30,30,30,90);
IPAddress gateway(30,30,30,254);
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(80);
int port = 80; //only open port on this network
EthernetClient client;
//<---------------END IP SETTINGS------------------>


//<------------------LOGGING VARIABLES--------------------->
String file_name;
int filenum;
tmElements_t tm;
unsigned long lastIntervalTime = 0;
unsigned long lastResetTime = 0;
long measure_interval = MEASURE_INTERVAL_NORMAL; //Time between measurements
String str2log = "";
//<------------------------------------------------>


//<------------------SENSOR VARIABLES-------------->
Adafruit_MCP9808 *sensors = (Adafruit_MCP9808*)malloc(NUM_SENSORS*sizeof(Adafruit_MCP9808));
int *sensor_addresses= (int*) malloc (NUM_SENSORS*sizeof(int));
float *temps = (float*) malloc (NUM_SENSORS* sizeof(float));
bool emergency_mode = false;
//<------------------------------------------------>



void setup(){

  //Start Serial monitoring
  Serial.begin(9600);
  
  //Start SD card
  initialize_sd();
  //sdRWTest(); //Uncomment to test SD card Read/Write

  
  //Start Ethernet Connection
  initialize_ethernet();

  
  //Start Temperature Sensor
  initialize_tempsensor(sensors, sensor_addresses);


  RTC.read(tm);
  Serial.println("Today is " + twoDigitString(tm.Month) + "-" + twoDigitString(tm.Day) + "-" + (String) tmYearToCalendar(tm.Year));
  getTemps(temps, sensors);
  Serial.print("Initial Temperatures:");
  for(int i = 0; i < NUM_SENSORS; i++){
    Serial.print(" " + (String) temps[i]);
  }
  Serial.println("");
  //set filename upon restart
  file_name = "data/" + ((String)tmYearToCalendar(tm.Year)).substring(2) + "-" + twoDigitString(tm.Month) + "-" + twoDigitString(tm.Day) + ".CSV";
}

void loop(){
  if((millis()%lastResetTime >= 18000000)){ //reset ethernet every 5 hours for stability
      Ethernet.begin(mac, ip, gateway, gateway, subnet);
      server.begin();
      Serial.println("Server reset @ " + (String) Ethernet.localIP());
      lastResetTime = millis();
  }
  //Check if its time to take a new measurement
  if((millis()%lastIntervalTime) >= measure_interval){ //if its time, get new measuremnt and record it
    
    //Check if a day has passed to create a new log
    RTC.read(tm);
    Serial.println("Today is " + twoDigitString(tm.Month) + "-" + twoDigitString(tm.Day) + "-" + (String) tmYearToCalendar(tm.Year));
  
    //If new day happnes, new log will be started
    //file_name = "data/" + twoDigitString(tm.Month) + "-" + twoDigitString(tm.Day) + "-" + ((String)tmYearToCalendar(tm.Year)).substring(2) + ".CSV"; //update file name with new date
  
    //This code can be used to wait for multiple days to pass. Uncomment below and comment out line above to use;
//    Serial.println("EEPROM Day: " + (String)EEPROM.read(ADDR_DAY));
    if(EEPROM.read(ADDR_DAY) != tm.Day){
      Serial.println((String)EEPROM.read(ADDR_DAY) + " != " + (String)tm.Day + " || NEW DAY");
      EEPROM.write(ADDR_DAY, tm.Day); //overwrite EEPROM day
      EEPROM.write(ADDR_COUNT, EEPROM.read(ADDR_COUNT) + 1); //increment count by one
    }
    //if number of days to make a new log has occured, make a new log
    if(EEPROM.read(ADDR_COUNT) >= DAYS_BETWEEN_NEW_LOG){
      EEPROM.write(ADDR_COUNT, 0); //reset count
      file_name = "data/" + ((String)tmYearToCalendar(tm.Year)).substring(2) + "-" + twoDigitString(tm.Month) + "-" + twoDigitString(tm.Day) + ".CSV";
    }
    
    getTemps(temps, sensors);
    str2log = twoDigitString(tm.Month) + '-' + twoDigitString(tm.Day) + '-' + (String)tmYearToCalendar(tm.Year) +  ' ' + twoDigitString(tm.Hour) + ':' + twoDigitString(tm.Minute) + ':' + twoDigitString(tm.Second);
    for(int i = 0; i < NUM_SENSORS; i++){
      str2log += (',' + (String) temps[i]); 
    }
    sdLog(file_name, str2log);
    Serial.println("Writing: "+ str2log);
    //If any sensors are out of bounds, send turn on emergency mode
    for (int n = 0; n < NUM_SENSORS; n++){
      if (temps[n] > UPPER_TEMP_THRESH || temps[n] < LOWER_TEMP_THRESH){
        emergency_mode = true;
      }
    }
    if(emergency_mode){
      Serial.println("EMERGENCY MODE ACTIVATED; MEASUREMENT INTERVAL CHANGED TO 5 SECONDS");
      measure_interval = MEASURE_INTERVAL_EMERGENCY;
      emergency_mode = false;
     }else{
      measure_interval = MEASURE_INTERVAL_NORMAL;
    }
    lastIntervalTime = millis(); //update time since last interval
  }
    
//<---------------------WEB PAGE CODE-------------------------------------------------->
  char clientline[BUFSIZ];
  int index = 0;
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    
    // reset the input buffer
    index = 0;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ) 
            index = BUFSIZ -1;
          
          // continue to read more data!
          continue;
        }
        
        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;
        
        // Print it out for debugging
        Serial.println(clientline);
        
        // Look for substring such as a request to get the root file
        if (strstr(clientline, "GET / ") != 0) {
          // send a standard http response header
          HtmlHeaderOK(client);
          // print all the data files, use a helper to keep it clean


          //Styling to make the website look pretty
          client.print(F("<!DOCTYPE html><html><style> #green{background:limegreen;} #red{background:red;} th{font-weight:bold;} th,td {padding: 12px; color: #fff; text-align: center; border: 0px none;} table{color: #fff; border: 1px solid #ddd; border-collapse: collapse; width: auto;}"));
          client.print(F("h1 {font-size: 42px; color: #fff;} h2{color: #fff; font-size: 32px;} html {background: #ddd; height: 100%;} ul{columns: 5}"));
          client.print(F("body { color: #fff; height: 100%; background: #3498db; box-shadow: 0 0 2px rgba(0, 0, 0, 0.06); color: #545454; font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; text-align: center; line-height: 1.5; margin: 0 auto; max-width: 900px; padding: 2em 2em 4em;} li { list-style-type: none; font-size: 18px; font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif;} </style>"));
          client.print(F("<head><title>Arduino Battery Lab Monitor</title></head><body>"));
          client.print(F("<h1>Arduino Battery Lab Monitor</h1>"));
          client.print(F("<table style=\"width:100%\"><tr><th>Temperature</th>"));
          getTemps(temps, sensors); //Update Temperature readings
          for (int i = 0; i < NUM_SENSORS; i++){
            if (temps[i] > UPPER_TEMP_THRESH || temps[i] < LOWER_TEMP_THRESH){
              client.print(F("<td id=\"red\">"));
            } 
            else {
              client.print(F("<td id=\"green\">"));
            }
            client.print((String)temps[i] + "</td>");
          }
          client.print(F("</tr></table>"));
          client.print(F("<h2>View data for the week of (mm-dd-yy):</h2>"));
          ListFiles(client);
          client.print(F("</body></html>"));
        }
        else if (strstr(clientline, "GET /json")!=0){ //used for live update data
          client.println(F("HTTP/1.1 200 OK"));
          client.println("Access-Control-Allow-Origin: *");
          client.println(F("Content-Type: application/json;charset=utf-8"));
//          client.println(F("Connection: close"));
          client.println(F(""));
          getTemps(temps, sensors);
          RTC.read(tm);
          //formatting into json
          client.print("{");
          client.print("\"date\": ");
          client.print("\"" +twoDigitString(tm.Month) + '-' + twoDigitString(tm.Day) + '-' + (String)tmYearToCalendar(tm.Year) +  ' ' + twoDigitString(tm.Hour) + ':' + twoDigitString(tm.Minute) + ':' + twoDigitString(tm.Second)+"\"");
          client.print(",");
          for(int i = 0; i < NUM_SENSORS; i++){
            client.print("\"S");
            client.print(i);
            client.print("\": ");
            client.print(temps[i]);
            if( i!=1 ){
              client.print(",");
            }
          }
          client.println("}");
//          client.stop();
          break;
        } else if (strstr(clientline, "GET /history")!=0){ //url for iframe in main app
          // send a standard http response header
          HtmlHeaderOK(client);
          client.println(F(""));
          client.print(F("<!DOCTYPE html><style> body{font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; color:#FBFBFF; height: 100%; background: #01AADA; color #FBFBFF; margin: 0 auto; max-width: 100%; padding: 2em 2em 4em;} a:link{ text-decoration:none; font-weight: bold;color:#fff;}a:visited {text-decoration:none;font-weight:bold;color:#ddd;}a:hover{text-decoration:underline;font-weight:bold;color:#fff;}"));
          client.print(F("a:active{text-decoration:underline;font-weight:bold;color: white;}th,td {padding: 7px;color: #FBFBFF;text-align: center;border: 1px solid #0B4F6C;} table{color: #FBFBFF; border-collapse: collapse; border: 1px solid #0B4F6C; width: 100%;table-layout: fixed;}</style><html><body>"));
          ListFiles(client);
          client.print(F("</body></html>"));
          
          
        }else if (strstr(clientline, "GET /") != 0) {
          // this time no space after the /, so a sub-file!
          char *filename;
          filename = strtok(clientline + 5, "?"); // look after the "GET /" (5 chars) but before
          // the "?" if a data file has been specified. A little trick, look for the " HTTP/1.1"
          // string and turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;
          // print the file we want
          Serial.println(filename);
          File file = SD.open(filename,FILE_READ);
          if (!file) {
            HtmlHeader404(client);
            break;
          }
          
          Serial.println("Opened!");
          //HtmlHeaderOK(client);
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println("");
          while(file.available()) {
            int num_bytes_read;
            uint8_t byte_buffer[32];
            num_bytes_read=file.read(byte_buffer,32);
            client.write(byte_buffer,num_bytes_read);
          }
          file.close();
        }
        else {
          // everything else is a 404
          HtmlHeader404(client);
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    client.stop();
  }
  
  
}





void initialize_tempsensor(Adafruit_MCP9808 *sensors, int *sensor_addresses){
  Serial.println("Initializing Temp Sensors");
  for(int i = 0; i < NUM_SENSORS; i++){
    sensor_addresses[i] = 24 + i;
    sensors[i] = Adafruit_MCP9808();
    if(!sensors[i].begin(sensor_addresses[i])){
      Serial.println("Couldn't find Sensor " + (String) i);
    }
  }
//  if (!tempsensor0.begin(0x18)) {
//    Serial.println("Couldn't find Sensor 0");
//  }
//  if (!tempsensor1.begin(0x19)) {
//    Serial.println("Couldn't find Sensor 1");
//  }
  Serial.println(F("Temperature Sensors Initialized"));
}



// Sets up DHCP, waits 25s to make sure its connected
//sets some pin outputs for sd card
void initialize_ethernet() 
{
  Serial.println("Initializing Ethernet");
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(25000);    //Long enough to be fuly set up
  server.begin();
  Serial.println(Ethernet.localIP());
}


//Sets up SD card
void initialize_sd() {
  pinMode(4, OUTPUT); //Needed to not conflict with ethernet
  digitalWrite(4, HIGH);
  while (!SD.begin(4)) { //Initializes sd card
    Serial.println(F("Card failed, or not present")); //Prints if SD not detected
    //return; // don't do anything more:
    delay(1000);
  }
  Serial.println(F("SD card initialized."));
}



//Run this test to create and remove a text file
//on the SD card to test Read/Write functionality
void sdRWTest(void){
  File myFile;
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  Serial.println("Creating example.txt...");
  myFile = SD.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists:
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // delete the file:
  Serial.println("Removing example.txt...");
  SD.remove("example.txt");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }
}


//Inputs char array pointer to file name on sd and
//String of what to append to that file;
void sdLog(String fileName, String stringToWrite) {
  File myFile = SD.open(fileName, FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to ");
    Serial.print(fileName);
    Serial.print("...");
    myFile.println(stringToWrite);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(fileName);
  }
}




// A function that takes care of the listing of files for the
// main page one sees when they first connect to the arduino.
// it only lists the files in the /data/ folder. Make sure this
// exists on your SD card.
void ListFiles(EthernetClient client) {

  File workingDir = SD.open("/data/");
  int filecount = 0;
  client.print("<table><tr>");
    while(true) {
      File entry =  workingDir.openNextFile();
      if (! entry) {
        break;
      }
      if ((filecount % 5 == 0) && (filecount != 0)){
        client.print("</tr><tr>");
      }
      client.print("<td><a href=\"/HC.htm?file=");
      client.print(entry.name());
      client.print("\">");
      client.print(entry.name());
      client.println("</a></td>");
      entry.close();
      filecount++;
    }
  client.println("</tr></table>");
  //Serial.println("Number of files: " + (String) filecount);
  if(filecount > 29){
    removeOldestLog();
  }
  workingDir.close();
}

void removeOldestLog(){
  File workingDir = SD.open("/data/");
  File entry = workingDir.openNextFile();
  SD.remove("/data/" + (String) entry.name());
  Serial.println("Oldest Log Removed");
}

void getTemps(float *temps, Adafruit_MCP9808 *sensors) {
  for(int i = 0; i < NUM_SENSORS; i++){
    temps[i] = sensors[i].readTempC();
  }
//    temps[0] = tempsensor0.readTempC();
//    temps[1] = tempsensor1.readTempC();
 }


String twoDigitString(int num){
  if(num < 10){
    return '0' + (String) num;
  }
  else{
    return (String) num;
  }
}

//-------------------RAM SAVING HTML HEADERS----------------------
// Strings stored in flash mem for the Html Header (saves ram)
const char HeaderOK_0[] PROGMEM = "HTTP/1.1 200 OK";            //
const char HeaderOK_1[] PROGMEM = "Content-Type: text/html";    //
//const char HeaderOK_2[] PROGMEM = "Connection: keep-alive";     // the connection will be closed after completion of the response
//const char HeaderOK_3[] PROGMEM = "Refresh: 5";                 // refresh the page automatically every 5 sec
const char HeaderOK_4[] PROGMEM = "";                           //

// A table of pointers to the flash memory strings for the header
const char* const HeaderOK_table[] PROGMEM = {   
  HeaderOK_0,
  HeaderOK_1,
//  HeaderOK_2,
//  HeaderOK_3,
  HeaderOK_4
};

// A function for reasy printing of the headers  
void HtmlHeaderOK(EthernetClient client) {
  
    char buffer[30]; //A character array to hold the strings from the flash mem
    
    for (int i = 0; i < 3; i++) {
      strcpy_P(buffer, (char*)pgm_read_word(&(HeaderOK_table[i]))); 
      client.println( buffer );
    }
} 
  
  
// Strings stored in flash mem for the Html 404 Header
const char Header404_0[] PROGMEM = "HTTP/1.1 404 Not Found";     //
const char Header404_1[] PROGMEM = "Content-Type: text/html";    //
const char Header404_2[] PROGMEM = "";                           //
const char Header404_3[] PROGMEM = "<h2>File Not Found!</h2>"; 

// A table of pointers to the flash memory strings for the header
const char* const Header404_table[] PROGMEM = {   
  Header404_0,
  Header404_1,
  Header404_2,
  Header404_3
};

// Easy peasy 404 header function
void HtmlHeader404(EthernetClient client) {
  
    char buffer[30]; //A character array to hold the strings from the flash mem
    
    for (int i = 0; i < 4; i++) {
      strcpy_P(buffer, (char*)pgm_read_word(&(Header404_table[i]))); 
      client.println( buffer );
    }
} 
//-------------------------------------------------------------------




# HotroomMonitoring
A monitoring system for a hotroom test setup with many desktops running performance tests
The applicaiton sends requests to an arduino with temperature sensors attached which returns the temperature values and displays them first in the table at the top and appends the data to a live running highcharts graph on the left pane. The Right pane shows a webpage hosted on the arduino which allows for accessing and downloading of past data logs. The pingtest powershell script sends pings to all the ips from the iplist and populates the bad ip list with the ones that do not respond. The bottom chart shows a list of IP addresses from the [ip list](/webpage/iplist.txt) and cross references it to the [bad ip list](/webpage/badips.txt). Any IP's that show up bot lists will be highlighted in red.


### Hardware
* Arduino Mega
* Arduino Ethernet Shield 2
* Adafruit Chronodot v2.1
* Adafruit MCP9809 Temperature Sensors
* MicroSD Memory Card

### Software
* Text Editor (e.g. [Atom](https://atom.io/), [Brackets](http://brackets.io/), or [Visual Studio Code](https://code.visualstudio.com/))
* [HFS](http://www.rejetto.com/hfs/) (Included in the packaged folder)
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)
  * [Adafruit MCP9808 Library](https://github.com/adafruit/Adafruit_MCP9808_Library)
  * [DS1207RTC Library](https://github.com/PaulStoffregen/DS1307RTC)
  * [Ethernet2 Libarary](https://github.com/adafruit/Ethernet2/tree/master/examples)
* [Electron Packager](https://github.com/electron-userland/electron-packager) (For packaging into an executable files)

# Installation
## Arduino Assembly
The temperature sensors can be chained together along with the Chronodot which all use SDL/SDA ports on the arduino. Make sure to pull the teperature to high as shown in the picture below.
![Picture of Arduino](/ArduinoLayout.jpg "")

## Arduino Calibration & Setup
1. Calibrate the chronodot by using the DS1207RTC libary example sketches. Set the time with the SetTime sketch and verify it is working using the ReadTest sketch.
2. Load the [Hot Room Monitoring sketch](/Hot_Room_Monitor/Hot_Room_Monitor.ino) into the Arduino IDE and set variables such as IP address, Days between new logs, temperature thresholds, number of sensors, etc. which are all listed at the beginning of the code. In the setup function, the sdRWTest() can be uncommented to perform a test of the SD functionality at the beginning. Compile and upload the sketch to the Adruino via USB.
3. The Arduino needs javascript files to be hosted either on a computer on the local network, ideally the computer that will be running the monitoring program, or through an online CDN. The local computer method will be explianed more later, but for now set the paths in [HC.htm](/SD/HC.htm). Copy  [HC.htm](/SD/HC.htm) to the root of the Arduinos SD card. Also make sure that there is a folder named "data" present in the SD card's root.
4. Insert the SD card into the Arduino and test the system by connecting it to the network via Ethernet and monitoring the Serial output. Requests can be made by going to the Adruino's local IP address + /json (e.g. "192.168.100.100/json") or /history (e.g. "192.168.100.100/history") which should return a json object or a webpage, respectively.

## Monitoring Computer Setup
1. All the necessary files are prepackaged in the [HotroomMonitor-win32-x64 Folder](/webpage/HotroomMonitor-win32-x64). They can be repackaged by running the following command within the [webpage folder](/webpage).
```
electron-packager . --overwrite
```
2. Within this folder, there is a [Start.bat file](/webpage/HotroomMonitor-win32-x64/resources/app/Start.bat) that will run the powershell scripts to ping IP addresses and kill the computers if the threshold is passed. HFS.exe will also begin which will allow for hosting of local files on a local network. Use this to host the folder with the javascript files for the Arduino located in the [js folder](webpage/HotroomMonitor-win32-x64/resources/app/js). Edit this with correct paths should you move any of the files to other directories.
3. The application can be configured through the javascript file in the [packaged folder](webpage/HotroomMonitor-win32-x64/resources/app/js/update.js) for editing without having to recompile, or in the [source folder](webpage/js/update.js) which can be repackaged using electron packager. Some configuration options include temperature thresholds, and refresh times of application elements.
4. Edit the [IP list](HotroomMonitoring/webpage/HotroomMonitor-win32-x64/resources/app/iplist.txt) with the IP's that you would like the scrpit to ping.
5. Use [Start.bat](/webpage/HotroomMonitor-win32-x64/resources/app/Start.bat) to run all the scrips at once or open them individually.




Here are some screenshots:
![Screenshot 1](/sc_1.PNG "")
![Screenshot 2](/sc_2.PNG "")

# HotroomMonitoring
A monitoring system for a hotroom test setup with many desktops running performance tests
The applicaiton sends requests to an arduino with temperature sensors attached which returns the temperature values and displays them first in the table at the top and appends the data to a live running highcharts graph on the left pane. The Right pane shows a webpage hosted on the arduino which allows for accessing and downloading of past data logs. The ping batch file sends pings to all the ips from the ip list and populates the bad ip list with the ones that do not respond. The bottom chart shows a list of IP addresses from the [ip list](/webpage/iplist.txt) and cross references it to the [bad ip list](/webpage/badips.txt). Any IP's that show up bot lists will be highlighted in red.

Here are some screenshots:
![Screenshot 1](/sc_1.PNG "")
![Screenshot 2](/sc_2.PNG "")

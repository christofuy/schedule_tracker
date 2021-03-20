# Schedule Tracker
## Purpose
A student's calendar schedule is fetched from the Canvas API to display and alert
the student about their next class.
## Flow
1. ESP8266-01 makes HTTPS request to the Canvas API with your temporary Canvas
access token in the authorization header.
2. ESP8266-01 then parses the String payload into JSON using the ArduinoJson
library and serializes the data to send to the Arduino Mega 2560.
3. Arduino Mega receives the String of the list of events and times and stores them
in an array.
4. Name and time of event are displayed on the LCD and the passive buzzer rings
when it is 5 minutes before the start of the event time
## Components
- Liquid Crystal Display
- Arduino Mega 2560
- Real-time Clock Module (DS3231)
- ESP8266-01 WiFi module
- Passive Buzzer
- Push Button
## Limitations
- Canvas API only returns calendar events that YOU create and not all events that
are created by your professors/teachers
- I am not yet proficient in Arduino/C++, and the project was developed when I
was inexperienced. There is some spaghetti code, and no documentation. As I had to
return the Arduino kit, I cannot test and improve the code at this time.

#include <SoftwareSerial.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

//Init Button
const int buttonPin = 8;
int buttonState = 0;

//INIT PBUZZER
const int buzzerPin = 13;
bool appointmentAlarm = false;

//Init LCD Display
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Init ESP
SoftwareSerial esp(12, 11); //RX, TX of ESP-01 is wired to D11, D12 pins respectively
String aptmts[] = {"", "", "", "", "", "", "", "", "", ""};
String nextTime = "Loading:";
String nextEvent = "Setting Up";

//Init Real Time Clock
DS3231 rtc(SDA, SCL);

//SOME TIMING CONSTANTS
const unsigned long eventInterval = 1000;
int timeCount = 0; unsigned long prevTime = 0;
bool displayEvents = true;
bool receivingData = true;

void setup() {
  Serial.begin(115200);

  //ESP-01 Module
  esp.begin(115200);
  esp.write("r");

  //RTC Module
  rtc.begin();
// uncomment this to set the time on your RTC module for the first time
//  rtc.setDOW(FRIDAY);  rtc.setTime(16, 53, 40);  rtc.setDate(11, 12, 2020);

  //LCD Display
  lcd.begin(16, 2);
  lcd.print(nextTime);
  lcd.setCursor(0, 1); lcd.print(nextEvent);

  //Button input
  pinMode(buttonPin, INPUT);
  //BUZZER OUTPUT
  pinMode(buzzerPin, OUTPUT);

  Serial.println("Set up complete.");
  while (!esp.available());
}



void loop() {
  unsigned long currTime = millis();

  if (esp.available()) readAndStoreData();
  determineNextAppointment();

  //DETERMINING IF 5 MINUTES BEFORE APPOINTMENT TIME
  if (nextTime != "You have no more") appointmentAlarm = checkAlarm(nextTime, String(rtc.getTimeStr()));
  if (appointmentAlarm) signalAlarm();

  displayEventOrDT(currTime);

  //LISTENING TO REFRESH BUTTON
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    displayContent("Reloading:", "FetchingCalendar");
    esp.write("r");
  }
}


//AUXILIARY FUNCTIONS###################################################################################################

void displayEventOrDT(unsigned long currTime) { //DISPLAYING EVENTS AND TIME/DATE PERIODICALLY
    if (currTime - prevTime >= eventInterval) { //every 1 second
    if (displayEvents) displayContent(nextTime, nextEvent);
    else displayContent("Time: " + String(rtc.getTimeStr()), "Date: " + String(rtc.getDateStr()));
    timeCount++;
    if (timeCount == 5) {
      displayEvents = !displayEvents;
      timeCount = 0;
    }
    prevTime = currTime;
    }
}


void determineNextAppointment() { //ITERATING OVER APPOINTMENTS ARRAY AND DETERMINING WHAT TO DISPLAY
  bool broken = false;
  for (int i = 0; i < 10; i++) {
    String appointment = aptmts[i];
    if (appointment != "") {
      if (checkAppointment(String(rtc.getTimeStr()), appointment.substring(6, 11))) {
        nextTime = appointment.substring(0, 11);
        nextEvent = appointment.substring(11);
        broken = true;
        break;
      }
    }
  }
  if (!broken) {
    nextTime = "You have no more";
    nextEvent = "Appointments! :)";
  }
}


void readAndStoreData() {
  displayContent("Loading:", "Reading Data");
  String payload = "";
  while (esp.available())payload = esp.readString();
  int left = 0, openIndex = 0;
  for (int i = 0; i < payload.length(); i++) {
    if (payload[i] == ',') {
      aptmts[openIndex] = payload.substring(left, i);
      openIndex++;
      left = i + 1;
    }
  }
}


void displayContent(String line1, String line2) {
  lcd.clear(); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}


void signalAlarm() {
  for (int i = 0; i < 500; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
  for (int i = 0; i < 500; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(2);
    digitalWrite(buzzerPin, LOW);
    delay(2);
  }
}


bool checkAlarm(String apptmtTime, String currTime) {
  int apptmthr = (apptmtTime[0] == '0') ? apptmtTime.substring(1, 2).toInt() : apptmtTime.substring(0, 2).toInt();
  int apptmtmin = (apptmtTime[3] == '0') ? apptmtTime.substring(4).toInt() : apptmtTime.substring(3).toInt();
  int currhr = (currTime[0] == '0') ? currTime.substring(1, 2).toInt() : currTime.substring(0, 2).toInt();
  int currmin = (currTime[3] == '0') ? currTime.substring(4, 5).toInt() : currTime.substring(3, 5).toInt();
  int currsec = (currTime[6] == '0') ? currTime.substring(7).toInt() : currTime.substring(6).toInt();
  return (currhr == apptmthr && apptmtmin - currmin == 5 && currsec == 0);
}


bool checkAppointment(String now, String appointment) {
  int nowHour, appHour;
  if (now[0] == '0')nowHour = now.substring(1, 2).toInt();
  else nowHour = now.substring(0, 2).toInt();
  int nowMinutes = now.substring(3, 5).toInt();
  if (appointment[0] == '0')appHour = appointment.substring(1, 2).toInt();
  else appHour = appointment.substring(0, 2).toInt();
  int appMinutes = appointment.substring(3, 5).toInt();
  return (nowHour < appHour || (nowHour == appHour && nowMinutes < appMinutes));
}

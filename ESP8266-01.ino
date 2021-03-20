/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "" //TODO: YOUR WIFI NAME
#define STAPSK  "" //TODO: YOUR WIFI PASSWORD
#endif

const char* token=""; //TODO: YOUR CANVAS ACCESS TOKEN (generate from profile page)
const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = ""; //TODO: CANVAS HOST YOU ARE TRYING TO FETCH DATA FROM
const int httpsPort = 443;
const char fingerprint[] PROGMEM = ""; //TODO: YOUR SHA256 FINGERPRINT

String strAptmts[]={"","","","","","","","","",""};
String aptmts[]={"","","","","","","","","",""};

String payload;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){delay(1);}
}



//LOOP################################################################################################################
void loop() {
  if(Serial.available()){
    //REMOVE INPUT SIGNAL
    Serial.readString();

    //REQUEST API TO FETCH DATA
    requestAPI();

    //PARSE JSON AND WRITE OUTPUT
    if(payload==""){Serial.write(",");}
    else{
      separateJsonStrings();
      parseJsonStrings();
      writeOutput();
    }
  }
}


//AUXILARY FUNCTIONS##################################################################################################
void requestAPI(){
    WiFiClientSecure client;
    client.setFingerprint(fingerprint);
    if (!client.connect(host, httpsPort)) {
      Serial.println("--:--cxn failed,");
      return;
    }
    
    String url = "/api/v1/calendar_events";
    
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Authorization: Bearer " + token + "\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");
    
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
    }
    
    //READING PAYLOAD
    client.readStringUntil('\n');
    payload=client.readStringUntil('\n');
    payload.trim();
    payload=payload.substring(1,payload.length()-1);
}


void separateJsonStrings(){
  int openIndex=0,left=0,right=0;
  for(int i=0;i<payload.length();i++){
    if(payload[i]=='{')left=i;
    else if(payload[i]=='}'){
          right=i;
          strAptmts[openIndex]=payload.substring(left,right+1);
          openIndex++;
        }
  }
}


void parseJsonStrings(){
  for(int i=0;i<10;i++){
        if(strAptmts[i]!=""){
          char json[800];
          strAptmts[i].toCharArray(json,800);
          
          StaticJsonBuffer<1024> JSONBuffer;
          JsonObject& root=JSONBuffer.parseObject(json);
          if(!root.success()){
            Serial.println("ERROR PARSING JSON");
            return;
          }
          
          const char* start_at=root["start_at"]; String startTime=String(start_at);
          const char* end_at=root["end_at"]; String endTime=String(end_at);
          const char* event_title=root["title"]; String event=String(event_title);
          String entry=extractData(startTime,endTime,event);
          aptmts[i]=entry;
        }
    }
}


/*

*/
String extractData(String rawStart, String rawEnd, String rawTitle){
    String startHr=rawStart.substring(11,13); String startMin=rawStart.substring(14,16);
    String endHr=rawEnd.substring(11,13); String endMin=rawEnd.substring(14,16);
    if(startHr[0]=='0')startHr=startHr.substring(1);
    if(endHr[0]=='0')endHr=endHr.substring(1);

    int startHrInt=startHr.toInt()-8; int endHrInt=endHr.toInt()-8;
    if(startHrInt<0)startHrInt+=24;
    if(endHrInt<0)endHrInt+=24;

    String resStart=String(startHrInt)+":"+startMin;
    String resEnd=String(endHrInt)+":"+endMin;

    return (resStart+"-"+resEnd+rawTitle);
}


/*

*/
void writeOutput(){
  String output="";
  for(int i=0;i<10;i++){
    if(aptmts[i]!=""){
      output+=aptmts[i]+",";
    }
  }
  for(int i=0;i<output.length();i++){
    Serial.write(output[i]);
    delay(50);
  }
}

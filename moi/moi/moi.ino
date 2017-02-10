#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <TimeLib.h>
#include <TimeAlarms.h>


const char* ssid     = "ZAB";
const char* password = "Gearman1";
#define APPID       "MELON"
#define GEARKEY     "oNA35ZOOws6QDtr"
#define GEARSECRET  "tJOFJOfsDx7PINKwWANIKjK42"
#define ALIAS       "pcu1"

WiFiClient client;
MicroGear microgear(client);

// Date and time functions using a DS3231 RTC connected via I2C  D1&D2 and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

int timer = 0;
int relayPin[4] =  {D5,D6,D7,D8};
int time_pa = 0;
int time_pb = 0;
int time_mv = 0;
int time_wv = 0;
struct SchTime {
  int h;
  int m;
};
SchTime sch_ab[8];
SchTime sch_wt[8];

void(* resetFunc)(void)=0;

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char strState[msglen];
  for (int i = 0; i < msglen; i++) {
    strState[i] = (char)msg[i];
  }
  String stateStr = String(strState).substring(0,msglen);
  Serial.print(topic);Serial.print("-->");Serial.println(stateStr);

  if (String(topic) == "/MELON/cmd"){
     int no =  stateStr.substring(0,1).toInt();
     int state = stateStr.substring(1).toInt();     
     digitalWrite(relayPin[no],state ? HIGH:LOW);
  }
  
  if (String(topic) == "/MELON/reset"){
     ESP.restart();
  }
  
  if (String(topic) == "/MELON/set") {
     time_pa = getValue(stateStr, ',', 0).toInt();
     time_pb = getValue(stateStr, ',', 1).toInt();
     time_mv = getValue(stateStr, ',', 2).toInt();
     time_wv = getValue(stateStr, ',', 3).toInt();
     for (int i=0;i<8;i++){
      String strtime = getValue(stateStr,',',i+4);
      if (strtime.length() > 1) {
        sch_ab[i].h = getValue(strtime,':',0).toInt();
        sch_ab[i].m = getValue(strtime,':',1).toInt();
      }
      else {
        sch_ab[i].h = -1;
        sch_ab[i].m = -1;        
      }
      strtime = getValue(stateStr,',',i+12);
      if (strtime.length() > 1) {
        sch_wt[i].h = getValue(strtime,':',0).toInt();
        sch_wt[i].m = getValue(strtime,':',1).toInt();
      }
      else {
        sch_wt[i].h = -1;
        sch_wt[i].m = -1;        
      }
//      Serial.print(sch_ab[i].h);Serial.print(':');Serial.println(sch_ab[i].m);
//      Serial.print(sch_wt[i].h);Serial.print(':');Serial.println(sch_wt[i].m);
     }
      savedata(stateStr);
      readdata();
  }
  
  if (String(topic) == "/MELON/wtr") doWatering(); 
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
  microgear.subscribe("/cmd");
  microgear.subscribe("/set/#");
  microgear.subscribe("/wtr");
  microgear.subscribe("/reset");
}


void setup () {
  Serial.begin(9600);
  
  // NETPIE
  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);
  
  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, ALIAS);
    microgear.connect(APPID);
  }
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  for (int i=0;i<4;i++)
    pinMode(relayPin[i], OUTPUT);

  //SPTFFS
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  File f = SPIFFS.open("/config.txt", "r");  
  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");
    savedata("0");
  }else {
    readdata();
  }   
  f.close();


  DateTime now = rtc.now();    
  setTime(now.hour(),now.minute(),now.second(),now.day(),now.month(),now.year()-2000); //(h,m,d,) set time to Saturday 8:29:00am Jan 1 2011

  // create the alarms, to trigger at specific times
  for (int i=0;i<8;i++) {
    if (sch_ab[i].h != -1)
      Alarm.alarmRepeat(sch_ab[i].h,sch_ab[i].m,0, doWatering);
    if (sch_wt[i].h != -1)
      Alarm.alarmRepeat(sch_wt[i].h,sch_wt[i].m,0, doWaterOnly);
  }
}


void loop(){
  ArduinoOTA.handle();
  if (microgear.connected()) {
      microgear.loop();
      if (timer >= 1000) {
        // publish datetime
        DateTime now = rtc.now();    
        String timestr = String(now.year())+'/'+String(now.month())+'/'+String(now.day())+' '+String(now.hour())+':'+String(now.minute())+':'+String(now.second());  
        microgear.publish("/time",timestr);

        // publish status and settings
        String statusStr = String(digitalRead(relayPin[0])) + ',';
        statusStr += String(digitalRead(relayPin[1]))+',';
        statusStr += String(digitalRead(relayPin[2]))+',';
        statusStr += String(digitalRead(relayPin[3]))+',';
        statusStr += String(time_pa)+',';
        statusStr += String(time_pb)+',';
        statusStr += String(time_mv)+',';
        statusStr += String(time_wv);
        microgear.publish("/status",statusStr);
        
        // publist satatus/sch/ab
        String schstr="";
        for (int i=0;i<7;i++) {
          if (sch_ab[i].h != -1 )
            schstr += String(sch_ab[i].h)+':'+String(sch_ab[i].m)+',';
          else 
            schstr += ',';
        }
        if (sch_ab[7].h != -1)
          schstr += String(sch_ab[7].h)+':'+String(sch_ab[7].m);
        //Serial.println(schstr);

        microgear.publish("/status/sch/ab",schstr);
       
        // publist satatus/sch/wt
        schstr="";
        for (int i=0;i<7;i++) {
          if (sch_wt[i].h != -1 )          
            schstr += String(sch_wt[i].h)+':'+String(sch_wt[i].m)+',';
          else 
            schstr += ',';
        }
        if (sch_wt[7].h != -1)
          schstr += String(sch_wt[7].h)+':'+String(sch_wt[7].m);
  
        microgear.publish("/status/sch/wt",schstr);
        
        timer = 0;
      } 
      else {
        timer += 100;
      }
  }
  else {
      Serial.println("connection lost, reconnect...");
      if (timer >= 5000) {
        microgear.connect(APPID);
        timer = 0;
      }
      else {
        timer += 100;
      }
  }
  Alarm.delay(100);
}

void doWaterOnly() {
  Serial.println("Water Only");
  digitalWrite(relayPin[3],HIGH);
  delay(time_wv*1000);
  digitalWrite(relayPin[3],LOW);  
}

void doWatering() {
  //pump A start
  Serial.println("Watering ....");
  digitalWrite(relayPin[0],HIGH);
  delay(time_pa*1000);
  digitalWrite(relayPin[0],LOW);
  // main valve start
  digitalWrite(relayPin[2],HIGH);
  delay(time_mv/2*1000);
  digitalWrite(relayPin[2],LOW);
  
  //pump B start
  digitalWrite(relayPin[1],HIGH);
  delay(time_pa*1000);
  digitalWrite(relayPin[1],LOW);
  // main valve start
  digitalWrite(relayPin[2],HIGH);
  delay(time_mv/2*1000);
  digitalWrite(relayPin[2],LOW);
}

void readdata() {
    File f = SPIFFS.open("/config.txt", "r");
    Serial.println("Reading coniguration.");
    while(f.available()) {
      String line = f.readStringUntil('n');
      Serial.println(line);
      time_pa = getValue(line, ',', 0).toInt();
      time_pb = getValue(line, ',', 1).toInt();
      time_mv = getValue(line, ',', 2).toInt();
      time_wv = getValue(line, ',', 3).toInt();

      for (int i=0;i<8;i++){
        String strtime = getValue(line,',',i+4);
        if (strtime.length() > 1) {
          sch_ab[i].h = getValue(strtime,':',0).toInt();
          sch_ab[i].m = getValue(strtime,':',1).toInt();
        }
        else {
          sch_ab[i].h = -1;
          sch_ab[i].m = -1;        
        }
        strtime = getValue(line,',',i+12);
        if (strtime.length() > 1) {
          sch_wt[i].h = getValue(strtime,':',0).toInt();
          sch_wt[i].m = getValue(strtime,':',1).toInt();
        }
        else {
          sch_wt[i].h = -1;
          sch_wt[i].m = -1;        
        }
      }
    }

    // create the alarms, to trigger at specific times
    for (int i=0;i<8;i++) {
      if (sch_ab[i].h != -1)
        Alarm.alarmRepeat(sch_ab[i].h,sch_ab[i].m,0, doWatering);
      if (sch_wt[i].h != -1)
        Alarm.alarmRepeat(sch_wt[i].h,sch_wt[i].m,0, doWaterOnly);
    }

    
    f.close();
}

void savedata(String settings) {
    File f = SPIFFS.open("/config.txt", "w");
    if (!f) {
      Serial.println("file creation failed");
    } 
    else {
      f.print(settings);
    }
    f.close();    
} 

String getValue(String data, char separator, int index){
    int maxIndex = data.length() - 1;
    int j = 0;
    String chunkVal = "";
    for (int i = 0; i <= maxIndex && j <= index; i++){
        chunkVal.concat(data[i]);
        if (data[i] == separator){
            j++;
            if (j > index){
                chunkVal.trim();
                return chunkVal;
            }
            chunkVal = "";
        }
        else if ((i == maxIndex) && (j < index)) {
            chunkVal = "";
            return chunkVal;
        }
    }   
}

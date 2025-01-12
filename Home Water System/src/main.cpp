//Libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//Lookup Table
int lookup[100]{2,
7,12,19,26,35,44,53,63,74,85,96,108,120,133,146,159,173,187,201,216,231,246,261,276,292,308,324,340,357,373,390,407,424,441,458,
476,493,510,528,546,563,581,599,617,635,653,671,689,707,725,743,761,779,797,815,832,850,868,886,903,921,938,956,973,990,1007,
1024,1040,1057,1073,1090,1106,1122,1137,1153,1168,1183,1198,1212,1227,1241,1254,1268,1281,1293,1306,1318,1329,1340,1351,1361,
1370,1379,1387,1395,1401,1407,1411,1414};

//Pin Definitions
#define trigPin D1
#define echoPin D5
#define relayPin D0
#define ledPin D6



//Water Tank Dimensions(in cm)
//Note: Only Works With Cylindrical Tanks. If you have any other type of tank, you may replace the calculate fill percent function with your desired function.
#define tankRadius 100
#define tankLength 180
//Low Pass Filter
float filteredValue = 0;
const float alpha = 0.05;

//Global Vars
float lastPercent = 0;
int dataSent = 0;


//Settings
#define pumpCutOffPercent 20
#define wifiTimeoutLimit 30 //in seconds
#define medianFilterSize 10

//Wifi Info
const char* ssid = "****";
const char* password = "****";
const char* serverDataSendURL = "****";
const char* serverDataRecieveURL = "****";



//Function Declarations
void initialize();
void wifiConnect();
float measureLevel();
int sendVolumeData(float percentage);
String readSwitchData();
void switchToggle(String switchdata);


//HTTP Class Definitions
WiFiClient client;
HTTPClient http;


//Code 
void setup() {
 initialize();
}
void loop() {
  float sum = 0;
  int i = 0;
  while(i <10){
      filteredValue = alpha * measureLevel()  + (1 - alpha) * filteredValue;
      sum += filteredValue;
      delay(50);
      i++;
  }
  Serial.print("Sum:");
  Serial.println(sum/10);
  Serial.print("Volume:");
  float percent = (lookup[tankRadius - int(sum/10)] / 1414.0) * 100;
  Serial.println(percent);
  if((0< percent && percent < 100)){
      if((abs(lastPercent - percent) > 2) || dataSent == 0){
         dataSent = sendVolumeData(percent);
         lastPercent = percent;
      }
       
  }  
  else {sendVolumeData(1.01);
        dataSent = 0;
  }
  switchToggle(readSwitchData());
  delay(2000);
}


void initialize(){
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(ledPin,OUTPUT);
  pinMode(relayPin, OUTPUT);
  Serial.println("Pins configured");
  wifiConnect();
}

void wifiConnect(){
  int wifiTries = 0;
  WiFi.begin(ssid, password);
  
  while ((WiFi.status() != WL_CONNECTED) && wifiTries < wifiTimeoutLimit) {
    wifiTries++;
    delay(1000);
    Serial.println("Connecting to WiFi...");

  }
  if(WiFi.status() == WL_CONNECTED)
    Serial.println("Connected to WiFi");
  else{ 
    Serial.println("Wifi Connection Timeout, Restarting");
    ESP.restart();
  }
}

float measureLevel(){
  float duration;
  float distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  return distance;
}

int sendVolumeData(float percentage){
    if(WiFi.status() == WL_CONNECTED){
      http.begin(client,serverDataSendURL);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String postData = String(int(percentage)); 
      int httpCodePost = http.POST(postData);
      
      if (httpCodePost > 0){
        String payloadPost = http.getString(); 
        Serial.printf("POST Response Code: %d\n", httpCodePost);
        Serial.println("POST Response: " + payloadPost);
        return 1;
        
    } 
    else {
      Serial.printf("POST request failed, error: %s\n", http.errorToString(httpCodePost).c_str());
      http.end();
      return 0; 
    }
    
    }  
   return 0;
}
String readSwitchData(){
  String payload = "255";
  
  if(WiFi.status() == WL_CONNECTED){
      http.begin(client,serverDataRecieveURL);
      int httpCode = http.GET();
       if (httpCode > 0) {
      payload = http.getString();
      Serial.printf("GET Response Code: %d\n", httpCode);
      Serial.println("GET Response: " + payload);
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  return payload;   
}
void switchToggle(String switchdata){
  if(switchdata == "AC"){
    digitalWrite(relayPin,LOW);

  }
  else if(switchdata == "KAP"){
    digitalWrite(relayPin,HIGH);

  }
  else {Serial.println("Wrong Switch Data");}
}

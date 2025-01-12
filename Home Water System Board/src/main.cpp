#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Pins
#define redLedPin D0
#define yelLedPin D7
#define buttonPin D6
//Settings
  //Wifi
#define wifiTimeoutLimit 30
const char* ssid = "Dincer_Ailesi_Koy_Agi";
const char* password = "pusat2004@";
  //Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
  //Button
int lastButtonPress = 0;

//Class Def
ESP8266WebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Global Vars
String requestData = " ";
String buttonState = "AC";
//Function Dec
void handlePost();
void handleGET();
void wifiConnect();
void handleRoot();
void isButtonPressed();
void initialize();
void displayText(String waterlevel, String Hid);

void setup() {
  initialize();
  
}

void loop() {
  server.handleClient();
  isButtonPressed();
  displayText(requestData,buttonState);
  if(requestData.toInt() <= 50){
    digitalWrite(redLedPin,HIGH);
  }
  else digitalWrite(redLedPin,LOW);
}

void wifiConnect(){
  int wifiTries = 0;
  WiFi.begin(ssid, password);
  
  while ((WiFi.status() != WL_CONNECTED) && wifiTries < wifiTimeoutLimit) {
    wifiTries++;
    Serial.println("Connecting to WiFi...");
    digitalWrite(yelLedPin,HIGH);
    delay(500);
    digitalWrite(yelLedPin,LOW);
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED)
    Serial.println("Connected to WiFi");
  else{ 
    Serial.println("Wifi Connection Timeout, Restarting");
    digitalWrite(yelLedPin,HIGH);
    delay(3000);
    digitalWrite(yelLedPin,LOW);
    ESP.restart();
    

  }
}
void handlePost(){
  if(server.hasArg("plain")){
    requestData = server.arg("plain");
    Serial.println("Received POST data: " + requestData);
    server.send(200,"text/plain","Recieved Data");
  }
}
void handleGET(){
  server.send(200,"text/plain",buttonState);
}
void handleRoot(){
  String message = "This is the Home Water Automation System";
  server.send(200,"text/plain",message);
}
void initialize(){
  Serial.begin(115200);
  pinMode(redLedPin,OUTPUT);
  pinMode(yelLedPin,OUTPUT);
  pinMode(buttonPin,INPUT_PULLUP);
  Serial.println("Pins Configured");
  wifiConnect();
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  server.begin();
  server.on("/water",HTTP_POST,handlePost);
  server.on("/button",HTTP_GET,handleGET);
  server.on("/",HTTP_GET,handleRoot);
}

void displayText(String waterlevel, String Hid){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("Su: %"));
  display.println(waterlevel);
  display.print(F("Hid:"));
  display.println(Hid);
  display.display(); 
}
void isButtonPressed() {
    int reading = digitalRead(buttonPin);
    
    if(reading == 0 && lastButtonPress == 0){
      if(buttonState == "AC"){buttonState = "KAP";}
      else if(buttonState == "KAP"){buttonState = "AC";};
      lastButtonPress = 1;
    }
  
    if(reading == 1){lastButtonPress = 0;}
}
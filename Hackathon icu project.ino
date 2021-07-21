#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoJson.h>

#define BLYNK_PRINT Serial

//Web page from esp8266

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>

<html>

<body>
 
<h2>SoS </h2>


<form action="/action">
<pre>
Patient Name   : <input type="text" name="pname" maxlength="32">
<br>
Patient Age    : <input type="text" name="age" maxlength="2">
<br>
SSID           : <input type="text" name="ssid" maxlength="32">
<br>
Password       : <input type="password" name="passwd" maxlength="32">
<br>
Duty Doctor    : <input type="tel" id="phone" name="dutyphone"
pattern="[0-9]{3}[0-9]{3}[0-9]{4}"
required>
<br>
Concern Doctor : <input type="tel" id="phone" name="conphone"
pattern="[0-9]{3}[0-9]{3}[0-9]{4}"
required>
<br>
By-Stander     : <input type="tel" id="phone" name="byphone"
pattern="[0-9]{3}[0-9]{3}[0-9]{4}"
required>
<br>

<input type="submit" value="Submit">
</pre>

</form>

</body>

</html>
)=====";

const char* ssidap = "wifibutton";
const char* passap = "wifibuttontest";
char auth[] = "YourAuthToken";
char esid[] = "";
char epass[]= "";

const char* avgheart = "";
const char* oxystat = "";

String qsid = "";
String qpass = "";
String pname ="";
String page ="";
String conphone ="";
String dutyphone="";
String byphone="";
bool iskid;

void eepromwrit(void);
void webserver(void);

ESP8266WebServer server(80);

const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
DynamicJsonDocument doc(capacity);

void setup(){
  
  Serial.begin(9600);
  EEPROM.begin(512);

 
//Reading the saved ssid

  String esid = "";
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);

//Reading the saved password

  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 64; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  
  Serial.println("Reading EEPROM Patient name");

  for (int i = 64; i <96; i++)
  {
    pname += char(EEPROM.read(i));
  }
  Serial.println("reading age");
  for (int i = 96;i<= 98;i++){
    page +=char(EEPROM.read(i));
  }
  
  Serial.println("reading duty doc number");
  for (int i = 98;i<= 108;i++){
    dutyphone+=char(EEPROM.read(i));
  }
  Serial.println("reading consern number");
  for (int i = 109;i<= 119;i++){
    conphone+=char(EEPROM.read(i));
  }
  

  WiFi.begin(esid, epass);   // Connecting to the wifi
  
  if(testWifi()){
    Serial.println("sucessfully connected");
      Blynk.begin(auth,esid.c_str(), epass.c_str());
    return;
  }
  else{
    Serial.print("turning on hotspot");
    webserver();
  }
    
  int timeout = 60 * 4; // 10 seconds

// Running the web page until establishing a connection

  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.println(".");
    server.handleClient();

  }

//Wifi details

   if(WiFi.status() != WL_CONNECTED) {
       Serial.println("not connected");
   }
     else{
       Serial.println("connected");
       Serial.print(millis());
       Serial.print(", IP address: "); 
       Serial.println(WiFi.localIP());
    }


}

// Writing data to the esps eeprom

void eepromwrit(){

   
    Serial.println("writing eeprom SSID:");

    for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");

        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        Serial.println("writing eeprom pname:");

        for (int i = 0; i < pname.length(); ++i)
        {
          EEPROM.write(64 + i, pname[i]);
          Serial.print("Wrote: ");
          Serial.println(pname[i]);
        }
        Serial.println("writing age");

Dev G7, [07.03.20 09:36]
for (int i=0;i<page.length();++i){
          EEPROM.write(96+ i,page[i]);
          Serial.println(page[i]);
        }
        Serial.println("writing duty doctor number");

        for (int i=0;i<dutyphone.length();++i){
          EEPROM.write(131+ i,dutyphone[i]);
          Serial.println(dutyphone[i]);
        }
       Serial.println("writing consern doctor number");

        for (int i=0;i<conphone.length();++i){
          EEPROM.write(141+i,conphone[i]);
          Serial.println(conphone[i]);
        }


        EEPROM.commit();
        delay(150);
        ESP.reset();  //resetting the esp after writing the credentials  and triggers the iftt
}



//Turning on the web server

void webserver(){

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap,passap);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("disconnect from the WiFi ");
  delay(10);
  Serial.print("Local IP: ");
  Serial.println(IP);
  
server.on("/",root);
server.on("/action",handlepage);
server.begin();
Serial.println("server started");
  
}


//Checking the wifi connection

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

//Web page handler

void handlepage(){

qsid = server.arg("ssid");
qpass = server.arg("passwd");
pname = server.arg("pname");
page = server.arg("age");
conphone =server.arg("conphone");
dutyphone =server.arg("dutyphone");
byphone = server.arg("byphone");

Serial.println("data from the web");

Serial.println(qsid);
Serial.println(qpass);
Serial.println(pname);
Serial.println(page);
Serial.println(conphone);
Serial.println(dutyphone);
Serial.println(byphone);

eepromwrit();
}


void root(){
    String page = MAIN_page;
    server.send(200,"text/html",page);
  }

  // Nothing on void loop()
  
void loop(){
    Blynk.run();
   if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  //Object of class HTTPClient
    http.begin("http://jsonplaceholder.typicode.com/users/1");
    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {
      // Get the request response payload
      String payload = http.getString();
      // TODO: Parsing
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
         Serial.print(F("deserializeJson() failed: "));
         Serial.println(error.c_str());
         return;
  }
    avgheart = doc["avgheart"].as<char*>();
    oxystat = doc["oxystat"].as<char*>();
    Blynk.virtualWrite(V5,avgheart);
    Blynk.virtualWrite(V6,oxystat);
    if(page.toInt()>18){
        Serial.println("kid");
        iskid=0;
    }else{
        iskid=1;
        Serial.println("adult");
    }
    //calcuheart(avgheart,iskid);
    }
    http.end();   //Close connection
  }
    }

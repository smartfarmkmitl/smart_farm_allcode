

 //แก้ค่า setName เป็น Nextgenfarm และค่า ascii เป็น data

#include "DHT.h"
#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Wire.h>
#include "BH1750.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

const char* ssid     = "Magneto";
const char* password = "mgntkuyy";
const char* host = "192.168.1.5";

#define APPID       "Nextgenfarm"
#define GEARKEY     "PmXeGcsN33p0BBi"
#define GEARSECRET  "sAxh5GX8gNaQTMITJC1MzAmK4"
#define SCOPE       "node11"

#define DHTPIN 3
#define DHTTYPE DHT22

const int moistureAO = A0; //can not modify

WiFiClient client; // กำหนดผู้เชื่อมต่อ IP
AuthClient *authclient;

int moisture = 0;
int counter;
DHT dht(D3, DHTTYPE);


using namespace esl;

BH1750 bh1750(0); // ADDR pin is low (pull-down).
int sda = 4;      // SDA = GPIO-4 (D2 pin)
int scl = 5;      // SCL = GPIO-5 (D1 pin)

int timer = 0;
MicroGear microgear(client);

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) 
{
  Serial.println("Connected to NETPIE...");
  microgear.setName("Nextgenfarm");
}

void setup() {
  Serial.begin(115200);
  Serial.flush();
  dht.begin();
  Wire.begin( sda, scl ); 
  Wire.setClock( 100000 ); // set clock speed
  microgear.on(CONNECTED,onConnected);

  Serial.println("Starting...");

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // แสดงค่า IP ของ

    //microgear.useTLS(false);
    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(GEARKEY, GEARSECRET, SCOPE);
    microgear.connect(APPID);
  }
}

void loop() 
{
   sendtoDB();
}
void sendtoDB(){
      moisture = analogRead( moistureAO );
      
      float vhud = dht.readHumidity();
      float vtmp = dht.readTemperature();
      char data[32]; // กำหนดกล่องเก็บตัวอักษรขนาด 32 อักษร
      int tmp_moi = (int)((moisture - 524) / 5); // make it max 100 min 0
      if (tmp_moi > 100) {
      tmp_moi = 100;
     }

     if (tmp_moi < 0) {
      tmp_moi = 0;
     }

     tmp_moi = 100 - tmp_moi;
    
      if (isnan(vhud) || isnan(vtmp) || vhud > 100 || vtmp > 100){
          vhud = 0.0;
          vtmp = 0.0;
      }

      
      char sbuf[20]; 
      
      bh1750.sendCommand( BH1750::S_H_RESOLUTION_MODE );
      delay( 120 );   // wait at least 120msec
      uint16_t value = bh1750.readData(); 
  
  // Use WiFiClient class to create TCP connections
  Serial.print("connecting to ");
  Serial.println(host);
  
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/upload_file.php";
  url += "?temp=";
  url += vtmp;
  url += "?&humid=";
  url += vhud;
  url += "?&mois=";
  url += tmp_moi;
  url += "?&light=";
  url += value;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

  for(counter = 1; counter <20 ; counter = counter + 1){
        Serial.println(counter);
        sending_microgear();    
    }
               
  int timeout = millis() + 5000;
  
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
    delay(1000);
   
    }
  }


void sending_microgear(){
      if (microgear.connected()) {
      microgear.loop();
      timer=0;
      moisture = analogRead( moistureAO );
      
      float vhud = dht.readHumidity();
      float vtmp = dht.readTemperature();
      char data[32]; // กำหนดกล่องเก็บตัวอักษรขนาด 32 อักษร
      int tmp_moi = (int)((moisture - 524) / 5); // make it max 100 min 0
      if (tmp_moi > 100) {
      tmp_moi = 100;
     }

     if (tmp_moi < 0) {
      tmp_moi = 0;
     }

     tmp_moi = 100 - tmp_moi;
    
      if (isnan(vhud) || isnan(vtmp) || vhud > 100 || vtmp > 100){
          vhud = 0.0;
          vtmp = 0.0;
      }

      
      char sbuf[20]; 
      
      bh1750.sendCommand( BH1750::S_H_RESOLUTION_MODE );
      delay( 120 );   // wait at least 120msec
      uint16_t value = bh1750.readData(); 
      sprintf(data,"%d,%d,%d,%d,Nextgenfarm", (int)vtmp,(int)vhud,tmp_moi,value); // เก็บค่าต่างๆเข้าไปในกล่อง data ที่สร้างขึ้น
      Serial.println(data);
      sprintf( sbuf, "Light:%5u Lx", value );
      Serial.println(vtmp);
      Serial.println(vhud);
      Serial.println(tmp_moi);
      Serial.println( sbuf );
      //microgear.chat("manual_control",data);
      microgear.publish("/node11",data);
      //microgear.publish("/node11/temp",data);
      delay(2000);
  
  delay(1000);
 }
}


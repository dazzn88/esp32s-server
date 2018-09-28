#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
/*#define SSID*/
#define ThingSpeak

String APIKEY  = "M6WBABVJPEWG3FYQ";

char pin = 4,i = 0,g;
unsigned long duration,starttime;
unsigned long sampletime_ms =10000; //秒回傳一次量測值
unsigned long lowpulseoccupancy = 0;
const char* ssid;
const char* password;
float ratio = 0;
float concentration = 0;
float pm25val = 0;
float d=0,a=0,b=0,h,j=0;
float pm25coef = 0.00207916725464941; 
String GET = "GET /update?key=M6WBABVJPEWG3FYQ";

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  const char* ssid     = "Dazzn";
  const char* password = "18185566";
  Serial.begin(115200);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  //Serial.println("Characteristic defined! Now you can read it in your phone!");
  
  //wifi connect
  Serial.println( "WIFI Ready!" );
 /* Serial.print("Connect to ");
    Serial.println( SSID );*/
    WiFi.begin(ssid, password );
     while( WiFi.status() != WL_CONNECTED )
    {
        delay(500);
        Serial.print( "." );
    }
    Serial.println( "WiFi connected" );
   /* Serial.println( "IP address: " );
    Serial.println( WiFi.localIP() );
    Serial.println( "" ); */
  pinMode(4,INPUT);
  starttime = millis();
}

void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using PPD42 spec sheet curve
    
    // PM2.5 calc
    pm25val = pm25coef * concentration * 10; // 10 to transform 0.01 cf to 0.1 ft
    
    lowpulseoccupancy = 0;
    starttime = millis();
    
    g=1;
  }
  if(g==1){
  a=a+concentration;
  h=h+pm25val;
  i=i+1;
  if(i==5)
  {
   b= a/5;
   j=h/5;
   i=0;
   Serial.print(b);
   Serial.println("  pcs/0.01cf");
   Serial.print(j);
   Serial.println("  ug/m3");
   updateThingSpeak();
   b=0;
   a=0;
   h=0;
   j=0;
   }
    g=0;
  }
}

void updateThingSpeak(){
 WiFiClient client;
 if( !client.connect( "api.thingspeak.com", 80 ) )
    {
        Serial.println( "connection failed" );
        return;
    }
    else
    {
        String postStr = APIKEY + "&field1=" + String(j) + "&field2=" + String(b) + "\r\n\r\n";
        client.print( "POST /update HTTP/1.1\n" );
        client.print( "Host: api.thingspeak.com\n" );
        client.print( "Connection: close\n" );
        client.print( "X-THINGSPEAKAPIKEY: "+APIKEY+"\n" );
        client.print( "Content-Type: application/x-www-form-urlencoded\n" );
        client.print( "Content-Length: " );
        client.print( postStr.length() );
        client.print( "\n\n" );
        client.print( postStr );
        client.stop();
    }
}

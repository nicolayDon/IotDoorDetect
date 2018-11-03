#include<Arduino.h>

#ifdef TARGET_ESP32
    #include<HTTPClient.h>
    #include<WebServer.h>
    #include<WiFi.h>
    #include <WiFiClientSecure.h>
    #include <ArduinoJson.h>
    #include <FirebaseESP32.h>
    #define D1 23
    #define D2 22
    #define D3 21
    #define D4 19
    #define D5 18
    #define D6 21
    #define D7 5
#endif

#ifdef TARGET_ESP8266
    #include<ESP8266HTTPClient.h>
    #include<ESP8266WebServer.h>
    #include<ESP8266WiFi.h>
    #include <WiFiClientSecure.h>
    #include<FirebaseArduino.h>
#endif

#include <Adafruit_NeoPixel.h>



#include <vector>  
using namespace std;
struct Oblects {
	char ObjName[14];
	short PinOut1,PinOut2;
    int CurStat;
	// конструктор объекта
	Oblects(const char* ObjName, short PinOut1, short PinOut2=0, short CurStat=0) : PinOut1(PinOut1) , PinOut2(PinOut2) , CurStat(CurStat) { 	
		strncpy(this->ObjName, ObjName, sizeof(this->ObjName));
	}	
};
vector<Oblects> List_Obj;
// Set these to run example.

#define FIREBASE_HOST   "xxxxxxxxxxxxxxxxxxxxxxxx.firebaseio.com"
#define FIREBASE_AUTH   "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"                          	
 
#define WIFI_SSID       "esl"
#define WIFI_PASSWORD   "xxxxxxxxxxxxxxxxxxxxxx"


WiFiClientSecure client;

const char* ReqestUrl = "xxxxxxxxxxxxxxx.execute-api.us-east-2.amazonaws.com"; // Server URL

// pin for led indicator
#define PIN_LIGHT           D1

#define LeftTop             D2
#define LeftFoot            D3
#define RightTop            D4
#define RightFoot           D5
#define MotionPin           D6
#define SoundPin            D7

// How many NeoPixels are attached to the Arduino?
int numPixel = 8;
Adafruit_NeoPixel *pixels;

//conncet to lambda amazonaws
void SendRequst(String eventName, String eventData) {
  Serial.println("\nStarting connection to server...");
  if (!client.connect(ReqestUrl,443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    client.print(String("GET /default/Esp_Rec?address="+WiFi.macAddress()+"&eventName="+eventName+"&eventData="+eventData)  + " HTTP/1.1\r\n" +
               "Host: "+ReqestUrl+ "\r\n" +
               "User-Agent: ESp32\r\n" +
               "Connection: close\r\n\r\n"
              );     

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        
        Serial.println("headers received");
        break;
      }
    }
    while (client.available()) {
      Serial.println(client.readString());
      Serial.write("Ok");
    }
    client.stop();
  }
}
int n = 0;
bool fleg= false; 
//int Input1=0,Input2=0,Input3=0,Input4=0,Input5=0;
void set_Color(int light, int color)
{
     switch (color){
        case 0:
            pixels->setPixelColor(light, pixels->Color(0,25,0));           
            break;
        case 1:
            pixels->setPixelColor(light, pixels->Color(25,25,25));            
            break;
        case 2:
            pixels->setPixelColor(light, pixels->Color(25,0,0));           
            break;           
        default:
         break;
            // code to be executed if n doesn't match any constant
        }
}


void setup() {

    
    Serial.begin(9600);

    pinMode(RightTop,INPUT);
    pinMode(RightFoot,INPUT);
    pinMode(LeftTop,INPUT);
    pinMode(LeftFoot,INPUT);

    pinMode(PIN_LIGHT,OUTPUT);
    pinMode(SoundPin,OUTPUT);
    pinMode(MotionPin,INPUT);
    digitalWrite(SoundPin,true);
	
	// inint several sensor(gercon)
	List_Obj.push_back(Oblects("loggia1",       -1, -1));   // RightTop, RightFoot));//
	List_Obj.push_back(Oblects("loggia2",       -1, -1));   // LeftTop, LeftFoot));//
	List_Obj.push_back(Oblects("balcony1",      -1, -1));   // RightTop,   RightFoot));//
    List_Obj.push_back(Oblects("balcony2",       -1, -1));   // LeftTop,    LeftFoot));// 
    List_Obj.push_back(Oblects("smalRoom",      -1, -1));   // LeftTop, LeftFoot));//
    List_Obj.push_back(Oblects("kitchenWind",   LeftTop,    LeftFoot));//-1, -1));// -1, -1));
    List_Obj.push_back(Oblects("kitchenDoor",   RightTop, -1));// -1, -1));
    
    pixels = new Adafruit_NeoPixel(numPixel,   PIN_LIGHT, 0x52);
    pixels->begin(); // This initializes the NeoPixel library.
    WiFi.mode( WIFI_STA );

    for (int i=0;i < List_Obj.size(); i++)
        set_Color(i+1,List_Obj[i].CurStat);  
    
    // connect to wifi.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  


 
}
bool sound=false;


void loop() {

    if  (WiFi.status() != WL_CONNECTED) {
    pixels->setPixelColor(0, pixels->Color(255,0,0)); 
    }
    else {
        WiFiClient client;
        if (client.connect("google.com",80)){      
        pixels->setPixelColor(0, pixels->Color(0,125,0)); 
        }
        else{
            pixels->setPixelColor(0, pixels->Color(125,125,0)); 
            client.stop();
            return ;
        }
        client.stop();
    }
    if (Firebase.failed()) {
        Serial.println("streaming error");
        Serial.println(Firebase.error());
        delay(500);
         return ;

    }

    for (int i = 0; i < List_Obj.size(); i++){
      
       if (List_Obj[i].PinOut1==-1 &&List_Obj[i].PinOut2==-1){           
            List_Obj[i].CurStat = Firebase.getInt(String(List_Obj[i].ObjName));
            set_Color(i+1,List_Obj[i].CurStat);                        
       }else if (List_Obj[i].PinOut1!=-1 &&List_Obj[i].PinOut2==-1) {                  
            if(List_Obj[i].CurStat!=(digitalRead(List_Obj[i].PinOut1)==1) ? 2 : 0){
                List_Obj[i].CurStat=(digitalRead(List_Obj[i].PinOut1)==1) ? 2 : 0;
                Firebase.setInt(List_Obj[i].ObjName, List_Obj[i].CurStat);
                set_Color(i+1,List_Obj[i].CurStat);                  
            } 
       }else if (List_Obj[i].PinOut1!=-1 &&List_Obj[i].PinOut2!=-1) {
            if(List_Obj[i].CurStat!=digitalRead(List_Obj[i].PinOut1)+digitalRead(List_Obj[i].PinOut2)){
                List_Obj[i].CurStat=digitalRead(List_Obj[i].PinOut1)+digitalRead(List_Obj[i].PinOut2);
                Firebase.setInt(List_Obj[i].ObjName,List_Obj[i].CurStat);
                set_Color(i+1,List_Obj[i].CurStat);                   
            } 
       }
        
    } 

  //  digitalWrite(SoundPin,sound);
    if (sound){sound=false;} else sound=true;
Serial.println("================+");
 
    pixels->show();
    delay(500);
}

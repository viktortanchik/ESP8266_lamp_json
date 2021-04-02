#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
//WiFiServer server(8888);
/////////////////////////////////////
#include <Adafruit_NeoPixel.h>
#define PIN 5
//#define PIN 2
#define count_led 14 // количество светодиодов 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, PIN, NEO_GRB + NEO_KHZ800); //first number change does distance between colors

/////////////////////////////////////
WiFiClient client;
//WiFiServer server(8000);
String httpData;
int id;
String Status ="";
int Mode;
const char url[] = "http://ce05390-django.tw1.ru/api/v1/lamps/lamps/detail/4/?format=json";

//const char url[] = "http://192.168.1.5:8000/api/v1/lamps/lamps/detail/1/?format=json";
//const char url[] = "http://192.168.0.135:8000/api/v1/lamps/lamps/detail/1/?format=json";

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(2, OUTPUT);
  // put your setup code here, to run once:
Serial.begin(115200);
    
     WiFi.begin("Kyivstar_DA3E", "22546852");
     //WiFi.begin("electrowerty", "wertynet");
     while (WiFi.status() != WL_CONNECTED) 
     {digitalWrite(2, LOW);
       delay(100);
       Serial.print(".");
       digitalWrite(2, HIGH);
     } 
     Serial.println("connectedd");
     IPAddress myAddress = WiFi.localIP();
     Serial.println(myAddress);
      
}

void loop() {
 
HTTPClient client;
  bool find = false;
  //client.setTimeout(1000);
  Serial.print("Connecting ");
  client.begin(url);
  int httpCode = client.GET();
  if (httpCode > 0) {
    Serial.printf("successfully, code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      httpData = client.getString();
      Serial.println(httpData);
      }
      else Serial.println("Failed, json string is not found");
        client.end();
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(httpData);     // скармиваем String
  if (!root.success()) {
    Serial.println("parseObject() failed");
  }
  id = root["id"];
  Serial.print("id:");
  Serial.println(id);
  Status = root["status"].as<String>();
  Serial.print("status:");
  Serial.println(Status);
    Mode = root["mode"];
  Serial.print("mode:");
  Serial.println(Mode); 
  if (Mode ==1){
        digitalWrite(2, LOW);
      strip.clear();
        strip.show();
        delay(1000);}
  if (Mode ==2){
    rainbowCycle(5);
  }
  if (Mode ==3){
    whiteColor(100);
}
  if (Mode ==4){
    FlashWhiteColor(100);
}
  if (Mode ==5){
    randColorPixel(100);
}
if (Mode ==6){
  randColor(100);
}
   httpData = "";
}  
//////////////////////////////////////////////////////////

void FlashWhiteColor(uint8_t wait) {

  for (int i = 0; i <= count_led; i++)
  {
    int randNumber = random(1, count_led);
    strip.setPixelColor(randNumber, strip.Color(255, 255, 255));
    strip.show();
    delay(wait);
    strip.setPixelColor(randNumber, strip.Color(255, 200, 0));
    strip.show();
  }
}
///////////////////////////////////////////////////////
void randColor(uint8_t wait)
{
 int randNumber = random(1, count_led);
    int randColorR = random(1, 255);
    int randColorG = random(1, 255);
    int randColorB = random(1, 255);
     for (int i = 0;i <= count_led; i++)
  {     
    strip.setPixelColor(i, strip.Color(randColorR, randColorG, randColorB));
    strip.show();
    delay(wait);  
  }
  }
  ////////////////////////////////////////////////////////////
void randColorPixel(uint8_t wait)
{
     for (int i = 0;i <= count_led; i++)
  {     
    int randNumber = random(1, 11);
    int randColorR = random(1, 255);
    int randColorG = random(1, 255);
    int randColorB = random(1, 255);
    strip.setPixelColor(randNumber, strip.Color(randColorR, randColorG, randColorB));
    strip.show();
    delay(wait);
    strip.setPixelColor(randNumber, strip.Color(0, 0, 0));
    //strip.setPixelColor(randNumber, strip.Color(r, g, b));
    strip.show();
  }
  }
////////////////////////////////////////////////////////////////////////
void whiteColor(uint8_t wait) {
  for(uint16_t t=0; t<count_led; t++){
    strip.setPixelColor(t, strip.Color(255, 255, 255));
    strip.show();
    delay(wait);
  }
}
///////////////////////////////////////////////////
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
////////////////////////////////////////////////////  

// Новая библиотека для JSON
// Добавлена световая индикация при настройке.
// Попытка переделать алгоритм програмы что бы JSON сохранялся в ЕEPROM.
// и если новый JSON отличается от старого то мы меняем режим работы.

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
///////////////////////////////////////////////
#include <Adafruit_NeoPixel.h>
//#define PIN 5 // для Большой лампы 12
#define PIN 2  // для маленькой лампы 01
#define count_led 35 // количество светодиодов 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, PIN, NEO_GRB + NEO_KHZ800); //first number change does distance between colors
//int SW = 4; // Большая лампа
int SW = 0;// МаленькаЯ Лампа.
int sw_u = 4;
int pw_u = 5;
int flagU;
///////////////////////////////////////////////
String httpData;
int id;
String Status = "";
int StatusJ;
int Mode;
//const char url[] = "http://192.168.1.6:8000/api/v1/lamps/lamps/detail/10/?format=json";
const char url[] = "http://ce05390-django.tw1.ru/api/v1/lamps/lamps/detail/21/?format=json";

///////////////////////////////////////////////

boolean flag = false;
int flagErorr = 0;
///////////////////////////////////////////////
int red = 0;
int green = 0;
int blue = 0;
////////////////////////////////////////////////
String str = "";

boolean conf = false;

String html_header = "<html>\
 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
 <head>\
   <title>ESP8266 Settings</title>\
   <style>\
     body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
   </style>\
 </head>";

ESP8266WebServer server(80);

void setup(void)
{
  flagErorr = 0;

  //////////////////////////////////////
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //////////////////////////////////////
  byte len_ssid, len_pass;

  delay(3000);
  Serial.begin(115200);
  Serial.println();

  EEPROM.begin(98);

  len_ssid = EEPROM.read(96);
  len_pass = EEPROM.read(97);
  if (len_pass > 64) len_pass = 0;

  //pinMode(2, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  pinMode(sw_u, OUTPUT);
  pinMode(pw_u, OUTPUT);
  //digitalWrite(sw_u, HIGH);
  digitalWrite(pw_u, LOW);

  if ((len_ssid < 33) && (len_ssid != 0)) {
    // Режим STATION
    WiFiClient client;
    WiFi.mode( WIFI_STA);
    unsigned char* buf_ssid = new unsigned char[32];
    unsigned char* buf_pass = new unsigned char[64];
    for (byte i = 0; i < len_ssid; i++) buf_ssid[i] = char(EEPROM.read(i));
    buf_ssid[len_ssid] = '\x0';
    const char *ssid  = (const char*)buf_ssid;
    for (byte i = 0; i < len_pass; i++) buf_pass[i] = char(EEPROM.read(i + 32));
    const char *pass  = (const char*)buf_pass;
    buf_pass[len_pass] = '\x0';
    delay(2000);
    Serial.print("SSID: ");
    Serial.print(ssid);
    Serial.print("   ");
    Serial.print("Password: ");
    Serial.println(pass);

    WiFi.begin(ssid, pass);
    // Wait for connection
    int t = 0;
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 250 );
      Serial.print ( "." );

      strip.setPixelColor(t, strip.Color(0, 255, 0));
      strip.show();
      t++;
      if ((digitalRead(SW) == LOW) && !conf) {
        EEPROM.write(96, 255);
        EEPROM.commit();
        EEPROM.end();
        conf = true;
        flag = false;
        Serial.println("Please reboot module for coniguration --gpio5--");
        Serial.println(conf);
        for (uint16_t t = 0; t < count_led; t++) {
          strip.setPixelColor(t, strip.Color(0, 0, 50));
          strip.show();
          delay(50);
        }
        //Serial.println(f_state);
      }
    }
    Serial.println();
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    flag = true;
  }
  else // Режим SoftAP
  {
    for (uint16_t t = 0; t < count_led; t++) {
      strip.setPixelColor(t, strip.Color(255, 0, 0));
      strip.show();
      delay(50);
    }
    const char *ssid_ap = "LAMP_001_Beta";
    WiFi.mode(WIFI_AP);
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid_ap);
    delay(2000);
    Serial.println("done");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleRoot);
    server.on("/ok", handleOk);
    server.begin();
    Serial.println("HTTP server started");
  }
  strip.clear();
  strip.show();
}
void loop() {
  server.handleClient();

  // Перевод модуля в режим конфигурации путем замыкания GPIO5 на массу

  if ((digitalRead(SW) == LOW) && !conf) {
    EEPROM.write(96, 255);
    EEPROM.commit();
    EEPROM.end();
    conf = true;
    flag = false;
    Serial.println("Please reboot module for coniguration --gpio5--");
    Serial.println(conf);
    //Serial.println(f_state);
    for (uint16_t t = 0; t < count_led; t++) {
      strip.setPixelColor(t, strip.Color(0, 0, 255));
      strip.show();
      delay(50);
    }
  }
  else if (flag == true)
  {
    MAIN();
  }

}
//////////////////////////////////////////////////////////
void(* resetFunc) (void) = 0;
//////////////////////////////////////////////////////////////
void MAIN()
{

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
    //resetFunc();
    client.end();

  }
  // DynamicJsonBuffer jsonBuffer;
  //  StaticJsonBuffer<200> jsonBuffer;
  //DynamicJsonDocument jsonBuffer(1024);
  //JsonObject& root = jsonBuffer.parseObject(response);
  //  JsonObject root = jsonBuffer.parseObject(httpData);
  //deserializeJson(jsonBuffer, httpData);
  //DeserializationError error = deserializeJson(jsonBuffer, httpData);

  // скармиваем String
  //  if (!root.success()) {
  //    Serial.println("parseObject() failed");
  //    resetFunc();
  //    //continue;
  //  }
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, httpData);
  if (error)
    return;
  //int value = doc["value"];


  id = doc["id"];
  Serial.print("id:");
  Serial.println(id);
  ////////////////////////////////// УВЛАЖНИТЕЛЬ//////////////////////////////////
  // Status = doc["status"].as<String>();
  StatusJ = doc["status"];
  Serial.print("status:");
  Serial.println(StatusJ);

  Mode = doc["mode"];
  Serial.print("mode:");
  Serial.println(Mode);
  if (StatusJ == 1)
  {
    digitalWrite(pw_u, HIGH);
    

    digitalWrite(sw_u, LOW);
    flagU = 0;
  }
  if (StatusJ == 0)
  {
    if (flagU == 0)
    {
      digitalWrite(sw_u, HIGH);
      digitalWrite(pw_u, LOW);

      delay(500);
      digitalWrite(sw_u, LOW);
      delay(500);

      digitalWrite(sw_u, HIGH);

      flagU = 1;
    }

  }
  ////////////////////////////////////////////
  red = doc["red"];
  Serial.print("red:");
  Serial.println(red);
  green = doc["green"];
  Serial.print("green:");
  Serial.println(green);
  blue = doc["blue"];
  Serial.print("blueen:");
  Serial.println(blue);

  int ModeE = EEPROM.read(10);

  if (Mode != ModeE)
  {
    Serial.println("Данные изменены Перезапись.");
    EEPROM.write(10, Mode);
    EEPROM.write(20, red);
    EEPROM.write(30, green);
    EEPROM.write(40, blue);

  }

  ////////////////////////////////////////////
  Serial.print("EEPROM:");
  Serial.println(ModeE);

  if (ModeE == 1) {
    digitalWrite(2, LOW);
    strip.clear();
    strip.show();
    delay(1000);
  }
  if (ModeE == 2) {
    rainbowCycle(5);
  }
  if (ModeE == 3) {
    whiteColor(100);
  }
  if (ModeE == 4) {
    FlashWhiteColor(100);
  }
  if (ModeE == 5) {
    randColorPixel(100);
  }
  if (ModeE == 6) {
    randColor(100);
  }
  if (Mode == 7) {
    int  redE = EEPROM.read(20);
    int  greenE = EEPROM.read(30);
    int  blueE = EEPROM.read(40);

    for (int i = 0; i <= count_led; i++)
    {
      strip.setPixelColor(i, strip.Color(red, green, blue));
      strip.show();
      delay(10);

    }
  }

  httpData = "";
}
////////////////////////////////////////////////////////////
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
  for (int i = 0; i <= count_led; i++)
  {
    strip.setPixelColor(i, strip.Color(randColorR, randColorG, randColorB));
    strip.show();
    delay(wait);
  }
}
////////////////////////////////////////////////////////////
void randColorPixel(uint8_t wait)
{
  for (int i = 0; i <= count_led; i++)
  {
    int randNumber = random(0, count_led);
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
  for (uint16_t t = 0; t < count_led; t++) {
    strip.setPixelColor(t, strip.Color(255, 255, 255));
    strip.show();
    delay(wait);
  }
}
///////////////////////////////////////////////////
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
////////////////////////////////////////////////////
void handleRoot() {
  String str = "";
  str += html_header;
  str += "<body>\
   <form method=\"POST\" action=\"ok\">\
     <input name=\"ssid\"> WIFI Net</br>\
     <input name=\"pswd\"> Password</br></br>\
     <input type=SUBMIT value=\"Save settings\">\
   </form>\
 </body>\
</html>";
  server.send ( 200, "text/html", str );
}

void handleOk() {
  String ssid_ap;
  String pass_ap;
  unsigned char* buf = new unsigned char[64];

  String str = "";
  str += html_header;
  str += "<body>";

  EEPROM.begin(98);

  ssid_ap = server.arg(0);
  pass_ap = server.arg(1);

  if (ssid_ap != "") {
    EEPROM.write(96, ssid_ap.length());
    EEPROM.write(97, pass_ap.length());
    ssid_ap.getBytes(buf, ssid_ap.length() + 1);
    for (byte i = 0; i < ssid_ap.length(); i++)
      EEPROM.write(i, buf[i]);

    pass_ap.getBytes(buf, pass_ap.length() + 1);
    for (byte i = 0; i < pass_ap.length(); i++)
      EEPROM.write(i + 32, buf[i]);

    EEPROM.commit();
    EEPROM.end();

    str += "Конфигурация сохранена во FLASH</br>\
   Изменения применяются после перезагрузки</p></br></br>\
   <a href=\"/\">Return</a> to settings page</br>";
    for (uint16_t t = 0; t < count_led; t++) {
      strip.setPixelColor(t, strip.Color(255, 0, 0));
      strip.show();
      delay(50);
    }
  }

  else {
    str += "No WIFI Net</br>\
   <a href=\"/\">Return</a> to settings page</br>";
  }
  str += "</body></html>";
  server.send ( 200, "text/html", str );
}

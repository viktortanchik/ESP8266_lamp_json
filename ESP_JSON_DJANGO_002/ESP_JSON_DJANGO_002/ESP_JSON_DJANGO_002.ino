#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
//WiFiServer server(8888);

WiFiClient client;
//WiFiServer server(8000);
String httpData;
int id;
String Status ="";
int Mode;
const char url[] = "http://192.168.1.3:8000/api/v1/lamps/lamps/detail/1/?format=json";
void setup() {
  pinMode(2, OUTPUT);
  // put your setup code here, to run once:
Serial.begin(115200);
    
     WiFi.begin("Kyivstar_DA3E", "22546852");
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
  delay(1000);
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
       delay(1000);
  if (Mode ==1){digitalWrite(2, LOW);}
  if (Mode ==2){digitalWrite(2, HIGH);}
   httpData = "";

  
}    

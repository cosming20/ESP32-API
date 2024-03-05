#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLuetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

#define btcServerName "ESP_BTC"
BluetoothSerial SerialBT;
bool w_connected = false;
bool b_connected = false;
DynamicJsonDocument jsondoc(1024);
const char* TEAM_ID = "A28";
#define CONNECT_TIMEOUT 15000


void deviceConnected(esp_spp_cb_event_t event,esp_spp_cb_param_t *param) 
{
    if(event == ESP_SPP_SRV_OPEN_EVT){
      Serial.println("Device Connected");
      b_connected = true;
    }
    if(event == ESP_SPP_CLOSE_EVT ){
      Serial.println("Device disconnected");
      b_connected = false;
    }
}

void actions (){

    if(b_connected)
{
    while (SerialBT.available() == 0);
    String payload = SerialBT.readStringUntil('\n');
    DeserializationError error = deserializeJson(jsondoc, payload);
    if (error) 
    {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
    }  
    else 
    {
        String action = jsondoc["action"].as<String>();
        String teamId = jsondoc["teamId"].as<String>();
        Serial.print("action: ");
        Serial.println(action);
        Serial.print("teamId: ");
        Serial.println(teamId );
         if(action=="getNetworks")
        {
            int numNetworks = WiFi.scanNetworks();
            for (int i = 0; i < numNetworks; i++) 
            {
                DynamicJsonDocument jsonDoc(1024);
                jsonDoc["ssid"] = WiFi.SSID(i);
                jsonDoc["strength"] = WiFi.RSSI(i);
                jsonDoc["encryption"] = WiFi.encryptionType(i);
                jsonDoc["teamId"] = teamId;
                String jsonString;
                serializeJson(jsonDoc, jsonString);
                SerialBT.println(jsonString);
                delay(10);
            }
        }
        else if (action == "connect"){

            String received_ssid = jsondoc["ssid"];
            String received_password = jsondoc["password"];
            String ssid = received_ssid;
            String password = received_password;
            WiFi.begin(ssid.c_str(), password.c_str());
            Serial.println("Connecting");
            long connectStart = millis();
            while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(500);
                if (millis() - connectStart > CONNECT_TIMEOUT) {
                     break;
                }

            }
            Serial.println("");
    bool connected_WiFi = false;
    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection failed");
      } else {
          Serial.print("Connected to network: ");
          Serial.println(WiFi.SSID());
          Serial.print("Local IP address: ");
          Serial.println(WiFi.localIP());
          connected_WiFi = true;
      }
    DynamicJsonDocument doc1(200);
    doc1["ssid"] = ssid;
    doc1["connected"] = connected_WiFi;
    doc1["teamId"] = teamId;
    String output1;
    serializeJson(doc1, output1);
    SerialBT.println(output1);
    }
    else if (action == "getData"){

    String url = "http://proiectia.bogdanflorea.ro/api/bobs-burgers/characters";
    HTTPClient http;
    http.begin(url);
 
    int httpCode = http.GET();
 
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();
 
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);
 
    JsonArray jsonArray = doc.as<JsonArray>();
 
    for (JsonObject obj : jsonArray) {
      if (obj.containsKey("imageUrl")) {
      obj["image"] = obj["imageUrl"];
      obj.remove("imageUrl");
      }
      obj["teamId"] = TEAM_ID;
      DynamicJsonDocument jsonDoc(4096);
      jsonDoc.set(obj);
      String jsonData;
      serializeJson(jsonDoc, jsonData);
      delay(10);
      SerialBT.println(jsonData);
      Serial.println(jsonData);
      }
    }
    }
    else if (action == "getDetails"){
        String id = jsondoc["id"];
        Serial.println(id.c_str());
        String url = "http://proiectia.bogdanflorea.ro/api/bobs-burgers/character?id=";
        String url_final = String(url.c_str()) + String(id.c_str());
        Serial.println(url_final.c_str());
        HTTPClient http;
        Serial.println(url_final);
        http.begin(url_final);
 
        int httpCode = http.GET();
 
        if(httpCode == HTTP_CODE_OK){
          String payload = http.getString();
          http.end();
          DynamicJsonDocument doc3(2048);
          deserializeJson(doc3, payload);
          String name = doc3["name"];
          String gender = doc3["gender"].as<String>();
          String hairColor = doc3["hairColor"].as<String>();
          String occupation = doc3["occupation"].as<String>();
          String firstEpisode = doc3["firstEpisode"].as<String>();
          String voicedBy = doc3["voicedBy"].as<String>();
          String relatives = "";
          JsonArray relativesArray = doc3["relatives"].as<JsonArray>();
          for (const auto& relative : relativesArray) {
            String relativeName = relative["name"].as<String>();
            String relativeRelationship = relative["relationship"].as<String>();
            String wikiUrl = relative["wikiUrl"].as<String>();
            String rurl = relative["url"].as<String>();
            if (relativeName)
              relatives +=  "Relative Name: " + relativeName; 
            if (wikiUrl)
              relatives += "\nWiki Url: " + wikiUrl; 
            if (rurl)  
              relatives += "\nUrl: " + rurl; 
            if (relativeRelationship)
              relatives += "\nRelative Relationship: " + relativeRelationship;
            if (relatives)
              relatives += "\n";
          }

          
          String type = doc3["type"].as<String>();
          String imageUrl = doc3["imageUrl"].as<String>();
          String combinedstring;
          if (relatives != "")
             combinedstring = "Gender: " + gender + "\n" + "Hair Color: " + hairColor + "\n" + "Occupation: " + occupation + "\n" + "First Episode: " + firstEpisode + "\n" + "Voiced By: " + voicedBy + "\n" + "Relative:\n" + relatives + "Type: " + type;
          else 
             combinedstring = "Gender: " + gender + "\n" + "Hair Color: " + hairColor + "\n" + "Occupation: " + occupation + "\n" + "First Episode: " + firstEpisode + "\n" + "Voiced By: " + voicedBy + "\n" + "Type: " + type;
          Serial.println(name);
          Serial.println(gender);
          Serial.println(hairColor);
          Serial.println(occupation);
          Serial.println(firstEpisode);
          Serial.println(voicedBy);
          Serial.println(type);
          Serial.println(imageUrl);
          DynamicJsonDocument jdoc3(2048);
          jdoc3["id"] = id;
          jdoc3["name"] = name;
          jdoc3["image"] = imageUrl;
          jdoc3["description"] = combinedstring;
          jdoc3["teamId"] = TEAM_ID;
          String jsonResponse3;
          serializeJson(jdoc3, jsonResponse3);

          SerialBT.println(jsonResponse3); 
    } 
    delay(10);
  }
          
}
}
}


void setup() {
Serial.begin(115200);
WiFi.mode(WIFI_STA);
SerialBT.begin(btcServerName);
Serial.println("Device ready for pairing!");
SerialBT.register_callback(deviceConnected);
const long connection_timeout = 20000;
long startConnection = 0;
startConnection = millis();
while(b_connected==0){
    Serial.print(".");
    delay(500);
    if (millis() - startConnection > connection_timeout) {
        break;
    }
}
}
void loop() {
    
actions();

}
//libraries necessaires
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <AsyncElegantOTA.h>
//partie de declaration des variables
#define SEALEVELPRESSURE_HPA (1013.25)

// Declaration object 
Adafruit_BME280 bme; // I2C

//pour la partie du code de SPIFFS pour qu'on gere la memoire flash, et pour qu'on ajoute des fichiers du html, css et js
//source: https://randomnerdtutorials.com/esp32-ota-over-the-air-vs-code/

//const char* ssid = "UNIFI_IDO1";
//const char* password = "42Bidules!";
const char* ssid = "Ste-adele";
const char* password = "allo1234";
const char* DATAFILE = "/Data.json";


AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  //partie bme
  Serial.println(F("BME280 test"));
  //cette prtie pour le OTA est copie depuis https://randomnerdtutorials.com/esp32-web-server-gauges/
  //configuration de wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {   // Wait for connection
    delay(500);
    Serial.print("-");
  }
  Serial.println(WiFi.localIP());



  
// Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    //si ya une erreur ca affiche ce message
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }




  if(!SPIFFS.exists(DATAFILE)) {
    //si le fichier n'existe pas, ca va creer le fichier json, l'ouvrir avec le parametre write, faire la serialization et le fermer
    DynamicJsonDocument doc(1024);
    File file = SPIFFS.open(DATAFILE, FILE_WRITE);
    serializeJson(doc, file);
    file.close();
    // la serialization est de prendre l'objet doc et le serialize sous format json et l'ecrit sur le fichier file.
  }
  //lance la page html dans le serveur apres qu'il la recupere depuis la memoire avec le spiffs
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("main.html");
//AsyncElegantOTA.begin(&server);    // Start serveur ElegantOTA
server.begin();


}
void loop() {
  // Read BME280 sensor data
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.readHumidity();

  // Open JSON data file for reading and writing
  File dataFile = SPIFFS.open(DATAFILE, "r+");
  if (!dataFile) {
    Serial.println("Error opening data file");
    return;
  }

  // Deserialize JSON data from file
 DynamicJsonDocument doc(9000);
   deserializeJson(doc, dataFile);
   /*if (deserializeJson(doc, dataFile) != DeserializationError::Ok) {
    Serial.println("Error deserializing JSON data");
    dataFile.close();
    return;
  }*/
 

  // Update JSON object with sensor data
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  doc["altitude"] = altitude;
 dataFile.seek(0);
  // Serialize JSON object to file
  /*if (serializeJson(doc, dataFile) == 0) {
    Serial.println("Error serializing JSON data");
    dataFile.close();
    return;
  }*/
    serializeJson(doc, dataFile);
  dataFile.close();

  delay(5000);
}

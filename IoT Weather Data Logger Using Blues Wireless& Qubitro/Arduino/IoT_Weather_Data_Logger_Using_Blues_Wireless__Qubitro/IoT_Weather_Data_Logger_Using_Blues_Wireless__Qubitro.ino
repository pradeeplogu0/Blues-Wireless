
#include <Notecard.h>
#include "Air_Quality_Sensor.h"
#include <DHT.h>

AirQualitySensor sensor(A1);
#define DHTPIN 13               

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define txRxPinsSerial Serial1
#define productUID "com.gmail.pradeeplogu26:env_data_logger"
Notecard notecard;

String Stage;

void setup(void) {

  notecard.begin(txRxPinsSerial, 9600);

  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  notecard.sendRequest(req);

  delay(1000);

  dht.begin(); // Init DHT11
  Serial.println("Waiting sensor to init...");
  delay(10000);
}

void loop(void) {

  int quality = sensor.slope();
  Serial.print("AQ Sensor value    : ");
  Serial.println(sensor.getValue());

  Serial.print("Pollution Level    : ");
  if (quality == AirQualitySensor::FORCE_SIGNAL) {
    String Stage = "ERROR!";
    Serial.println(Stage);
  } else if (quality == AirQualitySensor::HIGH_POLLUTION) {
    String Stage = "High pollution!";
    Serial.println(Stage);
  } else if (quality == AirQualitySensor::LOW_POLLUTION) {
    String Stage = "Low pollution!";
    Serial.println(Stage);
  } else if (quality == AirQualitySensor::FRESH_AIR) {
    String Stage = "Fresh air!";
    Serial.println(Stage);
  }


  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  Serial.print("Humidity Level     : ");
  Serial.print(h);
  Serial.println(" %");

  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  Serial.print("Temperature Level  : ");
  Serial.print(t);
  Serial.println(" C");

  int value = analogRead(A2);
  float valuel = map(value, 0, 800, 0, 100);
  Serial.print("Light Sensor Level : ");
  Serial.print(valuel);
  Serial.println(" %");

  int rainSensor = analogRead(A3);
  float rainSensor1 = map(rainSensor, 0, 1024, 0, 100);
  Serial.print("Rain Sensor Level  : ");
  Serial.println(rainSensor);
  Serial.print("Dry level          : ");
  Serial.print(rainSensor1);
  Serial.println(" %");
 

  J *req = notecard.newRequest("note.add");
  if (req != NULL) {
    JAddStringToObject(req, "file", "sensors.qo");
    JAddBoolToObject(req, "sync", true);

    J *body = JCreateObject();
    if (body != NULL) {
      
      JAddNumberToObject(body, "temperature", t);
      JAddNumberToObject(body, "humidity", h);
      JAddNumberToObject(body, "air quality value", sensor.getValue());
      JAddNumberToObject(body, "light intencity", valuel);
      JAddNumberToObject(body, "rain sensor", rainSensor);
      JAddNumberToObject(body, "dry level", rainSensor1);
      JAddItemToObject(req, "body", body);
      
    }

    notecard.sendRequest(req);
  }

  Serial.println("------------------------------------------------");
  delay(10000);

}

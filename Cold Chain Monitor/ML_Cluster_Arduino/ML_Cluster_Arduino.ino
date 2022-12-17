
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <M5StickC.h>

// MESH Details
#define   MESH_PREFIX     "BlueMesh" //name for your MESH
#define   MESH_PASSWORD   "AamericanPsycho" //password for your MESH
#define   MESH_PORT       5555 //default port

float humi;
float tempC;

//EI Side
#include <DHT.h>
#include <freezer-monitor_inferencing.h>
#define DHT_SENSOR_PIN  14 // ESP32 pin GIOP21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

// to classify 1 frame of data you need EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE values
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
// keep track of where we are in the feature array
size_t feature_ix = 0;

static bool debug_nn = false;

float anomalydata;

typedef struct {
  int anomaly;
  int var1;
  int var2;

} WeatherData;

// Forward declarations
void getWeatherData(WeatherData &data);
void sendWeatherData(const WeatherData &data);

// Global variables
char sendBuf[256];

//LED
#define LED 2

//Number for this node
int nodeNumber = 3;

//String to send to other nodes with sensor readings
String readings;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND * 5 , TASK_FOREVER, &sendMessage);

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["Box"] = nodeNumber;
  jsonReadings["Box3_temp"] = tempC ;
  jsonReadings["Box3_humi"] = humi;
  jsonReadings["Box3_Anomaly_Score"] = anomalydata;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.println(msg.c_str());
  digitalWrite(LED, HIGH);
  delay(100);                           // wait for 1 second.
  digitalWrite(LED, LOW);                // turn the LED on.
  delay(100); // wait for 1 second.
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  M5.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setSwapBytes(true);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(7, 20, 2);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);

  Serial.println("Edge Impulse Inferencing Demo");
  dht_sensor.begin(); // initialize the DHT sensor


  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {

  static unsigned long last_interval_ms = 0;

  WeatherData data;
  getWeatherData(data);
  sendWeatherData(data);
  delay(3000);


  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

    humi  = dht_sensor.readHumidity();
    tempC = dht_sensor.readTemperature();
    Serial.println("---------------------------------------------------------------------------------");
    Serial.print("Temperature : ");
    Serial.print(tempC);
    Serial.print("  Humidity : ");
    Serial.println(humi);

    // keep filling the features array until it's full
    features[feature_ix++] = humi;
    features[feature_ix++] = tempC;

    // full frame of data?
    if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      ei_impulse_result_t result;

      // create signal from features frame
      signal_t signal;
      numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

      // run classifier
      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);
            ei_printf("run_classifier returned: %d\n", res);
      if (res != 0) return;

      // print predictions
            ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                      result.timing.dsp, result.timing.classification, result.timing.anomaly);

      // print the predictions
      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                ei_printf("%s:\t%.5f\n", result.classification[ix].label, result.classification[ix].value);
      }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
            ei_printf("anomaly:\t%.3f\n", result.anomaly);
      anomalydata = (float)result.anomaly;

#endif

      // reset features frame
      feature_ix = 0;
    }
  }

  // it will run the user scheduler as well
  mesh.update();
}

void getWeatherData(WeatherData &data) {
  ei_impulse_result_t result;
  // This just generates random data for testing
  data.anomaly = anomalydata;
  data.var1 = rand();
  data.var2 = rand();

}

void sendWeatherData(const WeatherData &data) {

  snprintf(sendBuf, sizeof(sendBuf), "%d,%d\n",
           data.anomaly, data.var1, data.var2);
  Serial.print(sendBuf);

}


void ei_printf(const char *format, ...) {
  static char print_buf[1024] = { 0 };

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0) {
    Serial.write(print_buf);
  }
}

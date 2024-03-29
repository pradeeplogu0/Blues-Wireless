
#include <Notecard.h>
#define txRxPinsSerial Serial1
#define productUID "com.gmail.pradeeplogu26:wio_gate_keeper"
Notecard notecard;

#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>
GroveAI ai(Wire);
uint8_t state = 0;

void setup() {

  notecard.begin(txRxPinsSerial, 9600);
  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  notecard.sendRequest(req);
  delay(1000);

  Wire.begin();
  Serial.begin(115200);
  Serial.println("begin");
  if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) // Object detection and pre-trained model 1
  {
    Serial.print("Version: ");
    Serial.println(ai.version());
    Serial.print("ID: ");
    Serial.println( ai.id());
    Serial.print("Algo: ");
    Serial.println( ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());
    state = 1;
  }
  else
  {
    Serial.println("Algo begin failed.");
  }

}

void loop(){
if (state == 1)
{
  uint32_t tick = millis();
  if (ai.invoke()) // begin invoke
  {
    while (1) // wait for invoking finished
    {
      CMD_STATE_T ret = ai.state();
      if (ret == CMD_STATE_IDLE)
      {
        break;
      }
      delay(20);
    }

    uint8_t len = ai.get_result_len(); // receive how many people detect
    if (len)
    {
      int time1 = millis() - tick;
      Serial.print("Time consuming: ");
      Serial.println(time1);
      Serial.print("Number of people: ");
      Serial.println(len);
      object_detection_t data;       //get data

      for (int i = 0; i < len; i++)
      {
        Serial.println("result:detected");
        Serial.print("Detecting and calculating: ");
        Serial.println(i + 1);
        ai.get_result(i, (uint8_t*)&data, sizeof(object_detection_t)); //get result

        Serial.print("confidence:");
        Serial.print(data.confidence);
        Serial.println();
        ////////////////////////////////////////////////
        J *req = notecard.newRequest("note.add");
        if (req != NULL) {
          JAddStringToObject(req, "file", "sensors.qo");
          JAddBoolToObject(req, "sync", true);

          J *body = JCreateObject();
          if (body != NULL) {
            JAddNumberToObject(body, "confidence", data.confidence);
            JAddNumberToObject(body, "count", len);
            JAddStringToObject(body, "result", "detected");
            JAddItemToObject(req, "body", body);
          }
          notecard.sendRequest(req);
        }
      }
    }
    else
    {
      Serial.println("No identification");
    }
  }
  else
  {
    delay(1000);
    Serial.println("Invoke Failed.");
  }
}
else
{
  state == 0;
}
}

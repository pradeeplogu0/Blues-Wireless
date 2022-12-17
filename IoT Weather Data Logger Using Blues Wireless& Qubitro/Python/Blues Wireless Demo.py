import json
import notecard
import serial
import time

productUID = "com.gmail.pradeeplogu26:blues_wio_anomaly_detector"

if __name__ == '__main__':
    ser = serial.Serial('COM22', 115200, timeout=1)
    ser.flush()
    serial = serial.Serial('COM7', 9600)
    card = notecard.OpenSerial(serial, debug=True)

    req = {"req": "hub.set"}
    req["product"] = productUID
    req["mode"] = "continuous"
    card.Transaction(req)

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            json_without_slash = json.loads(line)
            print("Serial Data is " + str(json_without_slash))
            
            req = {"req": "note.add"}
            req["file"] = "sensors.qo"
            req["start"] = True
            req["body"] = json_without_slash
            
            print("Req is : " + str(req))
            card.Transaction(req)
            time.sleep(5)



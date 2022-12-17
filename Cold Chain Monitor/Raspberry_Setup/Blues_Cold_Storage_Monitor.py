import json
import serial
import time
import notecard

productUID = "com.gmail.pradeeplogu26:cold_storage_monitor"

if __name__ == '__main__':
    ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
    ser.flush()
    serial = serial.Serial("/dev/ttyACM0", 9600)
    card = notecard.OpenSerial(serial)

    req = {"req": "hub.set"}
    req["product"] = productUID
    req["mode"] = "continuous"
    card.Transaction(req)
    

def is_json(myjson):
  try:
    json_object = json.loads(myjson)
    req = {"req": "note.add"}
    req["file"] = "sensors.qo"
    req["start"] = True
    req["body"] = json_object
            
    print("Req is : " + str(req))
    card.Transaction(req)
    print("------------------------------------------------")
    time.sleep(5)
    
  except ValueError as e:
    return False
  return True
    
while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            myjson=line
            print(is_json(myjson))
          
            


   
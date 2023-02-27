from json.decoder import JSONDecodeError
import subprocess
import json
import base64
import serial

uart_grove = serial.Serial('/dev/ttyS1', 9600, timeout=0.5)
reconizer = subprocess.Popen(['/home/m5stack/payload/bin/object_recognition', '/home/m5stack/payload/uploads/models/v2model_2d03a4c01727079a'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
reconizer.stdin.write("_{\"stream\":1}\r\n".encode('utf-8'))
reconizer.stdin.flush()

img = b''

while 1:
    doc = json.loads(reconizer.stdout.readline().decode('utf-8'))
    uart_grove.write(('{"req":"hub.set", "product":"com.gmail.pradeeplogu26:v2"}' + '\n').encode('utf-8'))
    
    if 'img' in doc:
        byte_data = base64.b64decode(doc["img"])
        img = bytes(byte_data)

    elif 'num' in doc:
        for obj in doc['obj']:
            Object =str(obj['type'])
            
            if Object=="Perfect":
                req={}
                req = {"req": "note.add"}
                req["sync"] = True
                req["body"] = {"Object": str(Object),"Alarm":True}
                json_data = json.dumps(req)
                print(json_data)
                uart_grove.write(str(json_data).encode('utf-8'))

            else:
                req={}
                req = {"req": "note.add"}
                req["sync"] = True
                req["body"] = {"Object": str(Object),"Alarm":False}
                json_data = json.dumps(req)
                print(json_data)
                uart_grove.write(str(json_data).encode('utf-8'))

import json
import notecard
import serial
import time

productUID = "com.gmail.pradeeplogu26:env_data_logger"

# Select Serial or I2C with this flag
use_uart = True
card = None

# Configure the serial connection to the Notecard
serial = serial.Serial('COM7', 9600)
card = notecard.OpenSerial(serial, debug=True)


req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"
card.Transaction(req)

while True:

    temp = 10
    humidity = 20

    print('Temperature: {} degrees C'.format(temp))
    print('Humidity: {}%'.format(humidity))

    req = {"req": "note.add"}
    req["file"] = "sensors.qo"
    req["start"] = True
    req["body"] = {"temp": temp, "humidity": humidity}
    card.Transaction(req)

    time.sleep(15)
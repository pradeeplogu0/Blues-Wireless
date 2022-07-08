import requests
import time
from colorama import Fore, Back, Style

while (True):
    url = "https://api.qubitro.com/v1/projects/e675f223-ff1e-4bab-85e1-37de1e944f6d/devices/dev%3A867730051771776-e675f223-ff1e-4bab-85e1-37de1e944f6d"
    headers = {
    "Accept": "application/json",
    "Authorization": "Bearer-xxxxxxxxxxxxxxxxxxxxxxxx"
     }
    response = requests.get(url, headers=headers)
    print(response.text)
    time.sleep(10)
import xml.etree.ElementTree as ET
import requests
import serial
import struct
import time
from uptime import *
from datetime import datetime

lat = 28.6918
lon = -81.4187
url = 'https://forecast.weather.gov/MapClick.php?lat=' + str(lat) + '&lon=' + str(lon) + '&unit=0&lg=english&FcstType=dwml'
xml = requests.get(url)

root = ET.fromstring(xml.text)

creationDateElem = root.findall('.//creation-date')
creationDate = creationDateElem[0].text

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/temperature[@type="apparent"]/value'
tempElem = root.findall(path)
currentTemp = 0
for i in tempElem:
    currentTemp = i.text
    break   # should only be one current temp, but just in case only take the first one returned

path = './/data[@type="forecast"]/parameters[@applicable-location="point1"]/temperature[@type="maximum"]/value'
tempElem = root.findall(path)
hiTemp = 0
for i in tempElem:
    hiTemp = i.text
    break   # data includes forecasts for 7 days, only take the first one
 
path = './/data[@type="forecast"]/parameters[@applicable-location="point1"]/temperature[@type="minimum"]/value'
tempElem = root.findall(path)
loTemp = 0
for i in tempElem:
    loTemp = i.text
    break   # see maximum above

up = int(uptime())

ser = serial.Serial('COM3', 9600)
time.sleep(2)

ser.write(struct.pack('<l', up))
time.sleep(1)
ser.write(struct.pack('<l', int(loTemp)))
time.sleep(1)
ser.write(struct.pack('<l', int(currentTemp)))
time.sleep(1)
ser.write(struct.pack('<l', int(hiTemp)))
time.sleep(1)

ser.close()

currentTime = datetime.now()

print(currentTime, creationDate, int(up), loTemp, currentTemp, hiTemp, sep=",")


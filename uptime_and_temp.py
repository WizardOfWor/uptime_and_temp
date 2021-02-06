import xml.etree.ElementTree as ET
import os
import requests
import serial
import struct
import time
from uptime import *
from datetime import datetime, timezone

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

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/humidity[@type="relative"]/value'
humidElem = root.findall(path)
currentHumid = 0
for i in humidElem:
    currentHumid =int(i.text)
    break   # should only be one current humidity, but just in case only take the first one returned

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/direction[@type="wind"]/value'
windDirElem = root.findall(path)
currentWindDir = 0
for i in windDirElem:
    currentWindDir = int(i.text)
    break   # should only be one current wind direction, but just in case only take the first one returned

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/wind-speed[@type="sustained"]/value'
windSpeedElem = root.findall(path)
currentWindSpeed = 0
for i in windSpeedElem:
    currentWindSpeed = int(i.text)
    break   # should only be one current wind direction, but just in case only take the first one returned

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/wind-speed[@type="gust"]/value'
windGustElem = root.findall(path)
currentWindGust = 0
for i in windGustElem:
    if i.text != "NA":
        currentWindGust = int(i.text)
    break   # should only be one current wind direction, but just in case only take the first one returned

otherWeatherInfo = currentHumid
otherWeatherInfo |= currentWindSpeed << 8
otherWeatherInfo |= currentWindDir << 16
windWeatherInfo = currentWindGust

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

dt = datetime.now()
epochSecs = int(dt.timestamp())

ser = serial.Serial('COM3', 9600)
time.sleep(2)

ser.write(struct.pack('<l', up))
ser.write(struct.pack('<l', epochSecs))
ser.write(struct.pack('<l', int(loTemp)))
ser.write(struct.pack('<l', int(currentTemp)))
ser.write(struct.pack('<l', int(hiTemp)))
ser.write(struct.pack('<l', int(otherWeatherInfo)))
ser.write(struct.pack('<l', int(windWeatherInfo)))
time.sleep(1)

ser.close()

currentTime = datetime.now()

print(currentTime, creationDate, int(up), loTemp, currentTemp, hiTemp, int(epochSecs), currentHumid, currentWindSpeed, currentWindDir, otherWeatherInfo, sep=",")


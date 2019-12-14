import xml.etree.ElementTree as ET
import requests
import serial
import struct
import time
from uptime import *

xml = requests.get('https://forecast.weather.gov/MapClick.php?lat=28.6918&lon=-81.4187&unit=0&lg=english&FcstType=dwml')

root = ET.fromstring(xml.text)

creationDateElem = root.findall('.//creation-date')
creationDate = creationDateElem[0].text

path = './/data[@type="current observations"]/parameters[@applicable-location="point1"]/temperature[@type="apparent"]/value'
tempElem = root.findall(path)
currentTemp = 0
for i in tempElem:
    currentTemp = i.text

path = './/data[@type="forecast"]/parameters[@applicable-location="point1"]/temperature[@type="maximum"]/value'
tempElem = root.findall(path)
hiTemp = 0
for i in tempElem:
    hiTemp = i.text
 
path = './/data[@type="forecast"]/parameters[@applicable-location="point1"]/temperature[@type="minimum"]/value'
tempElem = root.findall(path)
loTemp = 0
for i in tempElem:
    loTemp = i.text

up = uptime()

ser = serial.Serial('COM3', 9600)
time.sleep(2)

#ser.write(struct.pack('>iiii', int(up), int(loTemp), int(currentTemp), int(hiTemp)))
ser.write(struct.pack('<i', int(up)))
time.sleep(1)
ser.write(struct.pack('<i', int(loTemp)))
time.sleep(1)
ser.write(struct.pack('<i', int(currentTemp)))
time.sleep(1)
ser.write(struct.pack('<i', int(hiTemp)))
time.sleep(1)

ser.close()

print(creationDate, up, loTemp, currentTemp, hiTemp, sep=",")


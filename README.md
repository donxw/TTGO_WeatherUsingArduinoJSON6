# How to use a TTGO-T-Display ESP32 board and ArduinoJason 6 library to display current weather

Hardware:  https://github.com/Xinyuan-LilyGO/TTGO-T-Display  
Purchase:  https://www.aliexpress.com/item/33048962331.html?spm=a2g0o.productlist.0.0.1b627a05OAaRf9&algo_pvid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4&algo_expid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4-1&btsid=a7eaf880-8e7c-437a-be59-8efd9408aa30&ws_ab_test=searchweb0_0,searchweb201602_5,searchweb201603_55

This code will demonstrate a very basic program that retrieves weather from openweathermap.org and parses the resulting data using ArduinoJson 6 and displays it onto a TFT Display.

## Section 1:  Getting the Weather data and Parsing it to Arduino Variables you can use**  
The weather data is available through api.openweathermap.org.  To get weather data sent to your code, you will need an api key.  Go to https://openweathermap.org/api, scroll to the bottom and follow the instructions to get started.  The site provides a lot of useful examples on how to structure your weather request here:  https://openweathermap.org/current

I used the request URL using Zipcode:  Sample from the website -  http://samples.openweathermap.org/data/2.5/weather?zip=94040,us&appid=b6907d289e10d714a6e88b30761fae22

The URL I used is this:  http://api.openweathermap.org/data/2.5/weather?zip=94541&units=imperial&APPID=122c8b5cd4731038ff78486f1faa70c5.
If you put this into a browser, it will return JSON formatted data containing the current weather:
```
{"coord":{"lon":-122.09,"lat":37.67},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":61.5,"feels_like":60.67,"temp_min":57,"temp_max":66.2,"pressure":1020,"humidity":93},"visibility":11265,"wind":{"speed":6.93,"deg":270},"clouds":{"all":1},"dt":1579996765,"sys":{"type":1,"id":4322,"country":"US","sunrise":1579965455,"sunset":1580001801},"timezone":-28800,"id":0,"name":"Hayward","cod":200}
```
Using a browser is straight forward, but there are two hurdles to use an arduino
1) Sending the api url string to the website
2) Parsing the JSON data that is returned
---
**Sending an HTTP request (send the url) from Arduino**

Step 1)  Include the right libraries.  Arduino.cc has a good tutorial for installing libraries.  For and ESP32 TTGOI used:
```
#include <WiFiClient.h> // - https://www.arduino.cc/en/Reference/WiFiClient provides a good reference
#include <WiFi.h>
#include <WiFiManager.h> // - this is optional, but it's nice because it allows the end user to easily attach the device to his network.
```
WiFiManager is available here and has great examples: https://github.com/zhouhan0126/WIFIMANAGER-ESP32  
*I had to make a mod to get it to compile in Arduino on an ESP32 board. The mod was to change all instances of HTTP_HEAD[] to HTTP_HEAD_HTML[] in both the WifiManager.cpp and WiFiManager.h files.  It was fine in Platform IO without any modifications.*

Step 2)  Connect to the network and send the api request in a url.
* Connecting to the network is just these two lines in the setup() code:
```
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
```  

* Sending the url is trickier - the arduino WiFiClient reference above does a good job.  I used the example from Benoit Blanchon at arduinojason.org.  

- initialize the client before the setup() code
```
  WiFiClent client;
  const int httpPort = 80;
  char *servername = "api.openweathermap.org";
```    
- after the code to connect to the network, connect either in setup() or loop():
```
  client.connect(servername, httpPort);   
  //In the actual code there is some error checking, but basically this line connects you to api.openweathermap.org
```    
- use client.print to send the request as follows:
```    
  String url = "/data/2.5/weather?zip=" + ZipCode + "&units=imperial&APPID=" + APIKEY;
  client.print(String("GET ") + url + " HTTP/1.1\r\n");
  client.print("Host: api.openweathermap.org\r\n");
  client.print("Connection: close\r\n\r\n");
```  
---
**Parsing the returned JSON data**

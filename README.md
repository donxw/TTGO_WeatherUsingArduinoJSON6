# How to use a TTGO-T-Display ESP32 board and ArduinoJason 6 library to display current weather

Hardware:  https://github.com/Xinyuan-LilyGO/TTGO-T-Display  
Purchase:  https://www.aliexpress.com/item/33048962331.html?spm=a2g0o.productlist.0.0.1b627a05OAaRf9&algo_pvid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4&algo_expid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4-1&btsid=a7eaf880-8e7c-437a-be59-8efd9408aa30&ws_ab_test=searchweb0_0,searchweb201602_5,searchweb201603_55

This code demonstrates a basic program that retrieves weather from openweathermap.org, parses the resulting data into variables then displays it onto the built in TFT Display on the TTGO ESP32 board.

## Section 1:  Getting the Weather data and Parsing it to Arduino Variables you can use  
The weather data is available through api.openweathermap.org.  To get weather data sent to your code, you will need an api key.  Go to https://openweathermap.org/api, scroll to the bottom and follow the instructions to get started.  The site provides a lot of useful examples on how to structure your weather request here:  https://openweathermap.org/current

I used the request URL using Zipcode:  Sample from the website -  http://samples.openweathermap.org/data/2.5/weather?zip=94040,us&appid=b6907d289e10d714a6e88b30761fae22

The URL I used is this:  http://api.openweathermap.org/data/2.5/weather?zip=94541&units=imperial&APPID=122c8b5cd4731038ff78486f1faa70c5.
If you put this into a browser, it will return JSON formatted data containing the current weather:
```
{"coord":{"lon":-122.09,"lat":37.67},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":61.5,"feels_like":60.67,"temp_min":57,"temp_max":66.2,"pressure":1020,"humidity":93},"visibility":11265,"wind":{"speed":6.93,"deg":270},"clouds":{"all":1},"dt":1579996765,"sys":{"type":1,"id":4322,"country":"US","sunrise":1579965455,"sunset":1580001801},"timezone":-28800,"id":0,"name":"Hayward","cod":200}
```
Take note of this returned JSON data, because it will be used later by an assistant program at https://arduinojson.org to create parsing code.  
Here is how you can get the JSON data into an Arduino project.  There are two steps:  
1) Send the api url string to the website
2) Parse the JSON data that is returned
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
- if all went well, the client variable will receive and store the JSON data returned from api.openweathermap.org. The next step is extracting this data and assigning it to variables that can be used.
---
**Parsing the returned JSON data to Arduino variables**  

This next step heavily relies on the the JSON Assistant located here:  https://arduinojson.org/v6/assistant/.  This assistant can generate all the code needed to parse the returned JSON data within the client variable by providing a sample of the JSON data.  Here is one way to do it:  
1)  as shown above, manually entering the api url into a browser returns the JSON data to the browser window.  Copy this data.
2)  navigate to the JSON Assistant - https://arduinojson.org/v6/assistant/
3)  paste in the JSON data - example data:  
{"coord":{"lon":-122.09,"lat":37.67},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":61.5,"feels_like":60.67,"temp_min":57,"temp_max":66.2,"pressure":1020,"humidity":93},"visibility":11265,"wind":{"speed":6.93,"deg":270},"clouds":{"all":1},"dt":1579996765,"sys":{"type":1,"id":4322,"country":"US","sunrise":1579965455,"sunset":1580001801},"timezone":-28800,"id":0,"name":"Hayward","cod":200}
4) the JSON assistant will return the parsing code which is very nearly usable as is:
```
const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 280;
DynamicJsonDocument doc(capacity);

const char* json = "{\"coord\":{\"lon\":-122.09,\"lat\":37.67},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":61.5,\"feels_like\":60.67,\"temp_min\":57,\"temp_max\":66.2,\"pressure\":1020,\"humidity\":93},\"visibility\":11265,\"wind\":{\"speed\":6.93,\"deg\":270},\"clouds\":{\"all\":1},\"dt\":1579996765,\"sys\":{\"type\":1,\"id\":4322,\"country\":\"US\",\"sunrise\":1579965455,\"sunset\":1580001801},\"timezone\":-28800,\"id\":0,\"name\":\"Hayward\",\"cod\":200}";

deserializeJson(doc, json);

float coord_lon = doc["coord"]["lon"]; // -122.09
float coord_lat = doc["coord"]["lat"]; // 37.67

JsonObject weather_0 = doc["weather"][0];
int weather_0_id = weather_0["id"]; // 800
const char* weather_0_main = weather_0["main"]; // "Clear"
const char* weather_0_description = weather_0["description"]; // "clear sky"
const char* weather_0_icon = weather_0["icon"]; // "01d"

const char* base = doc["base"]; // "stations"

JsonObject main = doc["main"];
float main_temp = main["temp"]; // 61.5
float main_feels_like = main["feels_like"]; // 60.67
int main_temp_min = main["temp_min"]; // 57
float main_temp_max = main["temp_max"]; // 66.2
int main_pressure = main["pressure"]; // 1020
int main_humidity = main["humidity"]; // 93

int visibility = doc["visibility"]; // 11265

float wind_speed = doc["wind"]["speed"]; // 6.93
int wind_deg = doc["wind"]["deg"]; // 270

int clouds_all = doc["clouds"]["all"]; // 1

long dt = doc["dt"]; // 1579996765

JsonObject sys = doc["sys"];
int sys_type = sys["type"]; // 1
int sys_id = sys["id"]; // 4322
const char* sys_country = sys["country"]; // "US"
long sys_sunrise = sys["sunrise"]; // 1579965455
long sys_sunset = sys["sunset"]; // 1580001801

int timezone = doc["timezone"]; // -28800
int id = doc["id"]; // 0
const char* name = doc["name"]; // "Hayward"
int cod = doc["cod"]; // 200
```
5) Tweak the code from the assistant.  You will need to make two mandatory and one optional change to the code:  

Change 1:  add more padding to the variable "capacity":
```
// update the padding from 280 to 1333 per the recommended size for Visual Studio x64
const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 1333;  
```
Change 2:  this line of code needs to change as follows:
```
//deserializeJson(doc, json);  //from this
deserializeJson(doc, client);  //to this.  You want to parse the data the client recieved.
```
Change 3:  optionally add error checking to the deserialization call:
```
  DeserializationError error = deserializeJson(doc, client);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    //return;  //uncommented to turn it into a hard fail instead of warning
  }
```
## Section 2:  Displaying Weather Data to the TTGO-T-Display Built-in Screen  

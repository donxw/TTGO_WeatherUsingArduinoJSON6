# How to use a TTGO-T-Display ESP32 board and ArduinoJason 6 library to display current weather
![TTGO_jason6weather](https://user-images.githubusercontent.com/31633408/83311107-16b6e100-a1c3-11ea-9e81-40330834d1f3.jpeg)
This code demonstrates a basic program that retrieves weather from openweathermap.org, parses the resulting data into variables then displays it onto the built in TFT Display on the TTGO ESP32 board.  These topics are lightly covered:
* Using the ArduinoJSON Assistant
* Get JSON data from openweather.org
* Example use of wifimanager
* Basic use of the TFT display on the TTGO-T-Display ESP32 board

It is provided just for educational interest and fun.

Hardware:  https://github.com/Xinyuan-LilyGO/TTGO-T-Display  
Purchase:  https://www.aliexpress.com/item/33048962331.html?spm=a2g0o.productlist.0.0.1b627a05OAaRf9&algo_pvid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4&algo_expid=b48e0ab9-ee06-4d87-a31e-0962d569f0f4-1&btsid=a7eaf880-8e7c-437a-be59-8efd9408aa30&ws_ab_test=searchweb0_0,searchweb201602_5,searchweb201603_55

## Section 1:  Getting the Weather data and Parsing it to Arduino Variables  
The weather data is available from api.openweathermap.org.  To get weather data sent to your code, you will need an api key.  Go to https://openweathermap.org/api, scroll to the bottom and follow the instructions to get started.  The site provides a lot of useful examples on how to structure your weather request here:  https://openweathermap.org/current.  The free version is limited to less that 60 requests per minute (https://openweathermap.org/price), which is plenty for personal use. But keep this in mind in your projects.

Here is a sample url to get weather JSON data from the website -  http://samples.openweathermap.org/data/2.5/weather?zip=94040,us&appid=b6907d289e10d714a6e88b30761fae22.  You can use this to see how to parse the output.

The URL I used is like this:  http://api.openweathermap.org/data/2.5/weather?zip=94541&units=imperial&APPID=xxxxxxxxxxxxxxxxxxxxxxxxxxx.
If you put this into a browser with your own zip and API, it will return JSON formatted data containing the current weather:
```
{"coord":{"lon":-122.09,"lat":37.67},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":61.5,"feels_like":60.67,"temp_min":57,"temp_max":66.2,"pressure":1020,"humidity":93},"visibility":11265,"wind":{"speed":6.93,"deg":270},"clouds":{"all":1},"dt":1579996765,"sys":{"type":1,"id":4322,"country":"US","sunrise":1579965455,"sunset":1580001801},"timezone":-28800,"id":0,"name":"Hayward","cod":200}
```
*Take note of this returned JSON data, because it will be used later by an assistant program at https://arduinojson.org to create parsing code.*  

### Here is how to send a URL and recieve JSON data in an Arduino project.  
---
**Sending an HTTP request (send the url) from Arduino**

Step 1)  Include the right libraries.  Arduino.cc has a good tutorial for installing libraries.
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
  wifiManager.autoConnect("AutoConnectAP");  //This will be the SSID broadcast to input WiFi credentials.  Rename as desired.
```  

* Sending the url is trickier - the arduino WiFiClient reference https://www.arduino.cc/en/Reference/WiFiClient does a good job of teaching this. The ArduinoJSON web site also has great examples:  https://arduinojson.org/v6/example/.

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
- if all went well, the client variable will receive and store the JSON data returned from api.openweathermap.org. The next step is extracting this data and assigning it to variables.
---
**Parsing the returned JSON data to Arduino variables**  

This next step heavily relies on the the JSON Assistant located here:  https://arduinojson.org/v6/assistant/.  This assistant can generate all the code needed to parse the returned JSON data from a sample of the JSON data you would like parsed.  Here is one way to do it:  
1)  as shown above, manually enter the api url into a browser.  The JSON data is returned to the browser window.  Copy this data.
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
Change 2:  update the deserialize parameter to client instead of json:
```
/**** Comment out or delete this line  *********************/
//const char* json = "{\"coord\":{\"lon\":-122.09,\"lat\":37.67},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":61.5,\"feels_like\":60.67,\"temp_min\":57,\"temp_max\":66.2,\"pressure\":1020,\"humidity\":93},\"visibility\":11265,\"wind\":{\"speed\":6.93,\"deg\":270},\"clouds\":{\"all\":1},\"dt\":1579996765,\"sys\":{\"type\":1,\"id\":4322,\"country\":\"US\",\"sunrise\":1579965455,\"sunset\":1580001801},\"timezone\":-28800,\"id\":0,\"name\":\"Hayward\",\"cod\":200}";

/**** Change this line to parse client instead of json  ****/
//deserializeJson(doc, json);  //from this
deserializeJson(doc, client);  //to this.  You want to parse the data the client recieved.
```
Change 3:  optionally add error checking to the deserialization call (recommended in the jsonarduino examples):
```
  DeserializationError error = deserializeJson(doc, client);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    //return;  //uncommented to turn it into a hard fail instead of warning
  }
```
Once this code runs, the website will return the JSON data to the client and the code provided by the ArduinoJSON Assistant will parse it into variables.  All that is left is to display it to the built in screen.

## Section 2:  Displaying Weather Data to the TTGO-T-Display Built-in Screen  

Using the built in display requires the installation of the libraries kept at https://github.com/Xinyuan-LilyGO/TTGO-T-Display.  First the libraries must be installed, then a minor change is needed to add an include file to one of the library headers.  It is well described in the library link, but basically consists of commenting one line and uncommenting another in the TFT_eSPI/User_Setup_Select.h library file.

Once the libraries are installed, include the following files:
```
//************************************* TTGO-T-Display Libraries *************************************************************
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <Button2.h>

TFT_eSPI tft = TFT_eSPI(135, 240);
```
Optionally, custom fonts can be installed.  To install a font, the font's bitmap file needs to be placed in the library\TFT_eSPI\Fonts\Custom folder.  A few sample fonts come with the library.  The new fonts will also need to be included in the User_Custom_Fonts.h file as follows:
```
#ifdef LOAD_GFXFF

  // New custom font file #includes
  #include <Fonts/Custom/Orbitron_Light_24.h> // CF_OL24
  #include <Fonts/Custom/Orbitron_Light_32.h> // CF_OL32
  #include <Fonts/Custom/Roboto_Thin_24.h>    // CF_RT24
  #include <Fonts/Custom/Satisfy_24.h>        // CF_S24
  #include <Fonts/Custom/Yellowtail_32.h>     // CF_Y32
  #include <Fonts/Custom/Crafty_Girls_Regular_16.h>  //CF_CG16
  #include <Fonts/Custom/Crafty_Girls_Regular_20.h>  //CF_CG20
  #include <Fonts/Custom/Crafty_Girls_Regular_24.h>  //CF_CG24
  #include <Fonts/Custom/DSEG7_Classic_Bold_24.h>  //CF_DSEG24
  #include <Fonts/Custom/DSEG7_Classic_Bold_36.h>  //CF_DSEG36
  #include <Fonts/Custom/Aclonica_Regular_24.h>  //CF_Aclon24

#endif

// Shorthand references - any coding scheme can be used, here CF_ = Custom Font
// The #defines below MUST be added to sketches to use shorthand references, so
// they are only put here for reference and copy+paste purposes!
/*
#define CF_OL24 &Orbitron_Light_24
#define CF_OL32 &Orbitron_Light_32
#define CF_RT24 &Roboto_Thin_24
#define CF_S24  &Satisfy_24
#define CF_Y32  &Yellowtail_32
#define CF_DSEG24 &DSEG7_Classic_Bold_24
#define CF_DSEG36 &DSEG7_Classic_Bold_36
#define CF_Aclon24 &Aclonica_Regular_24
#define CF_CG16 &Crafty_Girls_Regular_16  
#define CF_CG20 &Crafty_Girls_Regular_20
#define CF_CG24 &Crafty_Girls_Regular_24
*/
```

Creating custom fonts can be done at several websites with online tools and with a command line tool which Adafruit provides in their GFX library.  A few good links for online converters are:
* http://oleddisplay.squix.ch/#/home
* https://rop.nl/truetype2gfx/
* http://www.rinkydinkelectronics.com/t_make_font_file.php
* https://github.com/adafruit/Adafruit-GFX-Library

Once the library files are updated with the custom fonts, they will automatically be included in the <TFT_eSPI.h> file provided the GFXFF flag is set as follows:

```
//custom fonts - must be located in the library under  "TFT_eSPI\Fonts\Custom" folder
// a #include must also be added to the "User_Custom_Fonts.h" file in the "TFT_eSPI\User_Setups" folder.
// Stock font and GFXFF reference handle
#define GFXFF 1  //switches on including the custom font files
#define FF18 &FreeSans12pt7b  //these defines are optional, but they reduce typing when the fonts are used
#define CF_CG16 &Crafty_Girls_Regular_16  
#define CF_CG20 &Crafty_Girls_Regular_20
```

The current font is set when the display is initialized.
```
  //************************************* Initialize the Screen **************************************************
  tft.init();
  tft.setRotation(1);                       // set rotation 
  tft.setTextColor(TFT_WHITE, TFT_RED);     // Set font color, white font, red background
  tft.fillScreen(TFT_RED);                  // Clear screen
  tft.setFreeFont(CF_CG20);                 // Select the font:  for print and printf or with GFXFF
  //***************************************************************************************************************

```
And now the weather variables can be displayed onto the screen.  There are a large number of ways to print, draw, show graphics which are well covered in the TTGO github site.  One of the simpler methods is to use the printf method, much as you would with Serial.  Here is one of many ways the weather can be displayed:

```
  //display the city name, temperature, humidity and description
  tft.setCursor(10,40);
  tft.printf("%s \n %.1fF %02d%% \n ", name, main_temp, main_humidity);
  tft.printf("%s\n", weather_0_description);
```

## Summary
This example just scratches the surface, but is enough to get a basic program capable of obtaining data from a website through an API URL, parse the resulting JSON data then display it on a TFT screen.

I posted a second program that makes the weather code a function then uses the built in buttons on the TTGO-T board to cycle through a set of zipcodes to report the weather for each.

I also posted a STEP file and 3D printable files for a general purpose case to put the TTGO-T-Display board into.  It is large enough to fit a battery.

![TTGO_Case](https://user-images.githubusercontent.com/31633408/73299642-70cb7980-41c4-11ea-9780-589656db571b.JPG)



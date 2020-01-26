
// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// This example shows how to parse a JSON document in an HTTP response.
// The program reads the weather once and displays it to the TTGO built in screen.
// 
//
// Step 1:  get a sample of the expected json file using your brower
// example from openweathermap.org:  api.openweathermap.org/data/2.5/weather?zip=73071,us&units=imperial&APPID=122c8b5cd4731038ff78486f1faa70c5
// cutpaste into arduinojason helper:  https://arduinojson.org/v6/assistant/
//   {"coord":{"lon":-97.41,"lat":35.23},"weather":[{"id":803,"main":"Clouds","description":"broken clouds","icon":"04d"}],"base":"stations","main":{"temp":47.28,"pressure":1023,"humidity":34,"temp_min":44.6,"temp_max":50},
//   "visibility":16093,"wind":{"speed":8.05,"deg":170},"clouds":{"all":75},"dt":1575317431,"sys":{"type":1,"id":5187,"country":"US","sunrise":1575292847,"sunset":1575328657},"timezone":-21600,"id":0,"name":"Norman","cod":200}
// Step 2:  paste in the capacity - const size_t capacity = JSON_ARRAY_SIZE..... (note:  this size seem borderline small, so I padded it - see code below)
// Step 3:  paste in the parsing lines generated by the helper
// https://arduinojson.org/v6/example/http-client/

#include <ArduinoJson.h>
#include <SPI.h>

//************************************* TTGO-T-Display Libraries *************************************************************
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"

//custom fonts - must be located in the library under  "TFT_eSPI\Fonts\Custom" folder
// a #include must also be added to the "User_Custom_Fonts.h" file in the "TFT_eSPI\User_Setups" folder.
// Stock font and GFXFF reference handle
#define GFXFF 1
#define FF18 &FreeSans12pt7b
#define CF_CG16 &Crafty_Girls_Regular_16  
#define CF_CG20 &Crafty_Girls_Regular_20
char oledbuf[240];   // for sprintf buffer
char tempchar[80] ;  // for dtostrf() conversions
String stringbuf;

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

//*******************************************************************************************************************************

// WiFi and AP
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
ESP8266WebServer server(80);
#else
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server(80);
#endif

#include <time.h>
//#include "credentials.h"
#include <WiFiManager.h>  //I copied the library from the arduino folder to this folder to hack it to work for ESP32

// Weather Variables
char *servername = "api.openweathermap.org"; // remote server we will connect to
//int iterations = 0;
//int weatherID = 0;
//String result, humid, loc, temp, weat, desc, tim, sunr, suns;

//String ZipCode = "73104,us"; //OKC
String ZipCode = "94304,us"; //Palo Alto
//String ZipCode = "94541,us"; //Hayward
String APIKEY = "122c8b5cd4731038ff78486f1faa70c5";

WiFiClient client;
const int httpPort = 80;

void setup()
{
  // Initialize Serial port
  Serial.begin(115200);
  while (!Serial)
    continue;

  //************************************* Initialize the Screen **************************************************
  tft.init();
  tft.setRotation(1);                                          // set rotation 
  tft.setTextColor(TFT_WHITE, TFT_RED);     // Set font color
  tft.fillScreen(TFT_RED);                                 // Clear screen
  tft.setFreeFont(CF_CG20);                           // Select the font:  for print and printf or with GFXFF
  //***************************************************************************************************************

  // Initialize wifi
  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  //display note to set up wifi
  tft.drawCentreString("Set Up Wifi @", tft.width()/2, 30, GFXFF);
  tft.drawCentreString("AutoConnectAP", tft.width()/2, 55, GFXFF);

  Serial.println(F("Connecting..."));
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  //delay(5000);  // just for debug
  tft.fillScreen(TFT_RED);
//*************************************  Get Weather ***********************************************************
  // Connect to HTTP server

  if (!client.connect(servername, httpPort))
  {
    return;
  }

  Serial.println(F("Connected!"));

  String url = "/data/2.5/weather?zip=" + ZipCode + "&units=imperial&APPID=" + APIKEY;

  // Send HTTP request
  Serial.println("Requesting URL: "+url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n");
  client.print("Host: api.openweathermap.org\r\n");
  client.print("Connection: close\r\n\r\n");

  delay(10);

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return;
  }

  // Allocate the JSON document as doc
  // Use arduinojson.org/v6/assistant to compute the capacity.  The pad came from the recommended "Additional bytes for strings duplication" size in the assistant.
  int PAD=1333;
  //****************************************************from https://arduinojson.org/v6/assistant/ **********************************************************************
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(13) + 270 + PAD;
  DynamicJsonDocument doc(capacity);
  //deserializeJson(doc, client);  // no error checking - never seems to fail or crash with propper PAD size, but also is a little risky
  //*********************************************************************************************************************************************************************
  
  // Instead of directly using the deserializeJson line, this modifies Parse JSON object as doc from client with error checking - this failed frequently until I added the padding to capacity so help in debugging the original code
  DeserializationError error = deserializeJson(doc, client);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    //return;  //commented this to turn it into a warning instead of a hard fail
  }
  

  //**************************************************** from https://arduinojson.org/v6/assistant/ **********************************************************************
  // Extract values
  float coord_lon = doc["coord"]["lon"]; // -97.41
  float coord_lat = doc["coord"]["lat"]; // 35.23

  JsonObject weather_0 = doc["weather"][0];
  int weather_0_id = weather_0["id"];                           // 800
  const char *weather_0_main = weather_0["main"];               // "Clear"
  const char *weather_0_description = weather_0["description"]; // "clear sky"
  const char *weather_0_icon = weather_0["icon"];               // "01d"

  const char *base = doc["base"]; // "stations"

  JsonObject main = doc["main"];
  float main_temp = main["temp"];         // 46.17
  int main_pressure = main["pressure"];   // 1025
  int main_humidity = main["humidity"];   // 36
  float main_temp_min = main["temp_min"]; // 44.6
  int main_temp_max = main["temp_max"];   // 48

  int visibility = doc["visibility"]; // 16093

  float wind_speed = doc["wind"]["speed"]; // 5.82
  int wind_deg = doc["wind"]["deg"];       // 210

  int clouds_all = doc["clouds"]["all"]; // 1

  long dt = doc["dt"]; // 1575312640

  JsonObject sys = doc["sys"];
  int sys_type = sys["type"];               // 1
  int sys_id = sys["id"];                   // 4424
  const char *sys_country = sys["country"]; // "US"
  long sys_sunrise = sys["sunrise"];        // 1575292847
  long sys_sunset = sys["sunset"];          // 1575328657

  int timezone = doc["timezone"]; // -21600
  int id = doc["id"];             // 0
  const char *name = doc["name"]; // "Norman"
  int cod = doc["cod"];           // 200
  //*********************************************************************************************************************************************************************

  Serial.println(name);
  Serial.println(main_temp);
  Serial.println(main_humidity);
  Serial.println(weather_0_description);

  //tft.fillScreen(TFT_BLACK)
  //tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setCursor(8,20);

  //*********** Option 1 ********* this works the best:  convert float to string
  tft.printf("%s \n %sF %02d%% \n ", name, dtostrf(main_temp, 2, 1, tempchar), main_humidity);
  tft.printf( "weather %s\n", weather_0_description);
  
  //********** Option 2 ********* works for float,  but cursor has to be manually managed
  //tft.drawFloat(main_temp, 1, 8, 60); tft.setCursor(20,60); tft.printf("F %02d%%", main_humidity);

  //********** Option 3 ********** This works well too, but sprintf seems to ignore \n  so need a differnt line of code for each line of text
  //tft.println(name);
  //sprintf(oledbuf, " %sF %02d%% \n ", dtostrf(main_temp, 2, 1, tempchar), main_humidity);
  //tft.print(oledbuf);
  //tft.drawString(oledbuf, 8, 30, GFXFF);   // this works too, but you have to track the cursor

  // Disconnect
  client.stop();
}

void loop()
{
  // not used in this example
}
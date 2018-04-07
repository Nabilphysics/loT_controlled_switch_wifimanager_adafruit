
/***************************************************
  Adafruit MQTT Library ESP8266 Example
  Edited by Syed Razwanul Haque (Nabil)
  CEO and Founder, www.cruxbd.com
  www.youtube.com/Nabilphysics

  This is a very basic code. I upload it just for an IDEA
  Three or more LED can be controlled by this code using adafruit IoT
  You can press D7 to enter WiFi configuration.
  Builtin LED will blink faster when it is in config mode.
  Builtin LED will be solid when adafruit server connection is achieved. 
  
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>  
//for LED status
#include <Ticker.h>
Ticker ticker;
// the on off button feed turns this LED on/off
#define LED1 D1  
#define LED2 D2
#define LED3 D3

// When Trigger Pin is pressed wifi manager will Trigger for Configuration Change.
// SSID, Password can be changed by Trigger Pin 
#define TRIGGER_PIN D7  
void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}
//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Username of io.adafruit.com"
#define AIO_KEY         "your AIO Key for Adafruit Account"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff1");
Adafruit_MQTT_Subscribe onoffbutton2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff2");
Adafruit_MQTT_Subscribe onoffbutton3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff3");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1,LOW);
  digitalWrite(LED2,LOW);
  digitalWrite(LED3,LOW);
  pinMode(TRIGGER_PIN, INPUT);
 
  Serial.begin(115200);
  Serial.println("\n Starting");


  delay(10);

  Serial.println(F("Smartswitch MQTT demo"));
  Serial.println(WiFi.SSID());
//  // Connect to WiFi access point.
//  Serial.println(); Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(WLAN_SSID);
//
//  WiFi.begin(WLAN_SSID, WLAN_PASS);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println();
//
//  Serial.println("WiFi connected");
//  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff & slider feed.
  mqtt.subscribe(&onoffbutton1);
  mqtt.subscribe(&onoffbutton2);
  mqtt.subscribe(&onoffbutton3);
}

uint32_t x=0;

void loop() {
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    ticker.attach(0.2, tick);
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    
    //reset settings - for testing
    //wifiManager.resetSettings();
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal("SmartSwitch Demo")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("Connected With SSID = ");
    //ticker.detach();
    Serial.print(WiFi.SSID());
    //keep LED on
    //digitalWrite(BUILTIN_LED, LOW);
  }
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10000))) {
    // Check if its the onoff button feed
    if (subscription == &onoffbutton1) {
      Serial.print(F("On-Off button: "));
      Serial.println((char *)onoffbutton1.lastread);
      
      if (strcmp((char *)onoffbutton1.lastread, "ON") == 0) {
        digitalWrite(LED1, HIGH); 
      }
      if (strcmp((char *)onoffbutton1.lastread, "OFF") == 0) {
        digitalWrite(LED1, LOW); 
      }
    }

    if (subscription == &onoffbutton2) {
      Serial.print(F("On-Off button 2: "));
      Serial.println((char *)onoffbutton2.lastread);
      
      if (strcmp((char *)onoffbutton2.lastread, "ON") == 0) {
        digitalWrite(LED2, HIGH); 
      }
      if (strcmp((char *)onoffbutton2.lastread, "OFF") == 0) {
        digitalWrite(LED2, LOW); 
      }
    }
     if (subscription == &onoffbutton3) {
      Serial.print(F("On-Off button 3: "));
      Serial.println((char *)onoffbutton3.lastread);
      
      if (strcmp((char *)onoffbutton3.lastread, "ON") == 0) {
        digitalWrite(LED3, HIGH); 
      }
      if (strcmp((char *)onoffbutton3.lastread, "OFF") == 0) {
        digitalWrite(LED3, LOW); 
      }
    }
    
//    // check if its the slider feed
//    if (subscription == &slider) {
//      Serial.print(F("Slider: "));
//      Serial.println((char *)slider.lastread);
//      uint16_t sliderval = atoi((char *)slider.lastread);  // convert to a number
//      analogWrite(PWMOUT, sliderval);
//    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    ticker.detach();
    digitalWrite(BUILTIN_LED, LOW);
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       ticker.attach(1.5, tick);
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  
}

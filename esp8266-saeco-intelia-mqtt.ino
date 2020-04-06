#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ArduinoOTA.h>
  
// WIFI
#define WLAN_SSID       "ssid"
#define WLAN_PASS       "pass"

WiFiClient client;

//MQTT SERVER
#define AIO_SERVER      "mqttserver"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "mqttuser"
#define AIO_KEY         "mqttpass"

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish cm_status = Adafruit_MQTT_Publish(&mqtt, "stat/coffee_maker/STATUS");
Adafruit_MQTT_Publish cm_log = Adafruit_MQTT_Publish(&mqtt, "stat/coffee_maker/LOG");
Adafruit_MQTT_Subscribe PB1 = Adafruit_MQTT_Subscribe(&mqtt, "cmnd/coffee_maker/PB1"); //small coffee button
Adafruit_MQTT_Subscribe PB2 = Adafruit_MQTT_Subscribe(&mqtt, "cmnd/coffee_maker/PB2"); //large coffee button
Adafruit_MQTT_Subscribe PB3 = Adafruit_MQTT_Subscribe(&mqtt, "cmnd/coffee_maker/PB3"); //tea water button
Adafruit_MQTT_Subscribe PB4 = Adafruit_MQTT_Subscribe(&mqtt, "cmnd/coffee_maker/PB4"); //on off button

//PIN DEFINITION
#define LED1 D8 //small coffee button
#define LED2 D7 //large coffee button
#define LED3 D6 //tea water button
#define LED4 D5 //on off button

#define LED5 D2 //red status led
#define LED6 D1 //green status led

// VARIABLES TO DETECT STATUS CHANGES
String status_string = "";
String previous_status = "";
//bool status_changed = false;

// ROUTINE TO CONNECT TO MQTT SERVER
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT connected!");
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Saeco Coffee Maker Wifi Hack"));
  
  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.print(WLAN_SSID);
  WiFi.hostname("coffee-controller");
  wifi_station_set_hostname("coffee-controller");
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();  

  // Setup MQTT subscriptions
  mqtt.subscribe(&PB1);
  mqtt.subscribe(&PB2);
  mqtt.subscribe(&PB3);
  mqtt.subscribe(&PB4);
  
  // set up pins for coffee maker push buttons
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  //Set up input pins for status LED
  pinMode(LED5, INPUT);
  pinMode(LED6, INPUT);
}

void loop() {
  ArduinoOTA.handle();
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &PB1) {
      cm_make_product(LED1);
    }
    else if (subscription == &PB2) {
      cm_make_product(LED2);
    }
    else if (subscription == &PB3) {
      cm_make_product(LED3);
      delay(500);
      cm_make_product(LED4); //tea water needs second button press to start...
      delay(500);
      while(cm_get_status() == "yellow") { //might need heating up
        delay(500);
      }
      delay(20000); //count down 10 seconds of tea water
      cm_make_product(LED1); //...and one to stop
    }
    else if (subscription == &PB4) {
      if(strcmp((char *)PB4.lastread, "ON") == 0) {
        cm_on_off(true);
      }
      else if (strcmp((char *)PB4.lastread, "OFF") == 0) {
        cm_on_off(false);
      }
    }    
  }

  //read status LEDs & output result
  cm_get_status();
}

String cm_get_status() {
  String status_str = "";
  String mqtt_status_str = "";
  
  if (digitalRead(LED5) == HIGH && digitalRead(LED6) == HIGH) {
    mqtt_status_str = String("{\"POWER\":\"ON\", \"STATUS\":\"YELLOW\"}");
    status_str = "yellow";
  }
  else if (digitalRead(LED5) == HIGH && digitalRead(LED6) == LOW) {
    mqtt_status_str = String("{\"POWER\":\"ON\", \"STATUS\":\"RED\"}");
    status_str = "red";
  }
  else if (digitalRead(LED5) == LOW && digitalRead(LED6) == HIGH) {
    mqtt_status_str = String("{\"POWER\":\"ON\", \"STATUS\":\"GREEN\"}");
    status_str = "green";
  }
  else if (digitalRead(LED5) == LOW && digitalRead(LED6) == LOW) {
    mqtt_status_str = String("{\"POWER\":\"OFF\", \"STATUS\":\"OFF\"}");
    status_str = "off";
  }

  if (previous_status != status_str) {
    previous_status = status_str;
    Serial.println(mqtt_status_str);
    cm_status.publish(mqtt_status_str.c_str());
  }
  
  return status_str;
}

void cm_make_product(int button) {
  if(cm_get_status() == "off") {
    cm_log.publish("Switching machine on");
    Serial.println("Switching machine on");
    cm_on_off(true);
    delay(500);
  }
  
  if(cm_get_status() != "green") {
    cm_log.publish("Cannot fulfill command, machine not ready");   
    Serial.println("Cannot fulfill command, machine not ready");   
  } else { 
    cm_log.publish("Preparing product");
    Serial.println("Preparing product");
    digitalWrite(button, HIGH);
    delay(100);
    digitalWrite(button, LOW);    
  }
}

void cm_push_button(int button) {
    digitalWrite(button, HIGH);
    delay(100);
    digitalWrite(button, LOW);      
}

void cm_on_off(bool on_or_off) {
  if((on_or_off == true and cm_get_status() == "off") or (on_or_off == false and cm_get_status() != "off")) {
    if(on_or_off == true) {
      cm_log.publish("Switching machine on");   
      Serial.println("Switching machine on");
    } else {
      cm_log.publish("Switching machine off");   
      Serial.println("Switching machine off");      
    }
    digitalWrite(LED4, HIGH);
    delay(100);
    digitalWrite(LED4, LOW); 
    cm_get_status(); 
    if(on_or_off == true) {
      while(cm_get_status() != "green") { //might need heating up
        delay(500);
      }
      cm_get_status();
    }
  }
}

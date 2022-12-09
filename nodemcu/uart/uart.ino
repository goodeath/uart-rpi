#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define MAXBUFFERSIZE (65535)
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

int INSIDE_LAB = 0;
#define AIO_SERVER      INSIDE_LAB ? "10.0.0.101" : "192.168.1.3"
#define AIO_SERVERPORT  1883                 // use 8883 for SSL
#define AIO_USERNAME    INSIDE_LAB ? "aluno" : ""
#define AIO_KEY         INSIDE_LAB ? "@luno*123" : ""

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish pub_sensor_update = Adafruit_MQTT_Publish(&mqtt,  "sensors/update");
Adafruit_MQTT_Subscribe sub_sensor_frequency = Adafruit_MQTT_Subscribe(&mqtt, "sensors/set_frequency");
void MQTT_connect();
String convert_intr_to_json(int *arr, int n);

#ifndef STASSID
#define STASSID "Neverland2G"
#define STAPSK  "longlife"
#define STASSID2 "INTELBRAS"
#define STAPSK2  "Pbl-Sistemas-Digitais"
#endif

#define CMD_NODEMCU_STATUS 0b001    
#define CMD_SENSOR_STATUS 0b010
#define CMD_SENSOR_READ 0b011
#define CMD_FREQUENCY 0b100
#define CMD_N 4

int CMD[CMD_N] = {CMD_NODEMCU_STATUS, CMD_SENSOR_STATUS, CMD_SENSOR_READ, CMD_FREQUENCY};
int SENSOR_VALUE[32][10] = {};
int SENSOR_STATUS[32] = {};


/**
 * Extract Cmd
 * 
 * Receive a word and extract only command part.
 *  
 * @return byte command binary
 */
byte extract_cmd(int   _word){
    int cmd = 0;
    for(int i=0;i<3;i++) cmd |= (_word & (1<<i));
    for(int i=0;i<CMD_N;i++) if(cmd == CMD[i]) return cmd;
    return 0;
}

byte extract_sensor(int _word){
  
  _word = _word >> 3;
  return _word;
}


// Ignore all messages before this.
char key[6] = {'u','n','l','o','c','k'};

const char* ssid = INSIDE_LAB ? STASSID2 : STASSID;
const char* password = INSIDE_LAB ? STAPSK2 : STAPSK;



void setup() {
  
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // Sensors
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(A0, INPUT);
    
    WiFi.mode(WIFI_STA);    
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) { 
        delay(1000);
        ESP.restart();
    }
    ArduinoOTA.setHostname(INSIDE_LAB ? "ESP-10.0.0.109" : "ALEX-LABx");
    ArduinoOTA.begin();  

    for(int i=0;i<6;i++) Serial.write(key[i]);

    mqtt.subscribe(&sub_sensor_frequency);
    digitalWrite(LED_BUILTIN, LOW);
}



int i = 0;

int interval = 5000;
int frequency = 5000; // 1 Second
int previous_time = millis();
#define MAXBUFFERSIZE (65535)
void update_data(){
    String payload = "{ \"labels\": [";
    for(int i=0;i<10;i++){
        if(i) payload += ",";
        payload += "\"M" + String(i+1) + "\"";
    }
    payload += "], \"dataset\": [";
    for(int i=0;i<3;i++){
        if(i) payload += ",";
        payload += "{ \"name\": \"Sensor " + String(i+1) + "\", \"values\":";
        payload += convert_intr_to_json(SENSOR_VALUE[i],10);
        payload += "}";
    }
    payload += "]}";
    //pub_sensor_update.publish(payload.c_str(), payload.length());
    int pay_len = payload.length();
    int chunks = pay_len/100;
    int i;
    for(i=0; i < chunks; i++){
        String chunk = payload.substring(i*100, i*100 + 100);
        pub_sensor_update.publish(chunk.c_str());
    }
    if(i*100<pay_len) pub_sensor_update.publish(payload.substring(i*100).c_str());
    pub_sensor_update.publish("DONE");
       
}

void loop() {
    i++;

    ArduinoOTA.handle();
    MQTT_connect();

    if(millis() - previous_time > frequency){
        previous_time = millis();
        for(int i=1;i<10;i++) 
            for(int j=0;j<3;j++)
                SENSOR_VALUE[j][i-1] = SENSOR_VALUE[j][i];
     
        SENSOR_VALUE[0][9] = digitalRead(D0);
        SENSOR_VALUE[1][9] = digitalRead(D1);
        SENSOR_VALUE[2][9] = analogRead(A0);
        update_data();
    }

     if(Serial.available() > 0){
        int ch = Serial.read();
        int c = ch;
        int cmd = extract_cmd(ch);
        int sensor = extract_sensor(ch);

        if(cmd) {
            switch(cmd){
                case CMD_NODEMCU_STATUS: Serial.write('0'); break;
                case CMD_SENSOR_STATUS: Serial.write(SENSOR_STATUS[sensor]); break;
                case CMD_SENSOR_READ: Serial.write(SENSOR_VALUE[sensor][9]); break;
                case CMD_FREQUENCY: frequency = sensor*1000; Serial.write(sensor); break;
            }
        }
    }

    Adafruit_MQTT_Subscribe *subscription;
    if ((subscription = mqtt.readSubscription(0))) {
        if (subscription == &sub_sensor_frequency) {
            int f  = atoi((char *)sub_sensor_frequency.lastread);
            
            frequency = f*1000;
        }
    }
}

/**
 * Convert Int_r to Json
 * 
 * Transform an array of int, in JSON string format;
 * 
 * @return String 
 */
String convert_intr_to_json(int *arr, int n){
    String ret = "[";
    for(int i=0;i<n;i++){
        if(i) ret += ",";
        ret += String(arr[i]); 
    }
    ret += "]";
    return ret;
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  //Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       //Serial.println(mqtt.connectErrorString(ret));
       //Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  //Serial.println("MQTT Connected!");
}
// @Todos
// Convert array of values to json
// Separate files

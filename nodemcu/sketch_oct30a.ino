#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#ifndef STASSID
#define STASSID "Neverland 2G"
#define STAPSK  "eamarambp6"
//#define STASSID "INTELBRAS"
//#define STAPSK  "Pbl-Sistemas-Digitais"
#endif

#define CMD_NODEMCU_STATUS 0b001    
#define CMD_SENSOR_STATUS 0b010
#define CMD_SENSOR_READ 0b011
#define CMD_N 3

int CMD[CMD_N] = {CMD_NODEMCU_STATUS, CMD_SENSOR_STATUS, CMD_SENSOR_READ};
int SENSOR_VALUE[32] = {};
int SENSOR_STATUS[32] = {};

/**
 * Extract Cmd
 * 
 * Receive a word and extract only command part.
 *  
 * @return byte command binary
 */
byte extract_cmd(byte _word){
    byte cmd = 0;
    for(int i=0;i<3;i++) cmd |= (_word & (1<<i));
    for(int i=0;i<CMD_N;i++) if(cmd == CMD[i]) return cmd;
    return 0;
}

byte extract_sensor(byte _word){
  byte sensor = 0;
  _word = _word >> 3;
  return _word;
}
//int data[3] = {0b00001001, 0b00001010, 0b00001011};

int data[5] = {'a', 'b', 'c', 'd', 'e'};
// Ignore all messages before this.
char key[6] = {'u','n','l','o','c','k'};

const char* ssid = STASSID;
const char* password = STAPSK;

void setup() {
  
  Serial.begin(9600);
  
  // Sensors
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(A0, INPUT);
  pinMode(D0, OUTPUT);

  
  //Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("ESP-10.0.0.109");
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setHostname("ESP Net");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(D0, HIGH);
  for(int i=0;i<5;i++)
    Serial.write(data[i]);
  
  
  for(int i=0;i<6;i++) Serial.write(key[i]);
  //Serial.print("unlock");
}



int i = 0;

void loop() {
    i++;

    ArduinoOTA.handle();
    SENSOR_VALUE[0] = digitalRead(D1);
    SENSOR_VALUE[1] = digitalRead(D2);
    SENSOR_VALUE[2] = analogRead(A0);

    if(Serial.available() > 0){
        byte ch = Serial.read();
        int c = ch;
        byte cmd = extract_cmd(ch);
        byte sensor = extract_sensor(ch);
        digitalWrite(D0, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        digitalWrite(D0, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        if(cmd) {
            switch(cmd){
                case CMD_NODEMCU_STATUS: Serial.write('0'); break;
                //case CMD_SENSOR_STATUS: Serial.write('1'); break;
                //case CMD_SENSOR_READ: Serial.write('2'); break;
                case CMD_SENSOR_STATUS: Serial.write(SENSOR_STATUS[sensor]); break;
                case CMD_SENSOR_READ: Serial.write(SENSOR_VALUE[sensor]); break;
            }
        }
    }
  
}

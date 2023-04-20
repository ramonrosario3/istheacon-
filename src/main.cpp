#include <stdio.h>
#include <PubSubClient.h>
#include <driver/adc.h>
#include <WiFi.h>

// WiFi parameters and Setup Function
const char *SSID = "Rak"; // WiFi SSID
const char *PWD = "rjrr1109"; // WiFi password

float voltage_out;
float temp; 

void connectToWiFi() {
  Serial.print("Connectiog to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.");
}

// MQTT Parameter, Setup Function and callback
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 
char *mqttServer = "whatac.space"; // server IP or address
int mqttPort = 1883;

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);

  Serial.print("Done");
}

void reconnect() {
  Serial.println("Connecting to MQTT");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("Calibration");
      }  
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(4000);
  Serial.println("Hello!");

  connectToWiFi(); //initialize WiFi connection

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);

  setupMQTT(); //initialize MQTT
}

void loop() {

  // put your main code here, to run repeatedly:
if (!mqttClient.connected())
   reconnect();
   mqttClient.loop();

  float sum = 0;
  float sensor_voltage;
  int i;

  // Sensor value cleaning
  for (i = 0; i < 100; ++i)
  {
  // Read analog value 
  int sensorValue = adc1_get_raw(ADC1_CHANNEL_0);
  voltage_out = (sensorValue * 3300) / 4095;
  sum = sum + voltage_out;
  delay(10);
  }

  sensor_voltage = sum / 100;

  //calculate temp
  temp = (sensor_voltage /3.3);

  // Print temp values
  Serial.print("\ntemp: ");
  Serial.print(temp);

  char test[20];
  sprintf(test, "%.2f", temp);
  mqttClient.publish("Test", test);
  
  delay(1000);
}
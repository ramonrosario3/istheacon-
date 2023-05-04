#include <PubSubClient.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <math.h>

#define LED_BUILTIN 18
const int buttonPin = 13;

// WiFi parameters and Setup Function
const char *SSID = "";     // WiFi SSID
const char *PWD = ""; // WiFi password

float voltageOut;

float temp_c;
float temp_f;
float temp_temp = 0;

float calibration_delta = 0;

int room = 229;
int occupancy = 0;

float subscribe_msg_convert(byte *s, int length);
void callback(char *topic, byte *payload, unsigned int length);

void connectToWiFi()
{
  Serial.print("Connecting to ");

  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
   while (WiFi.status() != WL_CONNECTED)
  {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
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

void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);

  Serial.print("Done");
}

void reconnect()
{
  Serial.println("Connecting to MQTT");
  while (!mqttClient.connected())
  {
    Serial.println("Reconnecting to MQTT");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("Connected.");
      // subscribe to topic
      mqttClient.subscribe("Calibration");
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // handle message arrived
  Serial.println();
  Serial.print("Message has arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  if (!strcmp(topic, "Calibration"))
  {
    float incoming = subscribe_msg_convert(payload, length);
    Serial.println(incoming);
    calibration_delta = temp_temp - incoming;
  }
  if (!strcmp(topic, "Room"))
  {
    float incoming = subscribe_msg_convert(payload, length);
    Serial.println(incoming);
    room = incoming;
  }
}

// Takes the bytes from the subscribe and conerts it to a float
float subscribe_msg_convert(byte *s, int length)
{
  float result = 0;
  int point = 0;
  int sign = 1;

  for (int i = 0; i < length; i++)
  {
    if ((char)s[i] == '.')
    {
      break;
    }
    point++;
  }

  for (int i = 0; i < length; i++)
  {
    if ((char)s[i] == '-')
    {
      sign *= -1;
      i++;
    }

    if (i < point)
    {
      result += pow(10, point - (i + 1)) * (((char)s[i]) - 48);
    }
    else if (i > point)
    {
      result += pow(10, (point - i)) * (((char)s[i]) - 48);
    }
  }

  return sign * result;
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);

  connectToWiFi(); // initialize WiFi connection

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  setupMQTT(); // initialize MQTT
}

void loop()
{
  if (!mqttClient.connected()){
  reconnect();
  mqttClient.loop();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  }
  else{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  }

  float sum = 0;
  float sensor_voltage;
  int i;

  // Sensor value cleaning
  for (i = 0; i < 100; ++i)
  {
    // Read analog value
    int sensorValue = adc1_get_raw(ADC1_CHANNEL_0);
    voltageOut = sensorValue * 3300 / 4095;
    sum = sum + voltageOut;
    delay(10);
  }

  sensor_voltage = sum / 100;

  float error_offset = 100; // voltage offset is mV for correct temp reading

  temp_temp = ((5.506 - sqrt(pow(-5.506, 2) + 4 * 0.00174 * (870.6 - (sensor_voltage + error_offset)))) / (2 * (-0.00176))) + 30;
  temp_c = temp_temp - calibration_delta;
  temp_f = (temp_c * 1.8) + 32;


  // counter
int tempo;
if (digitalRead(buttonPin) == HIGH) {
    tempo = 0;
    occupancy++ ;
    tempo = tempo + occupancy;
    digitalRead(buttonPin) == LOW;
    delay(100); // Wait second to avoid counting multiple times
  }
  tempo = occupancy;


  // Print temperature values

  Serial.print("  temp_temp: ");
  Serial.print(temp_temp);
  Serial.print(" Temperature(ºC): ");
  Serial.print(temp_c);
  Serial.print("  Temperature(ºF): ");
  Serial.print(temp_f);
  Serial.print("  Voltage(mV): ");
  Serial.println(sensor_voltage);
  Serial.println(calibration_delta);
  Serial.print("  ocupancy: ");
  Serial.print(tempo);

  char tempPubC[20];
  sprintf(tempPubC, "%.2f", temp_c);
  mqttClient.publish("Celsius", tempPubC);

  char tempPubF[20];
  sprintf(tempPubF, "%.2f", temp_f);
  mqttClient.publish("Fahrenheit", tempPubF);

  delay(1000);
}



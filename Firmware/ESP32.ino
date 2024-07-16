#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

String regionCode = "ap-in-1";                   // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visit [https://docs.anedya.io/device/#region]
const char *deviceID = "cfe58997-c68c-4473-b08f-3b1880262626"; // Fill your device Id, that you can get from your node description
const char *connectionkey = "dd73a8b6458bb637f7521c9ba1f67388";  // Fill your connection key, that you can get from your node description

// WiFi credentials
const char *ssid = "Redmi";     // Replace with your WiFi name
const char *pass = "drupa5813"; // Replace with your WiFi password

// MQTT connection settings
const char *mqtt_broker = "mqtt.ap-in-1.anedya.io";  // MQTT broker address
const char *mqtt_username = deviceID;  // MQTT username
const char *mqtt_password = connectionkey;  // MQTT password
const int mqtt_port = 8883;  // MQTT port
String responseTopic = "$anedya/device/" + String(deviceID) + "/response";  // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(deviceID) + "/errors";  // MQTT topic for device errors

// Root CA Certificate
const char *ca_cert = R"EOF(                           
-----BEGIN CERTIFICATE-----
MIICDDCCAbOgAwIBAgITQxd3Dqj4u/74GrImxc0M4EbUvDAKBggqhkjOPQQDAjBL
MQswCQYDVQQGEwJJTjEQMA4GA1UECBMHR3VqYXJhdDEPMA0GA1UEChMGQW5lZHlh
MRkwFwYDVQQDExBBbmVkeWEgUm9vdCBDQSAzMB4XDTI0MDEwMTAwMDAwMFoXDTQz
MTIzMTIzNTk1OVowSzELMAkGA1UEBhMCSU4xEDAOBgNVBAgTB0d1amFyYXQxDzAN
BgNVBAoTBkFuZWR5YTEZMBcGA1UEAxMQQW5lZHlhIFJvb3QgQ0EgMzBZMBMGByqG
SM49AgEGCCqGSM49AwEHA0IABKsxf0vpbjShIOIGweak0/meIYS0AmXaujinCjFk
BFShcaf2MdMeYBPPFwz4p5I8KOCopgshSTUFRCXiiKwgYPKjdjB0MA8GA1UdEwEB
/wQFMAMBAf8wHQYDVR0OBBYEFNz1PBRXdRsYQNVsd3eYVNdRDcH4MB8GA1UdIwQY
MBaAFNz1PBRXdRsYQNVsd3eYVNdRDcH4MA4GA1UdDwEB/wQEAwIBhjARBgNVHSAE
CjAIMAYGBFUdIAAwCgYIKoZIzj0EAwIDRwAwRAIgR/rWSG8+L4XtFLces0JYS7bY
5NH1diiFk54/E5xmSaICIEYYbhvjrdR0GVLjoay6gFspiRZ7GtDDr9xF91WbsK0P
-----END CERTIFICATE-----
)EOF";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 19
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long duration;
float distanceCm;
long long submitTimer;   
String timeRes, submitRes;

WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

// Function Declarations
void connectToMQTT();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void setDevice_time();                                      
void anedya_submitData(String datapoint, float sensor_data); 

void setup()
{
  Serial.begin(115200);
  delay(1500);

  // Connect to WiFi network
  WiFi.begin(ssid, pass);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  submitTimer = millis();
  esp_client.setCACert(ca_cert);
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setKeepAlive(60);
  mqtt_client.setCallback(mqttCallback);
  connectToMQTT();

  mqtt_client.subscribe(responseTopic.c_str());
  mqtt_client.subscribe(errorTopic.c_str());

  setDevice_time();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop()
{
  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  mqtt_client.loop();

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * 0.034 / 2;

  Serial.print("Water Level (cm): ");
  Serial.println(distanceCm);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Water Level: ");
  display.print(distanceCm);
  display.print(" cm");

  if (distanceCm > 25) {
    display.setCursor(0, 30);
    display.print("Vessel Empty!");
    digitalWrite(BUZZER_PIN, HIGH);
  } else if (distanceCm < 5) {
    display.setCursor(0, 30);
    display.print("Vessel Full!");
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  display.display();

  anedya_submitData("water_level", distanceCm);

  delay(5000);
}

void connectToMQTT()
{
  while (!mqtt_client.connected())
  {
    const char* client_id = deviceID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password))
    {
      Serial.println("Connected to Anedya broker");
    }
    else
    {
      Serial.print("Failed to connect to Anedya broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  char res[150] = "";
  for (unsigned int i = 0; i < length; i++)
  {
    res[i] = payload[i];
  }
  String str_res(res);
  if (str_res.indexOf("deviceSendTime") != -1)
  {
    timeRes = str_res;
  }
  else
  {
    submitRes = str_res;
  }
}

void setDevice_time()
{
  String timeTopic = "$anedya/device/" + String(deviceID) + "/sendTime";
  const char *mqtt_topic = timeTopic.c_str();
  String deviceTime = "{\"id\": \"" + String(deviceID) + "\", \"sendTime\": 0}";
  const char *request = deviceTime.c_str();
  mqtt_client.publish(mqtt_topic, request);
  delay(3000);
  String _timeRes = timeRes;
  DynamicJsonDocument doc(200);
  DeserializationError err = deserializeJson(doc, _timeRes);
  if (err)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
    delay(5000);
    return;
  }
  long long _epoch = doc["sendTime"];
  setTime(_epoch);
  delay(5000);
}

void anedya_submitData(String datapoint, float sensor_data)
{
  if ((millis() - submitTimer) >= 5000)
  {
    String submitTopic = "$anedya/device/" + String(deviceID) + "/submit/json";
    const char *mqtt_topic = submitTopic.c_str();
    String submitPayload = "{\"" + datapoint + "\": " + String(sensor_data) + "}";
    const char *payload = submitPayload.c_str();
    mqtt_client.publish(mqtt_topic, payload);
    submitTimer = millis();
  }
}

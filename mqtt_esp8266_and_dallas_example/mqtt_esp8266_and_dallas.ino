#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 1 

const char* ssid = "UPC5046338";
const char* password = "Ef6Zmfdvadfj";
const char* mqtt_server = "rpi.mateuszkapala.eu";

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensor(&oneWire);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE  (50)

char msg[MSG_BUFFER_SIZE] = { 0 };
int value = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  if ((char)payload[0] == '1') 
  {
    digitalWrite(BUILTIN_LED, LOW);
  } 
  else 
  {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);  // Create a random client ID

    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      client.publish("espOutTopic", "PublishOnStart");
      client.subscribe("espInTopic");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT); 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
    //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    sprintf(msg, "Temp #%ld: %f", value, sensor.getTempCByIndex(0));
    Serial.println(msg);
    client.publish("espOutTopic", msg);
  }
}

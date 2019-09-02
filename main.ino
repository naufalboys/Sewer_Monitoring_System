#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

const char* ssid = "SSID";
const char* password = "Password";
const char* mqtt_server = "iot.eclipse.org";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const int trigPin = D4;
const int echoPin = D3;
const int water_level = A0;
const int buttonPin = D2;
byte sensorInterrupt = 0;

float calibrationFactor = 4.5;

volatile byte pulseCount;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
int value_water;

long duration;
int distance;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(100);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  oldTime = 0;

  digitalWrite(buttonPin, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin), pulseCounter, RISING); 
  
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  Serial.println();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //WiFi.begin(ssid, password);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("NodeMCU")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("001uson", "{\"id\":\"node_1\",\"waterflow\":\"%ld\",\"waterlevel\":\"%ld\",\"Distance\":\"%ld\"}");
      client.publish("002gps", "{\"id\":\"322\",\"waterflow\":\"100\",\"waterlevel\":\"20\"}");
      // ... and resubscribe
      client.subscribe("002gps");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

void waterflow()
{
  detachInterrupt(sensorInterrupt);
  flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
  oldTime = millis();
  flowMilliLitres = (flowRate / 60) * 1000;
  totalMilliLitres += flowMilliLitres;
  unsigned int frac;

  Serial.print("Flow rate: ");
  Serial.print(int(flowRate));
  Serial.print(".");

  frac = (flowRate - int(flowRate)) * 10;
  
  Serial.print(frac, DEC);
  Serial.println("L/min");
  Serial.print("  Current Liquid Flowing: ");
  Serial.print(flowMilliLitres);
  Serial.println("mL/Sec");
  Serial.print("  Output Liquid Quantity: "); // Output separator
  Serial.print(totalMilliLitres);
  Serial.println("mL");
  delay(1000); 
}

void pulseCounter() 
{
    pulseCount++;
}

void waterlevel()
{
  value_water = analogRead(water_level); 
  
  if (value_water<=480){ 
    Serial.println("Water level: 0mm - Empty!"); 
  }
  else if (value_water>480 && value_water<=530){ 
    Serial.println("Water level: 0mm to 5mm"); 
  }
  else if (value_water>530 && value_water<=615){ 
    Serial.println("Water level: 5mm to 10mm"); 
  }
  else if (value_water>615 && value_water<=660){ 
    Serial.println("Water level: 10mm to 15mm"); 
  } 
  else if (value_water>660 && value_water<=680){ 
    Serial.println("Water level: 15mm to 20mm"); 
  }
  else if (value_water>680 && value_water<=690){ 
    Serial.println("Water level: 20mm to 25mm"); 
  }
  else if (value_water>690 && value_water<=700){ 
    Serial.println("Water level: 25mm to 30mm"); 
  }
  else if (value_water>700 && value_water<=705){ 
    Serial.println("Water level: 30mm to 35mm"); 
  }
  else if (value_water>705){ 
    Serial.println("Water level: 35mm to 40mm"); 
  }
  
  delay(1000); // Check for new value every 5 sec
}

void jarak_air()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance : ");
  Serial.println(distance);
  delay(1000);
}

void loop() {

  waterflow();
  waterlevel();
  jarak_air();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    //++value;
    //snprintf (msg, 300, "{\"id\":\"node_1\",\"waterflow\":\"%ld\",\"waterlevel\":\"%ld\",\"Distance\":\"%ld\"}" , flowMilliLitres, value_water, distance);
    snprintf (msg, 300, "{\"id\":\"322\",\"waterflow\":\"%ld\",\"waterlevel\":\"%ld\"}" , flowMilliLitres, value_water);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("002gps", msg);
  }
}

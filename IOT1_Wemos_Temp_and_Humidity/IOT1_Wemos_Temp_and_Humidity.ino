#include <ESP8266WiFi.h>
#include <DHT.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DHTPIN D8 // what pin we’re connected to
String apiKey = "0G0UYIO7YWLV11M7"; //API key from ThingSpeak channel
const char* ssid = "Atree Hostel 3"; //SSID of your wifi
const char* password = "Atree#123"; //password of your wifi
int duration=5;//delay between each data measure and uploading
////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* server = "api.thingspeak.com";


DHT dht(DHTPIN, DHT11, 15);// Start DHT sensor
WiFiClient client; //Start clinet

void setup() {
  Serial.begin(115200);

  dht.begin();
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  //Get temperature and humidity data
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  if (client.connect(server, 80)) {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "\r\n\r\n";

    //Uplad the postSting with temperature and Humidity information every
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius Humidity: ");
    Serial.print(h);
    Serial.println("% send to Thingspeak");
  }
  client.stop();

  Serial.println("Waiting…");
  // thingspeak needs minimum 15 sec delay between updates
  delay(duration*1000);
}

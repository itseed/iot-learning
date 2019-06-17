#include <DHT.h>
#include <DHT_U.h>
#include <FirebaseArduino.h>
#include <WiFiManager.h>
#include <time.h>

//Define Hardware ID
const String hardwareId = "abcd";

//Define DHT Sensor
#define DHTTYPE DHT11 //DHT11 Sensor
#define DHT_DPIN 5    //GPIO-5 (D1 PIN)

//Define Relay Switch
#define SW1 15        //GPIO-15 (D8 PIN)

DHT dht(DHT_DPIN, DHTTYPE);

// Define Firebase
#define FIREBASE_HOST "YOUR HOST"
#define FIREBASE_AUTH "YOUR KEY"

int timezone = 7 * 3600;  //SET TimeZone GMP +7
int dst = 0;

unsigned long lastMillis;
int intervalTime = 10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin(); //Temperature & Humidity Start

  WiFiManager wifiManager; //WifiManager
  wifiManager.autoConnect("YOUR_AP_NAME");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.println(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("WIFI Signal = ");
  Serial.print("Wifi: " + String(WiFi.RSSI()) + "db");
  Serial.println("");

  //Get Time from server
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  
  //Firebase Start
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.stream("/hwStatus/" + hardwareId);

  pinMode(SW1, OUTPUT);
  digitalWrite(SW1, LOW);
}

void loop() {
  // put your main code here, to run repeatedly: 
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi disconnect.");
    return;
  }
  
  if (Firebase.failed()) {
    Serial.println("streaming error");
    Serial.println(Firebase.error());
  }

  if (Firebase.available()) {
    FirebaseObject event = Firebase.readEvent();
    String eventType = event.getString("type");
    eventType.toLowerCase();

    Serial.print("event: ");
    Serial.println(eventType);
    if (eventType == "put") {
      Serial.print("data: ");
      Serial.println(event.getString("path") + " " + event.getInt("data"));
      String path = event.getString("path");
      int data = event.getInt("data");
      
      if (path == "/switch") {
        if (data == 1) {
          digitalWrite(SW1, HIGH);
        }

        if (data == 0) {
          digitalWrite(SW1, LOW);
        }
      }
    }
  }

  if (millis() - lastMillis > ((1000*60) * intervalTime)) {
    lastMillis = millis();
    float h = 0.0;
    float t = 0.0;
    float f = 0.0;
    
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = (t * 1.8) + 32;
    
    Serial.print("WIFI Signal = ");
    Serial.print("Wifi: " + String(WiFi.RSSI()) + "db");
    Serial.println("");
    Serial.print("Temperature in Celsius = ");
    Serial.print(t);
    Serial.println("");
    Serial.print("Temperature in Fahrenheit = ");
    Serial.print(f);
    Serial.println("");
    Serial.print("Humidity = ");
    Serial.print(h);
    Serial.print("%");
    Serial.println("");

    Firebase.setFloat("hwStatus/" + hardwareId + "/humidity", h);
    Firebase.setFloat("hwStatus/" + hardwareId + "/temperature", t);
  }
  delay(1000);
}

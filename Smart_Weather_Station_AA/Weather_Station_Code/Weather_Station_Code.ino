/*
Sketch to submit readings from an Arduino Ethernet to ThingSpeak
Billy Abbott 

*/

#include <SPI.h>
#include <Ethernet.h>
#include <dht.h>
#include <math.h>
#define WindSensorPin (2)

// MAC address for the Arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };



void setup() {
  Serial.begin(9600);

  // Start ethernet
  Serial.println("Getting IP");
  Ethernet.begin(mac);
  Serial.println(Ethernet.localIP());
  
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);
  pinMode(WindSensorPin, INPUT);
  
}

// DHT sensor setup
#define dht_dpin 8
#define WindSensorPin (2) // The pin location of the anemometer sensor
dht DHT;

// DHT & Barometric data
int temperature=0;
int humidity=0;

int pressureValue;
float pressure;

int sensorValue;
float sensor;

float WindSpeed; // speed miles per hour
volatile unsigned long Rotations;
volatile unsigned long ContactBounceTime;

// ThingSpeak data
char server[]  = "api.thingspeak.com";      // IP Address for the ThingSpeak API
String apiKey = "2X9XBGSQ5HIZ06FJ";  // Write API Key for a ThingSpeak Channel

// Check variables
long lastCheck=0;
const int updateInterval = 10000;           // Time interval in milliseconds to update ThingSpeak   

// Our ethernet client
EthernetClient client;

void loop() {
  
  long now=millis();
  if (now-lastCheck>updateInterval) {

    // read data from the DHT
    DHT.read11(dht_dpin);
    temperature=DHT.temperature;
    humidity=DHT.humidity;
    pressureValue = analogRead (A1);
    pressure = ((pressureValue / 1024.0)  + 0.095) /0.0009;
    sensorValue = analogRead(A0);
    sensor =(sensorValue);

    
    Rotations = 0; // Set Rotations count to 0 ready for calculations
    sei(); // Enables interrupts
   
    delay(10000);
    WindSpeed = Rotations * 1.207; // converted MPH to KM/H
    lastCheck=now;
    
   
   
    
    // build data strings/
    String tempData="field1=";
    String humData="field2=";
    String pressure="field3=";
    String Rotations="field4=";
    String sensor="field5=";
    
    tempData+=temperature;
    humData+=humidity;
    pressure+=pressureValue;
    Rotations+=WindSpeed;
    sensor+=sensorValue;
    
    Serial.print("\nTemperature: ");
    Serial.println(tempData);

    Serial.print("\nHumidity: ");
    Serial.println(humData);

    Serial.print("\nAir Pressure: ");
    Serial.println(pressure);

    Serial.print("\nRotations: ");
    Serial.println(Rotations);

    Serial.print("\nRaindrops: ");
    Serial.println(sensor);
    
    
    //connect to ThingSpeak and write the data
    if (client.connect(server,80)) {
      Serial.print("\n\nConnected: ");
      Serial.println(server);
      
      client.println("POST /update HTTP/1.1");
      client.println("Host: api.thingspeak.com");
      client.println("Connection: close");
      client.println("X-THINGSPEAKAPIKEY: "+apiKey);
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(tempData.length()+humData.length()+pressure.length()+sensor.length()+Rotations.length()+1); // need to include the & seperator
      client.println("");
     
      client.print(tempData);
      client.print("&"); 
      client.print(humData); 
      client.print("&"); 
      client.print(pressure); 
      client.print("&"); 
      client.print(sensor); 
      client.print("&");
      client.print(Rotations);
      
      

      client.stop();
    } else {
      Serial.println("Failed to connect"); 
    }
  }
}
void isr_rotation () {
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}


 

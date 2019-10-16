

/*

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include "DHTesp.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iostream>
#include <string>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;
const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
//const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
//DHT dht(DHTPIN, DHTTYPE);
const int personal_pin = 17;
const int rain_pin = 36;
const int rele = 25;


 
std::string rxValue;


#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
DHTesp dht;
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();
        }
        Serial.println();
        Serial.println("*********");
      }
    
};

void setup() {
  
  dht.setup(27, DHTesp::DHT22);
   //pinMode(LED, OUTPUT);
   pinMode(rele, OUTPUT);
   pinMode(personal_pin, INPUT);
   pinMode(rain_pin,INPUT);
   Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Micro app test"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Fabricate some arbitrary junk for now...
     TempAndHumidity newValues = dht.getTempAndHumidity();
     int rain_value = (analogRead(rain_pin)/4);
     Serial.println(rain_value);
      Serial.print("Temperatura: ");
      Serial.print(newValues.temperature);
      Serial.println(" *C");
      Serial.print("Umidade: ");
      Serial.print(newValues.humidity);
      if (rxValue.find('A') != -1) { 
      Serial.println("Turning ON!");
      digitalWrite(rele, LOW);
      Serial.println(digitalRead(25));
    }
     if (rxValue.find('B') != -1){
     Serial.println("Turning OFF!");
     digitalWrite(rele, HIGH);
     Serial.println(digitalRead(25));
    }
    char humidityString[4];
    char temperatureString[4];
    char rainString[4];
    char movieString[2];
    bool isDetected = digitalRead(personal_pin);
    dtostrf(newValues.humidity, 4, 2, humidityString);
    dtostrf(newValues.temperature, 4, 2, temperatureString);
    dtostrf(isDetected, 2, 4, movieString);
    dtostrf(rain_value, 2, 4, rainString);
    char SendDataString[100];
    sprintf(SendDataString,"%2.f,%2.f,%d,%d", newValues.temperature, newValues.humidity,isDetected,rain_value);
    //sprintf(dhtDataString, "%f", temperature);
    pCharacteristic->setValue(SendDataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print("*** Dado enviado: ");
    if(rain_value<=500){
      digitalWrite(25,LOW);
    }
    if(rain_value>=800 || newValues.temperature>40){
      digitalWrite(25,HIGH);
    }
    Serial.print(SendDataString);
    Serial.println(" ***");
  }
    delay(1000);
}

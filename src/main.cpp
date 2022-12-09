#include <Arduino.h>


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"

int tempValue;
String tempValueString;

//#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
//#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define SERVICE_UUID        "12345678-9abc-def0-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-9abc-def0-1234-56789abcdef1"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

boolean deviceConnected = false;
boolean oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic){
    std::string value = pCharacteristic->getValue();
    Serial.println("MyCallback");
  }
};

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting BLE Server!");

  BLEDevice::init("ESP32-BLE-Server");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pServer->setCallbacks(new MyServerCallbacks());
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_INDICATE
                                       );

  pCharacteristic->setValue("0");
  
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  //pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in the Client!");



}

void loop()
{
  /*  
  std::string value = pCharacteristic->getValue();
  //Serial.print("The new characteristic value is: ");
  //Serial.println(value.c_str());

  if (deviceConnected) { 
    //Serial.print("NOTIFY: ");
    //Serial.println(value.c_str());
    pCharacteristic->setValue(value); 
    pCharacteristic->notify(); 
    //pCharacteristic->indicate();
  }
  delay(1000);
  */

  // notify changed value
  if (deviceConnected) {
      std::string value = pCharacteristic->getValue();
      pCharacteristic->setValue(value);
      Serial.print("NOTIFY: ");
      Serial.println(value.c_str());
      pCharacteristic->notify();
      delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }

  if (!deviceConnected && oldDeviceConnected) {
        delay(1000); // give the bluetooth stack the chance to get things ready
        BLEDevice::startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting 
      oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}

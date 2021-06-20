#include <Arduino.h>
#include "Boat_MPU6050.h"

Boat_MPU6050 boat_MPU6050;

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ESP32Servo.h"
 

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;

//**************
// ESC LOGIC
//**************

class ESCMaderController {
  private:
    byte pinArray[2] = { // The pins used as output
      12, // motor 1 
      13, // motor 2 
    };
    int pinCount = 2; // Pins uses in array
	Servo rightThruster;
    Servo leftThruster;
   
  public:
    ESCMaderController() {
		delay(3000);
        rightThruster.attach(pinArray[0]);
        leftThruster.attach(pinArray[1]);
        rightThruster.writeMicroseconds(1500);
        leftThruster.writeMicroseconds(1500);
		delay(7000);
    }

    void drive(int right_power, int left_power, int drive_duration) { //Driveing the pins off of the input of x.
      Serial.println("driving thrusters start");
      Serial.println(right_power);
      Serial.println(left_power);
        for (int i = 0; i < drive_duration * 100; i++)
        {
			rightThruster.writeMicroseconds(right_power);
			leftThruster.writeMicroseconds(left_power);
			delay(10);
		}
		Serial.println("driving thrusters finish");
	}
};

ESCMaderController motor_controller;

int serial_state = 53;
int state = 2;

void types(String a) { Serial.println("it's a String"); }
void types(int a) { Serial.println("it's an int"); }
void types(char *a) { Serial.println("it's a char*"); }
void types(float a) { Serial.println("it's a float"); }
void types(bool a) { Serial.println("it's a bool"); }

void debugRxValue(std::string rxValue){
    Serial.println("*********");
    Serial.println("Received Value: ");
    for (int i = 0; i < rxValue.length(); i++){
      Serial.print(rxValue[i]);
      types(rxValue[i]);
      Serial.println("-");
      if (rxValue[i] == 1){
        Serial.println("current value is 1!");
      }
      if (rxValue[i] == 49){
        Serial.println("current value is 49!");
      }
    }
    Serial.println();
    Serial.println("*********");
}

void processRxValue(std::string rxValue){
    if (rxValue.length() > 0) {
      debugRxValue(rxValue);   
      serial_state = rxValue[0]; // state is askii!
        switch (serial_state) {
          case 49:  motor_controller.drive(1800, 1800, 10); break;
          case 50:  motor_controller.drive(1200, 1200, 10); break;
          case 51:  motor_controller.drive(1500, 1800, 5); break;
          case 52:  motor_controller.drive(1800, 1500, 5); break;
          case 53:  motor_controller.drive(1500, 1500, 5); break;
          default:  state = 4;        
          }
     }
}

//***************
// BLUETOOTH 
//***************

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


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
      std::string rxValue = pCharacteristic->getValue();
      processRxValue(rxValue);
    }
};

void setup() {
  Serial.begin(9600);
  Serial.println("test");

  pinMode(27, OUTPUT);

  boat_MPU6050.begin();

  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

long step_boat = 0;
int step_MPU6050 = 100;
int step_data = 10000;
float data[9];

int battery_pin = 15;
float splitted_battery_value;
float real_battery_value;


void loop() {

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    if (step_boat % step_MPU6050 == 0){
      boat_MPU6050.step();
    }
  	if (step_boat % step_data == 0 && deviceConnected){
		boat_MPU6050.data(data);
		Serial.println("sending bluetooth data..");
    /*
		Current bluetooth notify protocol send the following little-endian floats
		accelleration-x
		accelleration-y
		accelleration-z
		gyro-x
		gyro-y
		gyro-z
		angle-x
		angle-y
		temperature
    battery level
    */
		for (int i=0; i<9; i++){
		  Serial.println(data[i]);
		  pCharacteristic->setValue(data[i]);
		  pCharacteristic->notify();
		  delay(50);
		}  

    splitted_battery_value = analogRead(battery_pin);
    // 0.18 * dc power the real voltaage
    // actual 0.15
    Serial.println(splitted_battery_value);
    real_battery_value = splitted_battery_value / 0.1670;
    real_battery_value = real_battery_value / 1000;
    Serial.println(real_battery_value);
    pCharacteristic->setValue(real_battery_value);
    pCharacteristic->notify();
    delay(50);
  	}
  	step_boat += 1;
    if (step_boat == 100000){
      step_boat = 0;
    }
    delay(1);
}

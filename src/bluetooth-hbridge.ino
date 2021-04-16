#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
 #include <Arduino.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;

//**************
// HBRIDGE LOGIC
//**************

int Pwr = 3;
const int Low = 127;
const int Med = 191;
const int High = 255;

class HBridgeMaderController {
  private:
    int pinArray[4] = { // The pins used as output
      13, // motor 1 pin A
      12, // motor 1 pin B
      14, // motor 2 pin A
      27  // motor 2 pin B
    };
    int direction[5][4] = {
      {1, 0, 1, 0},    // Forward =0
      {0, 1, 0, 1},    // Backwords =1
      {1, 0, 0, 0},    // turn left =3
      {0, 0, 1, 0},    // Turn right = 4
      {0, 0, 0, 0},    // Standby =2
    };
    int pinCount = 4; // Pins uses in array
   
  public:
    HBridgeMaderController() {
      for (int count = 0; count <= pinCount; count++)
      {
          ledcSetup(count, 5000, 8);
          ledcAttachPin(this->pinArray[count], count);
          ledcWrite(this->pinArray[count], 0);
      }
    }

    void drive(int x, int drive_pwr) { //Driveing the pins off of the input of x.
        for (int i = 0; i < this->pinCount; i++)
        {
            if (this->direction[x][i] == 1)
            {
                ledcWrite(i, drive_pwr);
            }
            else
            {
                ledcWrite(i, 0);
            }
        }
    }
};

HBridgeMaderController motor_controller;

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
      //debugRxValue(rxValue);   
      serial_state = rxValue[0]; // state is askii!
      if(serial_state > 53 && serial_state <= 56){ // power setting switch
        switch (serial_state) {
          case 54:  Serial.println("serial_state Low");   Pwr = Low;  break;
          case 55:  Serial.println("serial_state Med");   Pwr = Med;  break;
          case 56:  Serial.println("serial_state High");  Pwr = High; break;
          default:  Serial.println("serial_state High");  Pwr = High;       }
      } else if(serial_state > 48 && serial_state <= 53){ // motor direction switch
        switch (serial_state) {
          case 49:  Serial.println("serial_state Forward");  state = 0;  break;
          case 50:  Serial.println("serial_state Back");     state = 1;  break;
          case 51:  Serial.println("serial_state Left");     state = 2;  break;
          case 52:  Serial.println("serial_state Right");    state = 3;  break;
          case 53:  Serial.println("serial_state STOPED");   state = 4;  break;
          default:  state = 4;        }
      }
      motor_controller.drive(state, Pwr);
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
  Serial.begin(115200);

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

void loop() {
    // notify changed value
    if (deviceConnected) {
        //pCharacteristic->setValue(&value, 1);
        //pCharacteristic->notify();
       // value++;
       // delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
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
}


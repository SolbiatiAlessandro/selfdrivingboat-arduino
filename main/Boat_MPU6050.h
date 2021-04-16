/* theselfdrivingboat.com | module to read gyro, acc, temp. example usage:

#include "Boat_MPU6050.h"

Boat_MPU6050 boat_MPU6050;

void setup()
{
  boat_MPU6050.begin();
}

int cnt = 0;
int cnt_data = 100;
int MPU6050_data[9];

void loop()
{
	boat_MPU6050.step();
	if (cnt % cnt_data == 0){
		boat_MPU6050.data(MPU6050_data);
    Serial.printf("%d", MPU6050_data[0]);
	}
  cnt == cnt + 1;
}

*/

#ifndef Boat_MPU6050_h
#define Boat_MPU6050_h

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "Arduino.h"

class Boat_MPU6050
{
  public:
    Boat_MPU6050();
    void begin();
    void step();
    int *data(int data_array[9]);

  private:
  	Adafruit_MPU6050 mpu;
  
  	float anglex = 0;
  	float angley = 0;
  	long current_time = millis();
  	long old_time = 0;
  
  	int print_r = 50;
  	int print_c = 1;
  	float max_values[6] = {0, 0, 0, 0, 0, 0};
  	int _low_sens_gyro_x;
  	float low_sens_gyro_x;
  	int _low_sens_gyro_y;
  	float low_sens_gyro_y;
  	sensors_event_t a, g, temp;
};

#endif

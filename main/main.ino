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

// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

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

void loop() {
  

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  max_values[0] = max(max_values[0], a.acceleration.x);
  max_values[1] = max(max_values[1], a.acceleration.y);
  max_values[2] = max(max_values[2], a.acceleration.z);
  max_values[3] = max(max_values[3], g.gyro.x);
  max_values[4] = max(max_values[4], g.gyro.y);
  max_values[5] = max(max_values[5], g.gyro.z);
  
  _low_sens_gyro_x = g.gyro.x * 10;
  low_sens_gyro_x = (float) _low_sens_gyro_x/10;
  _low_sens_gyro_y = g.gyro.y * 10;
  low_sens_gyro_y = (float) _low_sens_gyro_y/10;

  old_time = current_time;
  current_time = millis();
  anglex = anglex + (low_sens_gyro_x * (current_time - old_time) / 1000);
  angley = angley + (low_sens_gyro_y * (current_time - old_time) / 1000);
  

  if (print_c % print_r == 0) {
    Serial.printf("%f %f %f %f %f %f %f %f %f", 
    max_values[0],
    max_values[1],
    max_values[2],
    max_values[3],
    max_values[4],
    max_values[5],
    anglex,
    angley,
    temp.temperature
    );
    Serial.println("");

    max_values[0] = 0;
    max_values[1] = 0;
    max_values[2] = 0;
    max_values[3] = 0;
    max_values[4] = 0;
    max_values[5] = 0;
  }
  print_c = print_c + 1;

  delay(5);
}

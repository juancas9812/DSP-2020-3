#include <Wire.h>
#include <L3G.h>

L3G gyro;
volatile unsigned long actual;
volatile unsigned long previous;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  if (!gyro.init()){
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }
  gyro.enableDefault();
  Serial.print("X,");
  Serial.print("Y,");
  Serial.print("Z,");
  Serial.println("time");
  //for(int i=0;i<1000;i++){      //Prueba para leer datos desde el computador con Python
  //  Serial.print(i);
  //  Serial.print(",");
   // Serial.print(i);
   // Serial.print(",");
   // Serial.println(i);
  //}
  actual=micros();
  previous=micros();
}

void loop() {
  gyro.read();
  Serial.print((int)gyro.g.x);
  Serial.print(",");
  Serial.print((int)gyro.g.y);
  Serial.print(",");
  Serial.print((int)gyro.g.z);
  Serial.print(",");
  actual = micros();
  Serial.println(actual-previous);
  previous=actual;
}

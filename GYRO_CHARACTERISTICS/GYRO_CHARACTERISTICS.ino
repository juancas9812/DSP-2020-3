/*
The sensor outputs provided by the library are the raw 16-bit values
obtained by concatenating the 8-bit high and low gyro data registers.
They can be converted to units of dps (degrees per second) using the
conversion factors specified in the datasheet for your particular
device and full scale setting (gain).

Example: An L3GD20H gives a gyro X axis reading of 345 with its
default full scale setting of +/- 245 dps. The So specification
in the L3GD20H datasheet (page 10) states a conversion factor of 8.75
mdps/LSB (least significant bit) at this FS setting, so the raw
reading of 345 corresponds to 345 * 8.75 = 3020 mdps = 3.02 dps.
*/

#include <Wire.h>                        //Se incluye la librería para usar la comunicacion I2C
#include <L3G.h>                         //Se incluye la libreria de Pololu para usar el sensor L3GD20

#define STATUS_MASK 0b00001000           //Máscara para extraer el bit ZYXDA del STATUS_REG

const int L3GD20_Address = 106;          //Dirección del giroscopo
L3G gyro;                                //Crea un objeto de tipo L3G para usar el giroscopo
uint32_t ts1=0;                          //Para realizar medición de tiempo entre las muestras
uint32_t ts2=0;                          //Para realizar medición de tiempo entre las muestras
byte stats;                              //Para guardar el valor que se lee del STATUS_REG
byte stats_reg = 0x27;                   //Dirección del STATUS_REG
int sens = 70;                           //La sensitividad cuando se tiene escala de ±2000dps es de 70mdps/digit, cambiar si se utiliza otra escala
int X,Y,Z = 0;                           //Para guardar los valores en X, Y y Z

void setup() {
  Serial.begin(115200);                  //Se habilita el serial a máxima velocidad
  Wire.begin();                          //Se habilita la comunicacion I2C
  Wire.setClock(3400000);                //Se cambia la frecuencia del SCL a la máxima velocidad (3400000)
  
  if (!gyro.init()){                     //Se verifica que se haya inicializado y detectado el giroscopio
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }
  
  gyro.enableDefault();                  //Se ponen los valores default de los registros del giroscopo según la libreria de Pololu
  escribirReg(0x20, 0xEF);               //Se escribe en el CTRL_REG1 que la frecuencia de muestreo (ODR) sea de 760Hz
  escribirReg(0x23, 0x20);               //Se escribe en el CTRL_REG4 que la escala es de 2000dps
}

void loop() {
  stats = leerReg(stats_reg);            //Leer registro STATUS_REG del gyro
  stats = stats & STATUS_MASK;           //Revisar si el bit de ZYXDA está en 1 o 0.
  if (int(stats) == 8){                  //Si ZYXDA esta en 1, hay un nuevo dato
    ts2 = micros();                      //Se mide el tiempo que duró hasta obtener el nuevo dato
    gyro.read();                         //Se leen los registros de los valores para X,Y y Z
    X = (int(gyro.g.x) * sens) / 1000;   //Se convierte el valor del eje X de RAW a dps
    Y = (int(gyro.g.y) * sens) / 1000;   //Se convierte el valor del eje Y de RAW a dps
    Z = (int(gyro.g.z) * sens) / 1000;   //Se convierte el valor del eje Z de RAW a dps
    Serial.print(X);                     //Se imprimen los valores para los 3 ejes en dps (separado en comas para capturar los datos en python)
    Serial.print(",");
    Serial.print(Y);
    Serial.print(",");
    Serial.print(Z);
    Serial.print(",");
    Serial.println(ts2-ts1);             //Se imprime el tiempo entre las muestras en µs
    ts1 = ts2;                           //Se guarda el tiempo anterior para poder volver a medir el tiempo entre muestras
  }
}

void escribirReg(byte reg, byte value){  //Función para escribir un valor en un registro del sensor L3GD20
  Wire.beginTransmission(L3GD20_Address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

byte leerReg(byte regis){                //Función para leer de un registro del sensor L3GD20
  byte value;
  Wire.beginTransmission(L3GD20_Address);
  Wire.write(regis);
  Wire.endTransmission();
  Wire.requestFrom(L3GD20_Address, 1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

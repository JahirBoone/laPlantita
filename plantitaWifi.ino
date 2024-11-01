#include "arduino_secrets.h"
#include "DHT.h"
#include "Wire.h"
#include <ph4502c_sensor.h>
#include "SoftwareSerial.h"
#define DHTPIN 3
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
PH4502C_Sensor ph4502c(A2, A3);
const int pinHumedad = A0;
const int pinSol = A1;
const int pinpHvolt = 7;

char nadv[] = "0000";

#include "thingProperties.h"
void setup() {
  // Initialize serial and wait for port to open:
  dht.begin();
  Wire.begin();
  ph4502c.init();
  Serial.begin(9600);
  pinMode(pinpHvolt, OUTPUT);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  // Your code here 
  char* nadv2 = monitoreo();
  int humedad = nadv2[0] - '0';
  int pH = nadv2[1] - '0';
  int VPD = nadv2[2] - '0';
  int sol = nadv2[3] - '0';

  delay(1000);
}


char* monitoreo() {
  float temperatura, HR;
  sensorHumedadAmbiental(temperatura, HR);
  float humedadSuelo = sensorHumedadSuelo();
  float solar = sensorLDR();
  int etapaPlanta = 0;  //etapa 0: germinacion y plantulas, 1: fase vegetativa, 2: floracion, 3: fructificacion, 4: maduracion de los frutos
  float VPD = lvlVPD(temperatura, HR);
  float lvlpH = sensorPH();

  //datos enviados a la nube
  humedadSueloCloud = humedadSuelo;
  lvlpHCloud = lvlpH;
  solarCloud = solar;
  temperaturaCloud = temperatura;
  vpdCloud = VPD;
  nadvCloud = nadv;
  //revisar el nivel de humedad en el suelo para verificar que la planta tenga suficiente agua
  if (humedadSuelo >= 63 && humedadSuelo <= 83) {
    //suelo en condiciones ideales
    nadv[0] = '0';
  }
  if (humedadSuelo <= 62) {
    //suelo seco
    nadv[0] = '1';
  }
  if (humedadSuelo >= 84) {
    //suelo encharcado
    nadv[0] = '2';
  }

  if (lvlpH >= 5.9 && lvlpH <= 6.9) {
    //ph correcto
    nadv[1] = '0';
  }
  if (lvlpH <= 5.8) {
    //ph bajo
    nadv[1] = '1';
  }
  if (lvlpH >= 7.0) {
    //ph alto
    nadv[1] = '2';
  }

  switch (etapaPlanta) {
    //etapa 1 de la planta
    case 0:
      if (VPD >= 0.4 && VPD <= 0.8) {
        //VPD en condiciones optimas
        nadv[2] = '0';
      } else {
        if (VPD <= 0.3) {
          //vpd bajo
          nadv[2] = '1';
        }
        if (VPD >= 0.9) {
          //vpd alto
          nadv[2] = '2';
        }
      }
      break;
    //etapa 2 de la planta
    case 1:
      if (VPD >= 0.9 && VPD <= 1.2) {
        //vpd en condiciones optimas
        nadv[2] = '0';
      } else {
        if (VPD <= 1.0) {
          //vpd bajo
          nadv[2] = '1';
        }
        if (VPD >= 1.3) {
          //vpd alto
          nadv[2] = '2';
        }
      }
      break;
    //etapa 3 de la planta
    case 2:
      if (VPD >= 0.6 && VPD <= 1.0) {
        //VPD en condiciones optimas
        nadv[2] = '0';
      } else {
        if (VPD <= 0.5) {
          //vpd bajo
          nadv[2] = '1';
        }
        if (VPD >= 1.1) {
          //VPD alto
          nadv[2] = '2';
        }
      }
      break;
    //etapa 4 de la planta
    case 3:
      if (VPD >= 0.8 && VPD <= 1.2) {
        //VPD en condiciones optimas
        nadv[2] = '0';
      } else {
        if (VPD <= 0.7) {
          //VPD bajo
          nadv[2] = '1';
        }
        if (VPD >= 1.3) {
          //VPD alto
          nadv[2] = '2';
        }
      }
      break;
  }
  if (solar >= 78 && solar <= 87) {
    //luz solar optima
    nadv[3] = '0';
  }
  if (solar <= 77) {
    //luz solar bajo
    nadv[3] = '1';
  }
  if (solar >= 88){
    //luz solar alto
    nadv[3] = '2';
  }
  Serial.print("La temperatura es: ");
  Serial.println(temperatura);
  Serial.print("La humedad relativa es: ");
  Serial.println(HR);
  Serial.print("La humedad del suelo es: ");
  Serial.println(humedadSuelo);
  Serial.print("El nivel de luz solar es: ");
  Serial.println(solar);
  Serial.print("El nivel de VPD es: ");
  Serial.println(VPD);
  Serial.print("El nivel de pH es: ");
  Serial.println(lvlpH);

  return (nadv);
}

//codigo para el sensor de humedad
float sensorHumedadSuelo() {
  //entre mas cercano al 100 mas seco esta el sensor, entre mas cercano a 0 mas humedo esta el sensor
  int humedad = analogRead(pinHumedad);

  //convertir el dato de humedad en un porcentaje
  float porcentajeHumedad = 100.0 - (float)humedad / 1023.0 * 100;

  return porcentajeHumedad;
}

float sensorHumedadAmbiental(float& temperatura, float& HR) {
  HR = dht.readHumidity();
  temperatura = dht.readTemperature();
}

float sensorLDR() {
  int lecturaLDR = analogRead(pinSol);  // Leer el valor de la fotoresistencia
  /*float voltaje = (lecturaLDR / 1023.0) * 5.0; // Convertir la lectura a voltaje
  float intensidadLuz = map(voltaje, 0, 5, 0, 100); // Mapear a un rango de 0 a 100 (porcentaje)*/
  float intensidadLuz = (float)lecturaLDR / 1023.0 * 100;
  float refDark = 0.0;
  float refWhite = 12.0;  //tomar una referencia con un dia completamente soleado

  return intensidadLuz; //cambiar a la variable lecturaLDR
}

float lvlVPD(float temperatura, float HR) {
  float SPV = 0.61078 * exp((17.27 * temperatura) / (temperatura + 237.3));  // Calcular la presión de vapor saturada
  float VPA = (HR / 100) * SPV;                                              // Calcular la presión de vapor actual
  float VPD = SPV - VPA;                                                     // Calcular el VPD
  return VPD;
}

float buffpH[10];
float sensorPH() {
  digitalWrite(pinpHvolt, HIGH);
  delay(100);
  
  //almacenar en una matriz el valor del sensor del ph para luego promediarlo y tener una lectura mas precisa
  for(int i = 0; i <= 9; i++){
    buffpH[i] = ph4502c.read_ph_level();
    delay(10);
  }

  //promediar el valor de buffpH[10]
  float prompH;
  for(int i = 0; i <= 9; i++){
    prompH = prompH + buffpH[i];
  }

  prompH = prompH / 10;

  digitalWrite(pinpHvolt, LOW);

  return prompH;
}




















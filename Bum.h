// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef Bum_H_
#define Bum_H_
#include "Arduino.h"
//add your includes for the project Bum here
#include "Wire.h"
#include "Adafruit_L3GD20.h"
#include "LSM303.h"

//end of add your includes here
struct LeituraAcelerometro {
	float x, y, z;
};

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project Bum here

void log(int x);
void logln(int x);
void log(String x);
void logln(String x);

LeituraAcelerometro lerAcelerometro();
int obterAnguloInclinacao();
void parar();
void sobeEDesceProsDoisLados();
void lerComandoConfiguracao();



//Do not add code below this line
#endif /* Bum_H_ */

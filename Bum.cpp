#include "Bum.h"

const String AA = String("AA");
const String AM = String("AM");
const String PG = String("PG");
const String PM = String("PM");
const String CF = String("CF");
const String LG = String("LG");
const String LP = String("LP");
const String SIM = String("SIM");
const String NAO = String("NAO");

const byte BHI = 7;
const byte disable = 8;
const byte BLI = 3;
const byte ALI = 11;
const byte AHI = 13;
const byte BOTAO_PE = 12;

Adafruit_L3GD20 gyro;
LSM303 accelCompass;
int estadoBotaoPe = 0;
long previousMillis = 0;
float angle = 0.0;
char* buffer;
bool logar = false;

// Variáveis configuráveis
int anguloAjuste = 4000;
int anguloMaximo = 15000;
byte potenciaMaxima = 255;
byte limitadorPotencia = 150;
double pesoGiroscopio = 0.99357;

void setup() {
	Serial.begin(9600);
	Serial.println("Inicializando skate... ");
	buffer = (char *) malloc(6);
	buffer[5] = NULL;

	Wire.begin();
	if (!gyro.begin(gyro.L3DS20_RANGE_2000DPS)) { // Erro inicializando o giroscópio
		Serial.println("Erro ao inicializar o giroscópio ");
		while (1)
			;
	}

	accelCompass.init();
	accelCompass.enableDefault();
	accelCompass.writeAccReg(accelCompass.CTRL_REG4_A, 0x30); // Faz o acelerômetro funcionar na faixa de -8G a +8G

	TCCR2B = (TCCR2B & B11111000) | B00000001;
	// Bota a frequência dos pinos 3 e 11 (timer 2) em 31372.55 Hz

	pinMode(AHI, OUTPUT);
	pinMode(BHI, OUTPUT);
	pinMode(ALI, OUTPUT);
	pinMode(BLI, OUTPUT);
	pinMode(disable, OUTPUT);
	pinMode(BOTAO_PE, INPUT);
}
long contador = 0;
long tempoAnterior = millis();
void loop() {
	lerComandoConfiguracao();
	estadoBotaoPe = digitalRead(BOTAO_PE);
	int angulo = obterAnguloInclinacao();
	log(" Angulo: ");
	logln(angulo);

	if (estadoBotaoPe == LOW) {
		parar();
	} else if (angulo > -1000 && angulo < 1000 && estadoBotaoPe == HIGH) {
		while (estadoBotaoPe == HIGH) {
			angulo = obterAnguloInclinacao();
			log(" Angulo: ");
			logln(angulo);
			if (angulo > 1) {
				digitalWrite(disable, LOW);
				digitalWrite(AHI, HIGH);
				digitalWrite(BHI, HIGH);
				if (angulo > anguloMaximo) {
					angulo = anguloMaximo;
				}
				byte potencia = map(angulo, 1, anguloMaximo, 0, potenciaMaxima);
				if (potencia > limitadorPotencia) {
					potencia = limitadorPotencia;
				}
				analogWrite(ALI, potencia);
				digitalWrite(BLI, LOW);
			} else if (angulo < -1) {
				digitalWrite(disable, LOW);
				digitalWrite(AHI, HIGH);
				digitalWrite(BHI, HIGH);
				if (angulo < -anguloMaximo) {
					angulo = -anguloMaximo;
				}
				angulo = abs(angulo);
				byte potencia = map(angulo, 1, anguloMaximo, 0, potenciaMaxima);
				if (potencia > limitadorPotencia) {
					potencia = limitadorPotencia;
				}
				analogWrite(BLI, potencia);
				digitalWrite(ALI, LOW);
			} else {
				parar();
			}
			if (contador++ % 10000 == 0){
				long tempoAtual = millis();
				long duracao = tempoAtual - tempoAnterior;
				double duracaoSegundos = ((double) duracao) / 1000;
				double vezes = 10000.0 / duracaoSegundos; // iterações por segundo
				Serial.print("Duracao em segundos: ");
				Serial.println(duracaoSegundos);
				Serial.print("Iterações por segundo: ");
				Serial.println(vezes);
				tempoAnterior = tempoAtual;
			}
			estadoBotaoPe = digitalRead(BOTAO_PE);
		}
	}
}

void parar() {
	digitalWrite(disable, LOW);
	digitalWrite(AHI, LOW);
	digitalWrite(BHI, LOW);
	digitalWrite(ALI, LOW);
	digitalWrite(BLI, LOW);
}

int obterAnguloInclinacao() {
	gyro.read();
	float y = gyro.data.y;
	LeituraAcelerometro reading = lerAcelerometro();
	unsigned long currentMillis = millis();
	double dt = currentMillis - previousMillis;
	dt = dt / 1000.0;
	previousMillis = currentMillis;
	double pesoAcelerometro = 1 - pesoGiroscopio;
	if (estadoBotaoPe == HIGH) {
		angle = (pesoGiroscopio) * (angle + y * dt)
				+ (pesoAcelerometro) * (reading.x);
	} else { // Caso esteja parado, confia puramente no
			 // acelerômetro para ter uma leitura rápida da angulação
		angle = reading.x;
	}
//	Serial.print("Giroscopio: ");
//	Serial.print(y);
//	Serial.print(" Acelerometro: ");
//	Serial.print(reading.x);
//	Serial.print(" Angulo: ");
//	Serial.print(angle);
//	Serial.print(" BotaoPe: ");
//	Serial.println(estadoBotaoPe);
	int anguloBom = (int) (angle * 1000);
	anguloBom += anguloAjuste;
	return anguloBom;
}

float angle = 0.0;
float obterAnguloInclinacao2() {
	gyro.read();
	float y = gyro.data.y;
	LeituraAcelerometro reading = lerAcelerometro();
	unsigned long currentMillis = millis();
	double dt = currentMillis - previousMillis;
	dt = dt / 1000.0;
	previousMillis = currentMillis;
	float pesoGiroscopio = 0.9935;
	double pesoAcelerometro = 1 - pesoGiroscopio;
	angle = (pesoGiroscopio) * (angle + y * dt)
				+ (pesoAcelerometro) * (reading.x);
	return angle;
}

LeituraAcelerometro lerAcelerometro() {
	LeituraAcelerometro leitura;
	accelCompass.read();
	// De acordo com o datasheet do acelerômetro,
	// essa é a "mágica" matemática que deve ser feita
	// para transformar as leituras "cruas" para Gs
	// quando o acelerômetro está configurado
	// para trabalhar entre -8G <-> 8G.
	leitura.x = ((accelCompass.a.x >> 4) * 3.9 / 10.0);
//	leitura.y = ((accelCompass.a.y >> 4) * 3.9 / 10.0);
//	leitura.z = ((accelCompass.a.z >> 4) * 3.9 / 10.0);
	return leitura;
}

void lerComandoConfiguracao() {
	if (Serial.available()) {
		Serial.readBytes(buffer, 5);
		String comandoInteiro = String(buffer);
		String comando = comandoInteiro.substring(0, 2);
		if (AA.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			anguloAjuste = dado.toInt() * 1000;
			Serial.print("Angulo de ajuste definido em ");
			Serial.println(anguloAjuste);
		} else if (AM.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			anguloMaximo = dado.toInt() * 1000;
			Serial.print("Angulo maximo definido em ");
			Serial.println(anguloMaximo);
		} else if (PG.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			pesoGiroscopio = dado.toInt() / 1000.0;
			Serial.print("Peso do giroscopio definido em ");
			Serial.println(pesoGiroscopio);
		} else if (PM.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			int potencia = dado.toInt();
			if (potencia <= 0 || potencia > 255) {
				Serial.println("Comando invalido");
			} else {
				potenciaMaxima = potencia;
				Serial.print("Potencia maxima definida em ");
				Serial.println(potenciaMaxima);
			}
		} else if (LP.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			int potencia = dado.toInt();
			if (potencia <= 0 || potencia > 255) {
				Serial.println("Comando invalido");
			} else {
				limitadorPotencia = potencia;
				Serial.print("Potencia limitada em ");
				Serial.println(limitadorPotencia);
			}
		} else if (CF.equals(comando)) {
			Serial.print("Angulo de ajuste: ");
			Serial.println(anguloAjuste);
			Serial.print("Angulo maximo: ");
			Serial.println(anguloMaximo);
			Serial.print("Peso do giroscópio: ");
			Serial.println(pesoGiroscopio);
			Serial.print("Potencia maxima: ");
			Serial.println(potenciaMaxima);
			Serial.print("Limitador potencia: ");
			Serial.println(limitadorPotencia);
		} else if (LG.equals(comando)) {
			String dado = comandoInteiro.substring(2, 5);
			if (SIM.equals(dado)) {
				logar = true;
			} else {
				logar = false;
			}
		} else {
			Serial.println("Comando invalido");
		}
	}
}

void log(int x) {
	if (logar) {
		Serial.print(x);
	}
}
void logln(int x) {
	if (logar) {
		Serial.println(x);
	}
}
void log(String x) {
	if (logar) {
		Serial.print(x);
	}
}
void logln(String x) {
	if (logar) {
		Serial.println(x);
	}
}

void sobeEDesceProsDoisLados() {
	for (int i = 0; i < potenciaMaxima; i += 3) {
		digitalWrite(disable, LOW);
		digitalWrite(AHI, HIGH);
		digitalWrite(BHI, HIGH);
		analogWrite(ALI, i);
		digitalWrite(BLI, LOW);
		delay(100);
	}
	for (int i = potenciaMaxima; i > 0; i -= 3) {
		digitalWrite(disable, LOW);
		digitalWrite(AHI, HIGH);
		digitalWrite(BHI, HIGH);
		analogWrite(ALI, i);
		digitalWrite(BLI, LOW);
		delay(100);
	}
	for (int i = 0; i < potenciaMaxima; i += 3) {
		digitalWrite(disable, LOW);
		digitalWrite(AHI, HIGH);
		digitalWrite(BHI, HIGH);
		analogWrite(BLI, i);
		digitalWrite(ALI, LOW);
		delay(100);
	}
	for (int i = potenciaMaxima; i > 0; i -= 3) {
		digitalWrite(disable, LOW);
		digitalWrite(AHI, HIGH);
		digitalWrite(BHI, HIGH);
		analogWrite(BLI, i);
		digitalWrite(ALI, LOW);
		delay(100);
	}
}

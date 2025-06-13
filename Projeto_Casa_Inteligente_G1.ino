//biblioteca
//#include <ServoMotor.h>
#include <ESP32Servo.h>

//varival de deteccao de movimento
const int trigPin = 5;   // Pino Trigger
const int echoPin = 18;  // Pino Echo
const int ledPin = 2;    // Pino do LED
const int distanciaLimite = 100;

long duracao;     // Variável para armazenar o tempo do eco (em microssegundos)
int distanciaCm;  // Variável para armazenar a distância calculada (em cm)

//variavel de abrir e fechar porta
const int rele = 15;

// servo motor
Servo motor;  //variavel do tipo motor
const int servoMotor = 19;

void AbrirPortaServo() {
  distanciaCm = DetectarMovimentoComUltrassonico();  //pega o retorno da função que usa o sensor ultrasônico

  if (distanciaCm <= 100) {
    motor.write(90);
    digitalWrite(ledPin, HIGH);
    delay(3000);
  }
}

void FecharPortaServo() {
  // if (distanciaCm < 0 && distanciaCm > 100) {
    motor.write(0);
    digitalWrite(ledPin, LOW);
  // }
}

void TrancarPorta() {
  digitalWrite(rele, LOW);
  Serial.println(" Porta trancada ");
  delay(1500);
}

void DestrancarPorta() {

  digitalWrite(rele, HIGH);
  Serial.println("Porta destrancada");

  AbrirPortaServo();
  FecharPortaServo();
}

float DetectarMovimentoComUltrassonico() {

  long duracao;
  float distancia;

  // Envia pulso para o TRIG
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Mede o tempo que o ECHO levou para receber o sinal
  duracao = pulseIn(echoPin, HIGH);

  // Calcula a distância (velocidade do som = 343 m/s ou 0.0343 cm/us)
  distancia = (duracao * 0.0343) / 2;

  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println(" cm");

  return distancia;
}


void setup() {
  Serial.begin(115200);

  pinMode(rele, OUTPUT);
  motor.attach(servoMotor);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  //angulo da posicao inicial do motor
  motor.write(0);  //por enquanto o angulo e 0

  //deixa trancado o solenoide
  digitalWrite(rele, HIGH);

  //monitor serial
  Serial.println("Aguarde um tempo para os sensores serem calibrados.");
  delay(10000);
  Serial.println("Os sensores foram calibrados com sucesso.");
}

void loop() {
  AbrirPortaServo();
  FecharPortaServo();
  //DestrancarPorta();
  //TrancarPorta();
}

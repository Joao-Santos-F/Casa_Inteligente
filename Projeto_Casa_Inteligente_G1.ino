#include <ESP32Servo.h>

// Pinos do sensor ultrassônico
const int trigPin = 5;
const int echoPin = 18;
const int distanciaLimite = 30; // Distância em cm para ativar o sistema

// Pino do LED
const int ledPin = 2;

// Pino do relé (tranca)
const int rele = 15;

// Servo motor
Servo motor;
const int servoMotor = 19;

// Variáveis de controle
long duracao;
float distanciaCm;
bool portaAberta = false;

void setup() {
  Serial.begin(115200);

  // Configura os pinos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(rele, OUTPUT);

  // Inicia o servo
  motor.attach(servoMotor);
  motor.write(90); // posição inicial = fechada

  // Tranca a porta inicialmente
  digitalWrite(rele, LOW); // relé desligado = trancado

  Serial.println("Aguardando calibracao dos sensores...");
  delay(5000);
  Serial.println("Sistema iniciado.");
}

void loop() {
  distanciaCm = DetectarMovimentoComUltrassonico();

  if (distanciaCm > 0 && distanciaCm <= distanciaLimite) {
    if (!portaAberta) {
      AbrirPorta();
      portaAberta = true;
    }
  } else {
    if (portaAberta) {
      FecharPorta();
      portaAberta = false;
    }
  }

  delay(500); // Pequeno atraso para evitar leituras muito rápidas
}

float DetectarMovimentoComUltrassonico() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracao = pulseIn(echoPin, HIGH);
  float distancia = (duracao * 0.0343) / 2;

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  return distancia;
}

void AbrirPorta() {
  Serial.println("Pessoa detectada. Abrindo porta.");

  digitalWrite(ledPin, HIGH);
  DestrancarPorta();
  delay(1000);
  motor.write(0); // abre
}

void FecharPorta() {
  Serial.println("Pessoa ausente. Fechando porta.");

  digitalWrite(ledPin, LOW);
  motor.write(90); // fecha
  delay(1000);
  TrancarPorta();
}

void DestrancarPorta() {
  digitalWrite(rele, HIGH); // energiza o relé (destrava)
  Serial.println("Porta destrancada");
}

void TrancarPorta() {
  digitalWrite(rele, LOW); // desenergiza o relé (trava)
  Serial.println("Porta trancada");
}

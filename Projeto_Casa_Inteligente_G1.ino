#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- WiFi & MQTT ---
const char* ssid = "ProjetosIoT_Esp32";//sua rede wifi
const char* password = "Sen@i134";//senha da sua rede wifi
const char* mqtt_server = "172.16.39.118";//endereço do broker público
const int mqtt_port = 1883;//porta do broker público, geralmente 1883

//Tópicos
const char* topic_led = "escolainteligente/lab19/luzsala";
const char* topic_porta = "escolainteligente/lab19/porta";

// Pinos do sensor ultrassônico
const int trigPin = 5;
const int echoPin = 18;
const int distanciaLimite = 30; // Distância em cm para ativar o sistema

// Pino do LED
const int ledPin = 2;

// Pino do relé (tranca)
const int rele = 15;

// Servo motor
const int servoMotor = 19;

// Variáveis de controle
long duracao;
float distanciaCm;
bool portaAberta = false;

// --- Objetos ---
WiFiClient espClient;
PubSubClient client(espClient);
Servo motor;

// --- Funções WiFi e MQTT ---
void conectarWiFi() {//verifica conexão wifi para somente depois iniciar o sistema
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconectarMQTT() {//verifica e reconecta a conexão com o broker mqtt
  while (!client.connected()) {
    Serial.print("Reconectando MQTT...");
    if (client.connect("ESP32ClientTest")) {
      Serial.println("Conectado!");
      client.subscribe(topic_led);//conecta ao topico do led assim que estabelecer ligação com o broker
      client.subscribe(topic_porta);//conecta ao topico da porta assim que estabelecer ligação com o broker
    } else {
      Serial.print("Falha: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

/**
  Função para tratamento das mensagens de callback/retorno do broker de cada tópico subscrito (led, porta, etc.)

  char* é uma cadeia de caracteres em C como um vetor onde cada caractter/letra está em uma posição, 
  diferente de uma String em C++ que pode ser lida completamente
*/
void tratarMensagem(char* topic, byte* payload, unsigned int length) {//
  String mensagem = "";
  for (int i = 0; i < length; i++) {//concatena todas os char* para se ter o texto completo em String
    mensagem += (char)payload[i];
  }

  Serial.printf("Mensagem recebida [%s]: %s\n", topic, mensagem.c_str());
  
  //led - luz da sala
  if (strcmp(topic, topic_led) == 0) {//tópico atual é o do led?
    if (mensagem == "ligar") {
      digitalWrite(ledPin, HIGH);
    } else if (mensagem == "desligar") {
      digitalWrite(ledPin, LOW);
    }
  }
  
  /*
    Verifica se o tópico recebido é o topico da porta
  é uma função da linguagem C que compara duas strings (topic e topic_porta)
  */
  //porta
  if (strcmp(topic, topic_porta) == 0) {//tópico atual é o da porta?
    if (mensagem == "abrir") {
      DestrancarPorta();
      delay(500);
      AbrirPorta();
    } else if (mensagem == "fechar") {
      FecharPorta();
      delay(500);
      TrancarPorta();
    }
  }
}


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

  //digitalWrite(ledPin, HIGH);
  DestrancarPorta();
  delay(1000);
  motor.write(0); // abre
}

void FecharPorta() {
  Serial.println("Pessoa ausente. Fechando porta.");

  //digitalWrite(ledPin, LOW);
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

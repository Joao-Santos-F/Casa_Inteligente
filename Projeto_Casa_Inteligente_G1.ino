//#include <ServoMotor.h>
#include <WiFi.h>
#include <PubSubClient.h>
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
const int servoMotor = 19;

// --- WiFi & MQTT ---
const char* ssid = "ProjetosIoT_Esp32";//sua rede wifi
const char* password = "Sen@i134";//senha da sua rede wifi
const char* mqtt_server = "172.16.39.79";//endereço do broker público
const int mqtt_port = 1883;//porta do broker público, geralmente 1883

//Tópicos
const char* topic_led = "InovaHome_Tech/lampada/sala";
//const char* topic_temp = "escolainteligente/lab19/temperatura";
//const char* topic_umid = "escolainteligente/lab19/umidade";
const char* topic_porta = "InovaHome_Tech/lampada/sala";

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

/*
  Função para tratamento das mensagens de callback/retorno do broker de cada tópico subscrito (led, porta, etc.)
  char* é uma cadeia de caracteres em C como um vetor onde cada caractter/letra está em uma posição, 
  diferente de uma String em C++ que pode ser lida completamente
*/
void tratarMensagem(char* topic, byte* payload, unsigned int length) {
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
      AbrirPortaServo();
    } else if (mensagem == "fechar") {
      FecharPortaServo();
      delay(500);
      TrancarPorta();
    }
  }
}

void AbrirPortaServo() {
  distanciaCm = DetectarMovimentoComUltrassonico();  //pega o retorno da função que usa o sensor ultrasônico

  if (distanciaCm <= 100) {
    digitalWrite(ledPin, HIGH);
    motor.write(0);
    delay(500);
    DestrancarPorta();
  }
}

void FecharPortaServo() {
  if (distanciaCm < 0 && distanciaCm > 100) {
    digitalWrite(ledPin, LOW);
    motor.write(90);
    delay(500);
    TrancarPorta();
  }
}

void TrancarPorta() {
  digitalWrite(rele, LOW);
  Serial.println(" Porta trancada ");
}

void DestrancarPorta() {

  digitalWrite(rele, HIGH);
  Serial.println("Porta destrancada");

  //AbrirPortaServo();
  //FecharPortaServo();
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
  DestrancarPorta();
  TrancarPorta();
}


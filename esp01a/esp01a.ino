#include <stdio.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DEBUG_UART_BAUDRATE   115200   

#define MQTT_PUB_TOPIC "tago/data/post"
#define MQTT_USERNAME  "esp01a"
#define MQTT_PASSWORD  "af1876a8-6ab5-49d5-86eb-5f0e4774614e"
const int PinEcho = 2; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int PinTrigger = 0; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)
unsigned long TEMPO_ENVIO_INFORMACOES = millis();
/* WIFI */
const char* ssid_wifi = "hsmm";         /*  WI-FI network SSID (name) you want to connect */
const char* password_wifi = "Hsmm0106"; /*  WI-FI network password */
WiFiClient espClient;    

/* MQTT */
const char* broker_mqtt = "mqtt.tago.io"; /* MQTT broker URL */
int broker_port = 1883;                   /* MQTT broker port */
PubSubClient MQTT(espClient);

void connect_MQTT(void);
void connect_wifi(void);
void send_data_iot_platform(void);

float TempoEcho = 0;
const float VelocidadeSom_mpors = 340;
const float VelocidadeSom_mporus = 0.000340; 

void DisparaPulsoUltrassonico(){
  digitalWrite(PinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(PinTrigger, LOW);
  }

float CalculaDistancia(float tempo_us){
  return(100*(tempo_us*VelocidadeSom_mporus)/2);
}
void connect_wifi(void){
  Serial.println("------WI-FI -----");
  Serial.print("Tentando se conectar a rede wi-fi ");
  Serial.println(ssid_wifi);
  Serial.println("Aguardando conexao");  
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.begin(ssid_wifi, password_wifi);
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
    }
  Serial.println();
  Serial.print("Conectado com sucesso a rede wi-fi: ");
  Serial.println(ssid_wifi);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  }


void connect_MQTT(void){
  MQTT.setServer(broker_mqtt, broker_port);
  char mqtt_id_randomico[5] = {0};
  while (!MQTT.connected()){
    Serial.print("* Tentando se conectar ao broker MQTT: ");
    Serial.println(broker_mqtt);
    /* gera id mqtt randomico */
    randomSeed(random(9999));
    sprintf(mqtt_id_randomico, "%ld", random(9999));
    if (MQTT.connect(mqtt_id_randomico, MQTT_USERNAME, MQTT_PASSWORD)){
      Serial.println("Conectado ao broker MQTT com sucesso!");
      } 
    else{
      Serial.println("Falha na tentativa de conexao com broker MQTT.");
      Serial.println("Nova tentativa em 2s...");
      delay(2000);
      }
    }
  }

/*
JSON a ser enviado para Tago.io:

{
    "variable": "nome_da_variavel",
    "unit"    : "unidade",
    "value"   : valor
}
*/
void send_data_iot_platform(void){
  StaticJsonDocument<250> tago_json_distance;
  char json_string[250] = {0};
  DisparaPulsoUltrassonico();
  TempoEcho = pulseIn(PinEcho, HIGH);
  float distance = CalculaDistancia(TempoEcho);
  int i;
  for (i=0; i<5; i++)
    Serial.println(" ");
  Serial.println("----------");
  Serial.print("Distance: ");
  Serial.println(distance);
  /* Envio do volume */
  tago_json_distance["variable"] = "distance";
  tago_json_distance["unit"] = "cm";
  tago_json_distance["value"] = distance;
  memset(json_string, 0, sizeof(json_string));
  serializeJson(tago_json_distance, json_string);
  MQTT.publish(MQTT_PUB_TOPIC, json_string);
  }

void setup(){
  Serial.begin(DEBUG_UART_BAUDRATE);
  connect_wifi();
  connect_MQTT();
  pinMode(PinTrigger, OUTPUT);
  digitalWrite(PinTrigger, LOW);
  pinMode(PinEcho, INPUT);
  }

void loop() {
  connect_wifi();
  connect_MQTT();
  if((millis() - TEMPO_ENVIO_INFORMACOES) > 5000){
    send_data_iot_platform();
    TEMPO_ENVIO_INFORMACOES = millis();
    }
  }

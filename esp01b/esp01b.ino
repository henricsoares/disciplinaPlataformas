#include <stdio.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DEBUG_UART_BAUDRATE   115200   

#define MQTT_PUB_TOPIC "tago/data/post"
#define MQTT_USERNAME  "esp01b"
#define MQTT_PASSWORD  ""
unsigned long TEMPO_ENVIO_INFORMACOES = millis();
bool waterSupply = false;

/* MQTT */
const char* broker_mqtt = "mqtt.tago.io"; /* MQTT broker URL */
int broker_port = 1883;                   /* MQTT broker port */
PubSubClient MQTT(espClient);

/* WIFI */
const char* ssid_wifi = "hsmm";         /*  WI-FI network SSID (name) you want to connect */
const char* password_wifi = "********"; /*  WI-FI network password */
WiFiClient espClient;     
 

void connect_MQTT(void);
void connect_wifi(void);
void send_data_iot_platform(void);
void pumpWater(void);

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
  MQTT.setCallback(callback);
  char mqtt_id_randomico[5] = {0};
  while (!MQTT.connected()){
    Serial.print("* Tentando se conectar ao broker MQTT: ");
    Serial.println(broker_mqtt);
    /* gera id mqtt randomico */
    randomSeed(random(9999));
    sprintf(mqtt_id_randomico, "%ld", random(9999));
    if (MQTT.connect(mqtt_id_randomico, MQTT_USERNAME, MQTT_PASSWORD)){
      Serial.println("Conectado ao broker MQTT com sucesso!");
      MQTT.subscribe("tago/data/post/wsupply");
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
  StaticJsonDocument<250> tago_json_bomba;
  char json_string[250] = {0};
  /* Envio do estado */
  tago_json_volume["variable"] = "bomba";
  tago_json_volume["unit"] = "";
  tago_json_volume["value"] = waterSupply;
  memset(json_string, 0, sizeof(json_string));
  serializeJson(tago_json_bomba, json_string);
  MQTT.publish(MQTT_PUB_TOPIC, json_string);
  }

void callback(char* topic, byte* payload, unsigned int length){
  char resposta = (char)payload[0];
  if(resposta >> 0)
    waterSupply = true;        
    }
  else
    waterSupply = false;
  pumpWater();
  if((millis() - TEMPO_ENVIO_INFORMACOES) > 1000){
    send_data_iot_platform();
    TEMPO_ENVIO_INFORMACOES = millis();
    }  
  }
  

void pumpWater(void){
  if(waterSupply)
    digitalWrite(0, HIGH);
  else
    digitalWrite(0, LOW);
  }

void setup(){
  Serial.begin(DEBUG_UART_BAUDRATE);
  connect_wifi();
  connect_MQTT();
  pinMode(0, OUTPUT);
  }

void loop() {
  connect_wifi();
  connect_MQTT();
  MQTT.loop();
  }

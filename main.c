#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
#define TOPICO_SUBSCRIBE "comando"
#define TOPICO_PUBLISH   "umidade"    
#define ID_MQTT  "Planta"   
                                
#define D1    5
 
  
// WIFI
const char* SSID = "nome_da_rede"; //Nome da Rede wifi
const char* PASSWORD = "senha"; //Senha da Rede wifi
  
// MQTT
const char* BROKER_MQTT = "test.mosquitto.org"; //URL do broker MQTT
int BROKER_PORT = 1883; // Porta do Broker MQTT
 
WiFiClient espClient; 
PubSubClient MQTT(espClient);
 
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
 
void setup() 
{
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
}
 
void initSerial() 
{
    Serial.begin(115200);
}
 
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
 
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}
  
 
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
 
    Serial.println("mqtt_callback");
 
    if (msg.equals("L"))
    {
        Serial.println("Ligando Irrigacao");
        digitalWrite(D1, LOW);
    }
 
    if (msg.equals("D"))
    {
        Serial.println("Desligando Irrigacao");
        digitalWrite(D1, HIGH);
    }
     
}
 
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
 
void reconectWiFi() 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT();
     
     reconectWiFi();
}
 
 
void EnviaEstadoOutputMQTT(void)
{
    int umidade = analogRead(A0);
 
    if (umidade >= 0 && umidade <= 400) {
      MQTT.publish(TOPICO_PUBLISH, "Solo Umido");
    }
 
    if (umidade > 400 && umidade <= 800) {
      MQTT.publish(TOPICO_PUBLISH, "Solo com umidade moderada");
    }
 
    if (umidade > 800 && umidade <= 1024) {
      MQTT.publish(TOPICO_PUBLISH, "Solo seco");
    }
 
    Serial.println("- Umidade enviada ao broker!");
    delay(1000);
}
 
void InitOutput(void)
{
    pinMode(D1, OUTPUT);
    digitalWrite(D1, HIGH);          
}
 
 
void loop() 
{
    VerificaConexoesWiFIEMQTT();
    EnviaEstadoOutputMQTT();
    MQTT.loop();
}

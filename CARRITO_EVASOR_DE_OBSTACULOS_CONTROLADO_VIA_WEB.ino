#include <WiFi.h>
#include <Ultrasonic.h>
#include <analogWrite.h>
#include <ESP32Servo.h>
Servo myservo;
#define GPIO_LED    15
#define IN1         25
#define IN2         26
#define IN3         32
#define IN4         33
//------------------Servidor Web en puerto 80---------------------

Ultrasonic ultrasonic(13, 12);
WiFiServer server(80);

//---------------------COLA---------------------------------------

QueueHandle_t queue, queueA;

//---------------------Credenciales de WiFi-----------------------

const char* ssid     = "****";
const char* password = "KHsFH5ak1V";

//---------------------VARIABLES GLOBALES-------------------------

int contconexion = 0;

String header; // Variable para guardar el HTTP request

String estadoSalida = "off";

const int salida = 2;

int A;

bool E;

//-------variables globales para Programa Carrito autonomo-------

int obstamax =14;
int obstamin= 2;
int i=0;
int duracion;
int distancia;
int CMD;
int CMC;
int CMI;
int Cm;
int angizq= 150;
int angdere= 30;
int angcent=90;
int velocidad =245;
//------------------------CODIGO HTML------------------------------
String pagina = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<title>Servidor Web ESP32</title>"
"</head>"
"<body>"
"<center>"
"<h1>Servidor Web ESP32</h1>"
"<p><a href='formas.html'><button style='height:50px;width:100px'>Manual</button></a></p>"
"<p><a href='/auto'><button style='height:50px;width:100px'>Automatico</button></a></p>"
"</center>"
"</body>"
"</html>";

String pagina2 = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<title>Servidor Web ESP32</title>"
"</head>"
"<body>"
"<center>"
"<h1>CONTROL MANUAL</h1>"
"<p><a href='/adelante'><button style='height:50px;width:100px'>Adelante</button></a></p>"
"<p><a href='/izquierda'><button style='height:50px;width:100px'>Izquierda</button></a></p>"
"<p><a href='/derecha'><button style='height:50px;width:100px'>Derecha</button></a></p>"
"<p><a href='/atras'><button style='height:50px;width:100px'>Atras</button></a></p>"
"<p><a href='/paro'><button style='height:50px;width:100px'>Detenerse</button></a></p>"
"</center>"
"<script>"
"setInterval(function()"
    "{"
      "getPOTval();"
    "}, 2000);"
    //-------------------------------------------------------
    "function getPOTval()"
    "{"
      "var POTvalRequest = new XMLHttpRequest();"
      "POTvalRequest.onreadystatechange = function()"
      "{"
        "if(this.readyState == 4 && this.status == 200)"
        "{"
         " document.getElementById(""POTvalue"").innerHTML ="
          "this.responseText;"
        "}"
      "};"
      "POTvalRequest.open(""GET"", ""readPOT"", true);"
      "POTvalRequest.send();"
    "}"
    "</script>"
"</body>"
"</html>";
//------------------------Funtions------------------------------
static void Internet_task(void *argp){
  for (;;){
      WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) { 
    //bool E; // Si se conecta un nuevo cliente
    //int A;
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    client.println(pagina);
    while (client.connected()) {            // loop mientras el cliente está conectado
      
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;     
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            //client.println("HTTP/1.1 200 OK");
            //client.println("Content-type:text/html");
            //client.println("Connection: close");
            //client.println();
            
            // enciende y apaga el GPIO
            if (header.indexOf("GET /auto") >= 0) {
              Serial.println("GPIO on");
              estadoSalida = "on";
              E = false;
              A=0;
               xQueueSend(queue, &E,0);
              digitalWrite(salida, HIGH);
              
              //client.println(pagina);
            } else if (header.indexOf("GET /formas.html") >= 0) {
              Serial.println("GPIO off");
              estadoSalida = "off";
              E = true;
              xQueueSend(queue, &E, portMAX_DELAY);
              digitalWrite(salida, LOW);
              client.println(pagina2);
            } else if(header.indexOf("GET /adelante") >= 0){
              A=1;
              //xQueueSend(queueA, &A, 0);
              client.println(pagina2);
            } else if(header.indexOf("GET /izquierda") >= 0){
              A=2;
              //xQueueSend(queueA, &A,0);
              client.println(pagina2);
            } else if(header.indexOf("GET /derecha") >= 0){
              A=3;
              //xQueueSend(queueA, &A, 0);
              client.println(pagina2);
            } else if(header.indexOf("GET /atras") >= 0){
              A=4;
              //xQueueSend(queueA, &A, portMAX_DELAY);
              client.println(pagina2);
            } else if(header.indexOf("GET /paro") >= 0){
              A=5;
              //xQueueSend(queueA, &A, 0);
              client.println(pagina2);
            }

                  switch (A) {
        case 1:
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
        break;
        case 2:
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
        break;
        case 3:
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
        break;
        case 4:
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,HIGH);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,HIGH);
        break;
        default:
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
        break;

        }
            // Muestra la página web
            
            //client.println(pagina);
            
            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
    }
  }


  static void led_task(void *argp){
    for (;;){
      Cm = ultrasonic.read();//lee la funcion de leer los centimetros
      if (digitalRead(2)== HIGH){
      if(Cm< obstamax && Cm >=obstamin ) {
       digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
         myservo.write(angdere);//servomotor derecha;
         delay(1000);
         CMD = ultrasonic.read();//lee la funcion de leer los centimetros
         myservo.write(angizq);//servomotor derecha
         delay(1000);
         CMI = ultrasonic.read();//lee la funcion de leer los centimetros
         if(CMI > CMD)//HAY OBSTA EN DERECHA
          {
           myservo.write(angcent);//servomotor en 180 grados izquierda
           delay(1000);
           digitalWrite(IN1,LOW);
          digitalWrite(IN2,LOW);
          digitalWrite(IN3,HIGH);
          digitalWrite(IN4,LOW);
           delay(500);
          }
           if(CMI < CMD)//OBSTA IZQ
          {
           myservo.write(angcent);//servomotor en 180 grados izquierda
           delay(1000);
            digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
           delay(500);
          }
          }
         else if(digitalRead(2)== HIGH){
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,HIGH);
        digitalWrite(IN4,LOW);
            }
    
      }
      else if(E ==false){
       digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
      } 
    }
  }
//---------------------------SETUP--------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("");
  myservo.attach(14);
  int app_cpu = xPortGetCoreID();
  TaskHandle_t h;
  BaseType_t rc;
  delay(2000);
  pinMode(salida, OUTPUT);
  pinMode(GPIO_LED, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT); 
  digitalWrite(salida, LOW);
  digitalWrite(GPIO_LED, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  myservo.write(90);
  queue = xQueueCreate(10, sizeof(bool));
  queueA = xQueueCreate(10, sizeof(int));
   if(queue == NULL && queueA ==NULL ){
    Serial.println("Error creating the queue");
  }
  // Conexión WIFI
  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      //para usar con ip fija
      //IPAddress ip(192,168,1,180); 
      //IPAddress gateway(192,168,1,1); 
      //IPAddress subnet(255,255,255,0); 
      //WiFi.config(ip, gateway, subnet); 
      
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
      server.begin(); // iniciamos el servidor
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }
  rc = xTaskCreatePinnedToCore(
    Internet_task,
    "Conexion",
    2048,     // Stack size
    nullptr,  // No args
    1,        // Priority
    &h,       // Task handle
    0   // CPU
  );
  assert(rc == pdPASS);
  assert(h);

   rc = xTaskCreatePinnedToCore(
    led_task,
    "led",
    2048,     // Stack size
    nullptr,  // No args
    1,        // Priority
    &h,       // Task handle
    1   // CPU
  );
  assert(rc == pdPASS);
  assert(h);
}

//----------------------------LOOP----------------------------------

void loop(){
//vTaskDelete(nullptr);
}//

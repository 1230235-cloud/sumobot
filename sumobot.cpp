#include <Servo.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <QueueArray.h>
#include <StackArray.h>
#include <EEPROM.h>

// Definición del sensor de color
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Definiciones del servo
Servo servoMotor;
const int servoPin = 11;

// Definiciones de los pines de ultrasonido
const int Trigger = 8;
const int Echo = 7;

// Definiciones de los pines de motores
const int motorPin1 = 6;
const int motorPin2 = 9;
const int motorPin3 = 10;
const int motorPin4 = 5;

// Pines para sensores IR laterales
const int x1 = A0;
const int x2 = A1;
const int x3 = A2;
const int x4 = A3;

// Estructuras de datos para el laberinto
QueueArray<int> movimientos;
StackArray<int> camino;
const int TAMANIO_MATRIZ = 7;
int matriz[TAMANIO_MATRIZ][TAMANIO_MATRIZ];
int posX = 0;
int posY = 0;
int direccion = 0; // 0: Norte, 1: Este, 2: Sur, 3: Oeste

// Array para guardar el recorrido
int recorridoGuardado[50]; // Ajusta el tamaño según necesites
int indiceRecorrido = 0;

// Variables para el seguimiento de distancia
const float MEDIDA_CASILLA = 33.0; // cm por casilla
bool modoRetorno = false;
bool programaIniciado = false;

void setup() {
  Serial.begin(9600);
  
  // Inicializar sensor de color
  if (!tcs.begin()) {
    Serial.println("Error al iniciar TCS34725");
    while (1);
  }
  
  // Configuración del servo
  servoMotor.attach(servoPin);
  servoMotor.write(65);
  
  // Configuración de los pines
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(x1, INPUT);
  pinMode(x4, INPUT);
  
  digitalWrite(Trigger, LOW);
  
  // Inicializar matriz del laberinto
  for(int i = 0; i < TAMANIO_MATRIZ; i++) {
    for(int j = 0; j < TAMANIO_MATRIZ; j++) {
      matriz[i][j] = -1; // -1 indica no explorado
    }
  }
  
  // Esperar a detectar verde antes de iniciar
  Serial.println("Esperando señal verde para iniciar...");
  while(!programaIniciado) {
    if(detectarColor("verde")) {
      Serial.println("¡Verde detectado! Iniciando...");
      programaIniciado = true;
    }
    delay(100);
  }
}

// Función para detectar color
bool detectarColor(String color) {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  
  float suma = r + g + b;
  float rNorm = r / suma;
  float gNorm = g / suma;
  float bNorm = b / suma;
  
  if (color == "verde") {
    return (gNorm > 0.4 && rNorm < 0.3 && bNorm < 0.3);
  } else if (color == "rojo") {
    return (rNorm > 0.4 && gNorm < 0.3 && bNorm < 0.3);
  }
  return false;
}

// Función para medir distancia con ultrasonido
float medirDistancia() {
  digitalWrite(Trigger, HIGH);
  delayMicroseconds(33);
  digitalWrite(Trigger, LOW);
  
  long tiempo = pulseIn(Echo, HIGH);
  float distancia = tiempo / 30.0;
  return distancia;
}

// Función para encontrar la mejor ruta usando el servo
int encontrarMejorRuta() {
  float distancias[3];
  
  // Medir distancia a la derecha
  servoMotor.write(0);
  delay(500);
  distancias[0] = medirDistancia();
  
  // Medir distancia al frente
  servoMotor.write(65);
  delay(500);
  distancias[1] = medirDistancia();
  
  // Medir distancia a la izquierda
  servoMotor.write(180);
  delay(500);
  distancias[2] = medirDistancia();
  
  // Volver a posición central
  servoMotor.write(65);
  
  // Encontrar la dirección con mayor distancia
  int mejorDireccion = 1; // Por defecto al frente
  float maxDistancia = distancias[1];
  
  if (distancias[0] > maxDistancia) {
    maxDistancia = distancias[0];
    mejorDireccion = 0; // Derecha
  }
  if (distancias[2] > maxDistancia) {
    maxDistancia = distancias[2];
    mejorDireccion = 2; // Izquierda
  }
  if (distancias[1] == maxDistancia) {
    maxDistancia = distancias[2];
    mejorDireccion = 0; // Derecha
  }
  
  return mejorDireccion;
}

void actualizarPosicion() {
  // Marcar celda actual como visitada
  matriz[posY][posX] = 1;
  
  // Actualizar posición
  switch(direccion) {
    case 0: if(posY < TAMANIO_MATRIZ-1) posY++; break;
    case 1: if(posX < TAMANIO_MATRIZ-1) posX++; break;
    case 2: if(posY > 0) posY--; break;
    case 3: if(posX > 0) posX--; break;
  }
  
  // Guardar movimiento en la pila y en el array de recorrido
  camino.push(direccion);
  if(indiceRecorrido < 50) { // Prevenir desbordamiento
    recorridoGuardado[indiceRecorrido++] = direccion;
  }
}

// Funciones de movimiento
void adelante() {
  analogWrite(motorPin1, 255);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, 255);
}

void girarDerecha() {
  analogWrite(motorPin1, 250);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, 252);
  analogWrite(motorPin4, 0);
  direccion = (direccion + 1) % 4;
}

void girarIzquierda() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 255);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, 250);
  direccion = (direccion + 3) % 4;
}

void detener() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, 0);
}

void atras() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 255);
  analogWrite(motorPin3, 255);
  analogWrite(motorPin4, 0);
  delay(50);
}

void vuelta() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 255);
  analogWrite(motorPin3, 255);
  analogWrite(motorPin4, 0);
}

void autocorregir(){
  int valor2 = analogRead(x2);  
  int valor3 = analogRead(x3);
  while (valor2 < 500 && valor3 < 500){
    analogWrite(motorPin1, 0);   // Atrás en motor izquierdo
    analogWrite(motorPin2, 200);
    analogWrite(motorPin3, 200); // Atrás en motor derecho
    analogWrite(motorPin4, 0);
    delay(10);
    // Releer valores
    valor2 = analogRead(x2);  
    valor3 = analogRead(x3);
  }
}

// Función para guardar el recorrido en EEPROM
void guardarRecorrido() {
  // Primero guardamos la longitud del recorrido
  EEPROM.write(0, indiceRecorrido);
  
  // Luego guardamos cada movimiento
  for(int i = 0; i < indiceRecorrido; i++) {
    EEPROM.write(i + 1, recorridoGuardado[i]);
  }
  
  Serial.println("Recorrido guardado en EEPROM");
  Serial.print("Longitud del recorrido: ");
  Serial.println(indiceRecorrido);
}

// Función para retornar por el camino más eficiente
void retornarInicio() {
  // Guardar el recorrido antes de vaciar la pila
  guardarRecorrido();
  
  modoRetorno = true;
  
  while(!camino.isEmpty()) {
    int movimientoAnterior = camino.pop();
    // Calcular el movimiento más eficiente para volver
    int movimientoOptimo = (movimientoAnterior + 2) % 4; // Dirección opuesta
    
    // Girar hasta alcanzar la dirección deseada
    while(direccion != movimientoOptimo) {
      girarIzquierda();
    }
    
    // Avanzar hasta encontrar obstáculo
    while(medirDistancia() > MEDIDA_CASILLA) {
      adelante();
    }
    actualizarPosicion(); 
  }
  
  detener();
  modoRetorno = false;  
}

void loop() {
  if(!programaIniciado) return;
  if(!modoRetorno) {
    // Verificar color rojo para retornar
    if(detectarColor("rojo")) {
      detener();
      retornarInicio();
      return;
    }
    
    // Leer sensores IR laterales
    int valorX1 = analogRead(x1);
    int valorX2 = analogRead(x2);  
    int valorX3 = analogRead(x3);  
    int valorX4 = analogRead(x4);
    float distanciaActual = medirDistancia();
    
    // Si hay obstáculo o detección lateral
    if (distanciaActual <= MEDIDA_CASILLA || (valorX1 > 500 && valorX4 > 500)) {
      detener();
      int mejorRuta = encontrarMejorRuta();
           
      // Girar según la mejor ruta encontrada
      switch(mejorRuta) {
        case 0: // Derecha
          atras();
          girarDerecha();
          delay(500);
          // Si x2 detecta blanco realizar ajuste
          if (analogRead(x2) > 500) {
            autocorregir();
          }
          adelante();
          break;
        case 2: // Izquierda
          atras();
          girarIzquierda();
          delay(500);
          adelante();
          break;
        default: // Retroceder y buscar nueva opción
          atras();
          atras();
          girarDerecha();
          delay(500);
          break;
      }
      actualizarPosicion();
    } else {
      // Continuar advancing sin obstáculos
      adelante();
    }
    
    // Corrección en caso de desviación lateral
    if (valorX2 > 500) {  // Blanco detectado en sensor lateral derecho
      analogWrite(motorPin1, 250);  // Motor izquierdo adelante
      analogWrite(motorPin2, 0);
      analogWrite(motorPin3, 255);  // Motor derecho atrás
      analogWrite(motorPin4, 0);
      delay(10);  // Ajuste rápido
    }
    if (valorX3 > 500) {  // Blanco detectado en sensor lateral izquierdo
      analogWrite(motorPin1, 0);  
      analogWrite(motorPin2, 255);  // Motor izquierdo atrás
      analogWrite(motorPin3, 0);
      analogWrite(motorPin4, 250);  // Motor derecho adelante
      delay(10);  // Ajuste rápido
    }
  }
  delay(50);
}

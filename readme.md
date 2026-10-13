# 🤖 Rescue Bot — WALL-X

> **"Más allá de la búsqueda: soluciones al alcance."**

![Arduino](https://img.shields.io/badge/Board-Arduino%20Uno-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/Code-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

Documentación técnica y código fuente del sistema embebido para **WALL-X (Rescue Bot)**, desarrollado como Proyecto Integrador en la **Universidad Montemorelos** (Facultad de Ingeniería y Tecnología).

---

## 📌 Visión General del Proyecto

**WALL-X** es una plataforma robótica autónoma diferencial diseñada para la exploración y navegación en laberintos estructurados no conocidos (matrices discretas de $7 \times 7$ celdas). 

El robot utiliza un esquema de percepción activa y estructuras de datos dinámicas en memoria SRAM para registrar su trayectoria, detectar objetivos por color, almacenar el mapa en memoria EEPROM y realizar un retorno automático al punto de origen desapilando sus decisiones de movimiento.

### 🔑 Características Principales
* **Arranque/Meta por Color:** Uso del sensor RGB TCS34725 con espacios de color normalizados para iniciar con verde y finalizar con rojo.
* **Escaneo Activo con Servo:** Barrido ultrasónico a $0^\circ$, $65^\circ$ y $180^\circ$ para evaluar la ruta con mayor distancia disponible ante un obstáculo.
* **Acondicionamiento Análogo (LM324):** Digitalización rápida de señales infrarrojas para autocorrección sobre líneas blancas.
* **Algoritmo de Retorno en Pila (`StackArray`):** Reconstrucción e inversión cinemática del camino en tiempo real ($(\text{dir} + 2) \bmod 4$).
* **Persistencia No Volátil:** Guardado de la ruta en la memoria EEPROM del microcontrolador.

---

## 🛠️ Especificaciones de Hardware

| Componente | Modelo / Descripción |
| :--- | :--- |
| **Microcontrolador** | Arduino UNO R3 (ATmega328P / CH340G) |
| **Driver de Motores** | Puente H L293D |
| **Acondicionador de Señal** | Cuádruple Op-Amp LM324 |
| **Sensor de Color** | Adafruit TCS34725 / TCS230 (I2C) |
| **Sensor Ultrasónico** | HC-SR04 |
| **Sensores de Línea** | 4x Módulos Infrarrojos TCRT5000 |
| **Actuador de Escaneo** | Servomotor SG90 (9g) |
| **Alimentación** | Batería LiPo 3.7V 5000mAh |
| **Chasis** | Diseño en Inventor/AutoCAD (Corte CNC + Impresión 3D) |

---

## 🔌 Pinout y Conexiones

```cpp
// =======================================================
// MAPEO DE PINES EN ARDUINO UNO
// =======================================================

// Actuador de barrido
const int servoPin   = 11; // Servomotor SG90

// Sensor Ultrasónico HC-SR04
const int Trigger    = 8;  
const int Echo       = 7;  

// Driver L293D (Motores)
const int motorPin1  = 6;  // Motor Izquierdo (+)
const int motorPin2  = 9;  // Motor Izquierdo (-)
const int motorPin3  = 10; // Motor Derecho (+)
const int motorPin4  = 5;  // Motor Derecho (-)

// Sensores IR (Entradas Analógicas / Comparador LM324)
const int x1         = A0; // Sensor Lateral Ext. Izquierdo
const int x2         = A1; // Sensor Ajuste Izquierdo
const int x3         = A2; // Sensor Ajuste Derecho
const int x4         = A3; // Sensor Lateral Ext. Derecho

// Bus I2C (Sensor de Color TCS34725)
// SDA -> A4
// SCL -> A5

[ INICIO ]
                     |
         [ Esperando Señal Verde ]  <--- (TCS34725)
                     |
                     v
          [ Avanzar / Navegación ]
                     |
         +-----------+-----------+
         |                       |
  (Pared <= 33cm)         (Sensor Color Rojo)
         |                       |
         v                       v
 [ Barrido Ultrasónico ]  [ Guardar en EEPROM ]
 [ (0°, 65°, 180°)     ]         |
         |                       v
 [ Decidir Mejor Ruta  ]  [ Retorno por Pila ]
 [ Guardar en Pila     ]  [  (StackArray)   ]
         |                       |
         +-----------+-----------+
                     |
                     v
             [ DETENIDO / FIN ]




2. Librerías requeridas

Asegúrate de instalar las siguientes librerías desde el Gestor de Librerías de Arduino IDE:

    Servo.h (Incluida en el núcleo de Arduino)

    Wire.h (Incluida en el núcleo de Arduino)

    EEPROM.h (Incluida en el núcleo de Arduino)

    Adafruit_TCS34725

    QueueArray

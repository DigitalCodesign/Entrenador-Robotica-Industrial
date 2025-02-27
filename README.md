# Entrenador-Robotica-Industrial

Este código fuente está construido por Digital Codesign para utilizar junto con el kit educacional "Entrenador de Robótica Industrial".

Puedes encontrar nuestro Entrenador y mucho más material de electrónica y robótica en nuestra tienda oficial: https://digitalcodesign.com/shop

# Control de Brazo Robótico Arduino

Este repositorio contiene el código fuente de Arduino para controlar un brazo robótico con modos manual y automático. El brazo utiliza servomotores, un motor paso a paso y un actuador, controlados mediante potenciómetros, un codificador y un botón. El código incluye funciones para almacenar y ejecutar posiciones predefinidas, así como para calcular y moverse a coordenadas específicas.

## Características

-   **Control Manual:**
    -    Control de servomotores, motor paso a paso y actuador utilizando potenciómetros y un codificador.
    -    Visualización en tiempo real de las coordenadas del brazo (X, Y, Z), rotación, rotación de la base, estado del actuador y nivel de pH en una pantalla LCD.
    -    Capacidad para almacenar la posición actual del brazo.
 
-   **Control Automático:**
    -    Ejecución de posiciones almacenadas.
    -    Ejecución de una secuencia de posiciones almacenadas.
    -    Movimiento a coordenadas específicas introducidas por el usuario.

-   **Interfaz de Codificador y Botón:**
    -    Control preciso del motor paso a paso y navegación por el menú utilizando un codificador.
    -    Confirmación de acciones con una pulsación de botón.

-   **Pantalla LCD:**
    -    Visualización clara de las opciones de menú, coordenadas y estado del sistema.

-   **Monitoreo del Nivel de pH:**
    -   Monitoreo de un sensor de PH.

-   **Cinemática Inversa:**
    -   Cálculo de la cinemática inversa para mover el brazo a las coordenadas deseadas.

## Requisitos de Hardware

-   Placa Arduino
-   Servomotores
-   Motor paso a paso y controlador
-   Actuador
-   Potenciómetros (4)
-   Codificador rotatorio
-   Botón pulsador
-   Pantalla LCD I2C (20x4)
-   Controlador de Servos PWM Adafruit (PCA9685)
-   Sensor de pH.
-   Zumbador

## Librerías

-   `Arduino.h`
-   `math.h`
-   `Wire.h`
-   `LiquidCrystal_I2C.h`
-   `Adafruit_PWMServoDriver.h`
-   `PinChangeInterrupt.h`
-   `AccelStepper.h`
-   `CinematicaInversa.h` (Librería personalizada para cinemática inversa)

## GPIOs

-   `CLK` (2): Pin de reloj del codificador
-   `DT` (3): Pin de datos del codificador
-   `SW` (4): Pin del interruptor del codificador
-   `DIR` (6): Pin de dirección del motor paso a paso
-   `STEP` (5): Pin de paso del motor paso a paso
-   `STORE` (7): Pin del botón de almacenar posición
-   `ACTUATOR` (8): Pin del interruptor del actuador
-   `BUZZER` (9): Pin del zumbador
-   `MODE` (10): Pin del interruptor de modo (manual/automático)
-   `MOTOR` (11): Pin del motor del actuador
-   `POTE_1` (A6): Pin del potenciómetro 1
-   `POTE_2` (A7): Pin del potenciómetro 2
-   `POTE_3` (A2): Pin del potenciómetro 3
-   `POTE_4` (A3): Pin del potenciómetro 4
-   `PH` (A0): Pin del sensor de pH

## Configuración

1.  Clona el repositorio en tu máquina local.
2.  Asegúrate de tener todas las librerías requeridas instaladas en tu IDE de Arduino.
3.  Conecta los componentes de hardware de acuerdo con las definiciones de GPIO en el código.
4.  Carga el código en tu placa Arduino.

## Uso

-   **Modo Manual:**
    -    Cambia el interruptor `MODE` a modo manual.
    -    Utiliza los potenciómetros para controlar los servomotores y el actuador.
    -    Utiliza el codificador para controlar el motor paso a paso.
    -    Presiona el botón `STORE` para guardar la posición actual.
    -    La pantalla LCD mostrará las coordenadas actuales, la rotación y el estado del actuador.
-   **Modo Automático:**
    -    Cambia el interruptor `MODE` a modo automático.
    -    Utiliza el codificador y el botón para navegar por las opciones del menú.
    -    Selecciona "Ejec. movimiento" para ejecutar una sola posición almacenada.
    -    Selecciona "Ejec. movimientos" para ejecutar todas las posiciones almacenadas.
    -    Selecciona "Ir a posición" para introducir coordenadas y mover el brazo a esa ubicación.
    -    La pantalla LCD te guiará a través del proceso de selección y ejecución.

## Personalización

-   Ajusta los parámetros del brazo (`L1`, `L2`, `L3`) en el código para que coincidan con las dimensiones de tu brazo robótico.
-   Modifica las asignaciones de los potenciómetros y los valores de control de los servos según sea necesario.
-   Personaliza las opciones del menú y los mensajes de la pantalla.
-   Adapta la librería `CinematicaInversa.h` para que se ajuste a tus requisitos específicos de cinemática inversa.
-   Cambia el desplazamiento y la multiplicación del sensor de PH en la función `getPhReading()`.

## Contribución

Siéntete libre de contribuir a este proyecto enviando solicitudes de extracción o abriendo incidencias.

## Licencia

Este proyecto está bajo la Licencia MIT.

// DECLARATION //////////////////////////////////
// Libraries ------------------------------------
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_PWMServoDriver.h>
#include <PinChangeInterrupt.h>
#include <AccelStepper.h>
#include "CinematicaInversa.h"

// GPIOs ----------------------------------------
constexpr uint8_t CLK = 2;
constexpr uint8_t DT = 3;
constexpr uint8_t SW = 4;
constexpr uint8_t DIR = 6;
constexpr uint8_t STEP = 5;
constexpr uint8_t STORE = 7;
constexpr uint8_t ACTUATOR = 8;
constexpr uint8_t BUZZER = 9;
constexpr uint8_t MODE = 10;
constexpr uint8_t MOTOR = 11;
constexpr uint8_t POTE_1 = A6;
constexpr uint8_t POTE_2 = A7;
constexpr uint8_t POTE_3 = A2;
constexpr uint8_t POTE_4 = A3;
constexpr uint8_t PH = A0;

// Arm params -----------------------------------
constexpr float L1 = 16.25f;
constexpr float L2 = 11.5f;
constexpr float L3 = 15.5f;

// Global Variables -----------------------------
long int last_refresh = 0;
int cursor = 0;
int positions_stored = 0;
int last_pote_readings[4] = { 0, 0, 0, 0 };
int pote_gpios[4] = { POTE_1, POTE_2, POTE_3, POTE_4 };
int ph_array[10];
int ph_index = 0;
float actual_ph = 0.0;
float stepper_steps = 0.0;
float last_micros_1 = 0.0;
float last_micros_2 = 0.0;
float last_micros_3 = 0.0;
float last_micros_4 = 0.0;
float base_rotation = 0.0;
float actual_angle_1 = 90.0;
float actual_angle_2 = 90.0;
float actual_angle_3 = 90.0;
float actual_micros_1 = 0.0;
float actual_micros_2 = 0.0;
float actual_micros_3 = 0.0;
float actual_micros_4 = 0.0;
float actuator_rotation = 0.0;
float actuator_state = 1.0;
bool last_menu = false;

// Position matrixes ----------------------------
float positions_array[20][6] = { 0 };

// Objects --------------------------------------
Adafruit_PWMServoDriver PCA = Adafruit_PWMServoDriver(0x40);
AccelStepper Stepper(1, STEP, DIR);
LiquidCrystal_I2C LCD(0x27, 20, 4);

// Encoder --------------------------------------
volatile bool button_pressed = 0;
volatile bool encoder_moved_right = false;
volatile bool encoder_moved_left = false;
volatile bool last_state = false;

// SETUP ////////////////////////////////////////
void setup()
{
    Serial.begin(9600);
    // Peripheral initialization
    PCA.begin();
    PCA.setPWMFreq(60);

    pinMode(CLK, INPUT);
    pinMode(DT, INPUT);
    pinMode(SW, INPUT_PULLUP);

    pinMode(DIR, OUTPUT);
    pinMode(STEP, OUTPUT);

    pinMode(ACTUATOR, INPUT);
    pinMode(MODE, INPUT);
    pinMode(STORE, INPUT);

    pinMode(BUZZER, OUTPUT);
    noTone(BUZZER);

    last_menu = digitalRead(MODE);

    // LCD configuration
    LCD.init();
    LCD.backlight();
    LCD.setCursor(0, 0);
    LCD.print("Iniciando...");
    delay(1000);
    LCD.clear();
    last_refresh = millis();

    // Encoder configuration
    attachInterrupt(digitalPinToInterrupt(CLK), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(DT), encoderISR, CHANGE);
    attachPCINT(digitalPinToPCINT(SW), buttonISR, CHANGE);

    actual_micros_1 = map(analogRead(POTE_1), 0, 1023, 500, 2500);
    actual_micros_2 = map(analogRead(POTE_2), 0, 1023, 500, 2500);
    actual_micros_3 = map(analogRead(POTE_3), 0, 1023, 500, 2500);
    actual_micros_4 = map(analogRead(POTE_4), 0, 1023, 500, 2500);

    PCA.writeMicroseconds(0, actual_micros_1);
    PCA.writeMicroseconds(1, actual_micros_2);
    PCA.writeMicroseconds(2, actual_micros_3);
    PCA.writeMicroseconds(3, actual_micros_4);
    PCA.writeMicroseconds(4, 2500);

    actual_ph = getPhReading();

}

// LOOP /////////////////////////////////////////
void loop()
{
    bool actual_menu = digitalRead(MODE);
    if(actual_menu != last_menu)
    {
        LCD.clear();
        last_menu = actual_menu;
    }
    actual_menu == 0 ? showAutomaticMenu() : showManualMenu();
}

// FUNCTIONS ////////////////////////////////////
// Encoder Interruption function ----------------
void encoderISR()
{
    bool current_state = digitalRead(CLK);
    if(current_state != last_state && current_state == LOW)
    {
        int direction = (digitalRead(DT) != current_state) ? 1 : -1;
        if(direction > 0) encoder_moved_right = true;
        if(direction < 0) encoder_moved_left = true;
    }
    last_state = current_state;
}

// Encoder switch Interruption function ---------
void buttonISR()
{
    static unsigned long last_debounce_time = 0;
    int button_state = digitalRead(SW);
    if(button_state == LOW)
    {
        unsigned long current_time = millis();
        if(current_time - last_debounce_time > 200)
        {
            button_pressed = true;
            last_debounce_time = current_time;
            tone(BUZZER, 2000, 100);
        }
    }
}

// Displays the Manual Mode Menu ----------------
void showManualMenu()
{
    // Move the different motors
    moveServosManual();
    moveStepperManual();
    moveActuatorManual();
    // Calculate the coordenates of the arm end point
    float x_coord, y_coord, z_coord;
    calcCoordsFromAngles(
        getReadingFromPote(0),
        getReadingFromPote(1),
        getReadingFromPote(2),
        x_coord,
        y_coord,
        z_coord
    );
    // Store the actual position
    manualStorePosition();
    // Display the coordenates info in the LCD
    LCD.setCursor(0, 0);
    LCD.print("Modo Manual");
    LCD.setCursor(12, 0);
    LCD.print("A:");
    LCD.setCursor(15, 0);
    LCD.print((int)actuator_state, 1);
    LCD.setCursor(0, 1);
    LCD.print("X:");
    LCD.setCursor(3, 1);
    LCD.print(x_coord, 1);
    LCD.setCursor(10, 1);
    LCD.print("Y:");
    LCD.setCursor(13, 1);
    LCD.print(y_coord, 1);
    LCD.setCursor(0, 2);
    LCD.print("Z:");
    LCD.setCursor(3, 2);
    LCD.print(z_coord, 1);
    LCD.setCursor(10, 2);
    LCD.print("R:");
    LCD.setCursor(13, 2);
    LCD.print(actuator_rotation, 1);
    LCD.setCursor(0, 3);
    LCD.print("B:");
    LCD.setCursor(3, 3);
    LCD.print(base_rotation);
    LCD.setCursor(9, 3);
    LCD.print("pH:");
    LCD.setCursor(13, 3);
    LCD.print(actual_ph, 1);
    if(millis() - last_refresh > 1000)
    {
      LCD.setCursor(3, 1);
      LCD.print("     ");
      LCD.setCursor(13, 1);
      LCD.print("     ");
      LCD.setCursor(3, 2);
      LCD.print("     ");
      LCD.setCursor(13, 2);
      LCD.print("     ");
      LCD.setCursor(3, 3);
      LCD.print("     ");
      LCD.setCursor(13, 3);
      LCD.print("     ");
      last_refresh = millis();
      actual_ph = getPhReading();
    }
}

// Execute the Servos movements -----------------
int getReadingFromPote(uint8_t pote_index)
{
  last_pote_readings[pote_index] = (0.1 * analogRead(pote_gpios[pote_index])) + (0.9 * last_pote_readings[pote_index]);
  return (int)last_pote_readings[pote_index];
}

void moverSuavemente(int servo, int init, int end)
{
  if(init < end)
  {
    for(int i = init ; i <= end ; i++)
    {
      PCA.writeMicroseconds(servo, i);
      delayMicroseconds(250);
    }
  } else {
    for(int i = init ; i >= end ; i--)
    {
      PCA.writeMicroseconds(servo, i);
      delayMicroseconds(250);
    }
  }
}

// Execute the Servos movements -----------------
void moveServosManual()
{
    last_micros_1 = actual_micros_1;
    last_micros_2 = actual_micros_2;
    last_micros_3 = actual_micros_3;
    last_micros_4 = actual_micros_4;
    actual_micros_1 = map(analogRead(POTE_1), 0, 1023, 500, 2500);
    actual_micros_2 = map(analogRead(POTE_2), 0, 1023, 500, 2500);
    actual_micros_3 = map(analogRead(POTE_3), 0, 1023, 500, 2500);
    actual_micros_4 = map(analogRead(POTE_4), 0, 1023, 500, 2500);
    moverSuavemente(0, last_micros_1, actual_micros_1);
    moverSuavemente(1, last_micros_2, actual_micros_2);
    moverSuavemente(2, last_micros_3, actual_micros_3);
    moverSuavemente(3, last_micros_4, actual_micros_4);
    actual_angle_1 = map(analogRead(POTE_1), 0, 1023, 0, 180);
    actual_angle_2 = map(analogRead(POTE_2), 0, 1023, 0, 180);
    actual_angle_3 = map(analogRead(POTE_3), 0, 1023, 0, 180);
    actuator_rotation = map(analogRead(POTE_4), 0, 1023, 0, 180);
    delay(100);
}

// Execute the Stepper motor movements ----------
void moveStepperManual()
{
    if(encoder_moved_right || encoder_moved_left)
    {
        digitalWrite(DIR, encoder_moved_right ? HIGH : LOW);
        bool continue_moving = true;
        while(continue_moving)
        {
          digitalWrite(STEP, HIGH);
          delay(25);
          digitalWrite(STEP, LOW);
          delay(25);
          stepper_steps += encoder_moved_right ? 1 : -1;
          base_rotation = getAngleFromSteps(stepper_steps);
          if(button_pressed) continue_moving = false;
        }
        digitalWrite(STEP, LOW);
        encoder_moved_right = false;
        encoder_moved_left = false;
        button_pressed = false;
    }
}


// Execute the actuator movement ----------------
void moveActuatorManual()
{
    if(digitalRead(ACTUATOR) == 1)
    {
        if(actuator_state == 0.0)
        {
            PCA.writeMicroseconds(4, 2500);
            digitalWrite(MOTOR, LOW);
            actuator_state = 1.0;
        } else {
            PCA.writeMicroseconds(4, 500);
            // TODO: Search if this variable can be constrain between 0 and 180 instead of 0 to 30 like the coords
            digitalWrite(MOTOR, map(actuator_state, 0, 30, 0, 255));
            actuator_state = 0.0;
        }
    }
}

// Get the actual reading and add it to the readings array
float getPhReading()
{
    ph_array[ph_index++] = analogRead(PH);
    if(ph_index == 10) ph_index = 0;
    float voltage = averagePhValue(ph_array, 10) * 5.0 / 1024.0;
    return 3.5 * voltage; // + offset (if needed)
}

// Calculate the average value from the readings array
float averagePhValue(int* values_array, int number)
{
    if(number <= 0) return 0;
    if(number < 5)
    {
        int result = 0;
        for(int i = 0 ; i < number ; i++)
            result += values_array[i];
        return (float)result / number;
    }
    int min_value = values_array[0], max_value = values_array[0];
    int result = 0;
    for(int i = 0 ; i < number ; i++)
    {
        result += values_array[i];
        if(values_array[i] < min_value) min_value = values_array[i];
        if(values_array[i] > max_value) max_value = values_array[i];
    }
    result -= (min_value + max_value);
    return (float)result / (number - 2);
}

// Store the actual position --------------------
void manualStorePosition()
{
    if(digitalRead(STORE) == 1)
    {
        storePosition(
            base_rotation,
            actual_micros_1,
            actual_micros_2,
            actual_micros_3,
            actual_micros_4,
            actuator_state
        );
    }
}

// Calculate the coordinates from the inpu readings
void calcCoordsFromAngles(
    uint16_t reading_0,
    uint16_t reading_1,
    uint16_t reading_2,
    float& x_coord,
    float& y_coord,
    float& z_coord
) {
    // Transform the given reading to Radians
    float angle_1 = reading_0 / 1023.0 * M_PI;
    float angle_2 = reading_1 / 1023.0 * M_PI;
    float angle_3 = reading_2 / 1023.0 * M_PI;
    float theta1 = angle_1;                      // Angle in Radians for Segment 1
    float theta2 = angle_2 - theta1;             // Angle in Radians for Segment 2
    float theta3 = angle_3 - theta2 - theta1;    // Angle in Radians for Segment 3
    // Calculate the coordinates of the arm end point
    x_coord = cos(theta3) * L3 + cos(theta2) * L2 + cos(theta1) * L1;
    y_coord = sin(theta3) * L3 + sin(theta2) * L2 + sin(theta1) * L1;
    z_coord = sin(theta1 + theta2 + theta3) * L3 + sin(theta1 + theta2) * L2 + sin(theta1) * L1;
}

// Displays the Automatic Mode Menu -------------
void showAutomaticMenu()
{
    LCD.setCursor(0, 0);
    LCD.print("Modo Automatico");
    LCD.setCursor(0, 1);
    LCD.print((cursor == 0) ? "> Ejec. movimiento" : "  Ejec. movimiento");
    LCD.setCursor(0, 2);
    LCD.print((cursor == 1) ? "> Ejec. movimientos" : "  Ejec. movimientos");
    LCD.setCursor(0, 3);
    LCD.print((cursor == 2) ? "> Ir a posicion" : "  Ir a posicion");
    if(encoder_moved_right)
    {
        cursor++;
        if(cursor > 2) cursor = 0;
        encoder_moved_right = false;
    }
    if(encoder_moved_left)
    {
        cursor--;
        if(cursor < 0) cursor = 2;
        encoder_moved_left = false;
    }
    if(button_pressed)
    {
        LCD.clear();
        button_pressed = false;
        showMenu(cursor);
    }
}

// Displays the corresponding Sub-Menu ----------
void showMenu(int menu_index)
{
    switch (menu_index)
    {
        case 0: showGoToPositionMenu(); break;
        case 1: showLaunchMovementsMenu(); break;
        case 2: showAskPositionMenu(); break;
    }
}

// Displays the Select position Menu ------------
void showGoToPositionMenu()
{
    cursor = 0;
    while(true)
    {
        LCD.setCursor(0, 0);
        LCD.print("Seleccione posicion:");
        if(positions_stored != 0)
        {
            LCD.setCursor(0, 1);
            LCD.print("ID:");
            LCD.setCursor(4, 1);
            LCD.print(cursor + 1);
            LCD.setCursor(10, 1);
            LCD.print("Ba:");
            LCD.setCursor(14, 1);
            LCD.print(map(positions_array[cursor][0], 500, 2500, 0, 180));
            LCD.setCursor(0, 2);
            LCD.print("A1:");
            LCD.setCursor(4, 2);
            LCD.print(map(positions_array[cursor][1], 500, 2500, 0, 180));
            LCD.setCursor(10, 2);
            LCD.print("A2:");
            LCD.setCursor(14, 2);
            LCD.print(map(positions_array[cursor][2], 500, 2500, 0, 180));
            LCD.setCursor(0, 3);
            LCD.print("A3:");
            LCD.setCursor(4, 3);
            LCD.print(map(positions_array[cursor][3], 500, 2500, 0, 180));
            LCD.setCursor(10, 3);
            LCD.print("Rt:");
            LCD.setCursor(14, 3);
            LCD.print(map(positions_array[cursor][4], 500, 2500, 0, 180));
            if(encoder_moved_right)
            {
                cursor++;
                if(cursor >= positions_stored) cursor = 0;
                encoder_moved_right = false;
            }
            if(encoder_moved_left)
            {
                cursor--;
                if(cursor < 0) cursor = positions_stored - 1;
                encoder_moved_left = false;
            }
        } else {
            LCD.setCursor(0, 3);
            LCD.print("  No hay guardadas");
        }
        if(button_pressed)
        {
            button_pressed = false;
            LCD.clear();
            if(positions_stored != 0) launchArmMovement(cursor);
            cursor = 0;
            break;
        }
    }
}

// Execute all stored movements -----------------
void launchArmMovement(int position_index)
{
    last_micros_1 = actual_micros_1;
    last_micros_2 = actual_micros_2;
    last_micros_3 = actual_micros_3;
    last_micros_4 = actual_micros_4;
    moveStepperAutomatic(positions_array[position_index][0]);
    moverSuavemente(0, last_micros_1, positions_array[position_index][1]);
    moverSuavemente(1, last_micros_2, positions_array[position_index][2]);
    moverSuavemente(2, last_micros_3, positions_array[position_index][3]);
    moverSuavemente(3, last_micros_4, positions_array[position_index][4]);
    moveActuatorAutomatic(positions_array[position_index][5]);
    actual_micros_1 = positions_array[position_index][1];
    actual_micros_2 = positions_array[position_index][2];
    actual_micros_3 = positions_array[position_index][3];
    actual_micros_4 = positions_array[position_index][4];
    actuator_state = positions_array[position_index][5];
}

// Displays the Menu to store a position --------
void showLaunchMovementsMenu()
{
    cursor = 0;
    while(true)
    {
        LCD.setCursor(0, 0);
        LCD.print("Recorrer todas");
        LCD.setCursor(0, 1);
        LCD.print("las posiciones");
        LCD.setCursor(0, 2);
        LCD.print("guardadas?");
        LCD.setCursor(15, 2);
        LCD.print("(");
        LCD.print(positions_stored);
        LCD.print(")");
        LCD.setCursor(8, 3);
        if(cursor == 0) LCD.print("> Si   No");
        else if(cursor == 1) LCD.print("  Si > No");
        if(encoder_moved_right)
        {
            cursor++;
            if(cursor > 1) cursor = 0;
            encoder_moved_right = false;
        }
        if(encoder_moved_left)
        {
            cursor--;
            if(cursor < 0) cursor = 1;
            encoder_moved_left = false;
        }
        if(button_pressed)
        {
            LCD.clear();
            if(cursor == 0) launchArmMovements();
            button_pressed = false;
            cursor = 0;
            break;
        }
    }
}

// Execute all stored movements -----------------
void launchArmMovements()
{
    for(int i = 0 ; i < positions_stored ; i++)
    {
      last_micros_1 = actual_micros_1;
      last_micros_2 = actual_micros_2;
      last_micros_3 = actual_micros_3;
      last_micros_4 = actual_micros_4;
      moveStepperAutomatic(positions_array[i][0]);
      moverSuavemente(0, last_micros_1, positions_array[i][1]);
      moverSuavemente(1, last_micros_2, positions_array[i][2]);
      moverSuavemente(2, last_micros_3, positions_array[i][3]);
      moverSuavemente(3, last_micros_4, positions_array[i][4]);
      moveActuatorAutomatic(positions_array[i][5]);
      actual_micros_1 = positions_array[i][1];
      actual_micros_2 = positions_array[i][2];
      actual_micros_3 = positions_array[i][3];
      actual_micros_4 = positions_array[i][4];
      actuator_state = positions_array[i][5];
      delay(500);
    }
}

// Displays a Menu to ask for Coordenates -------
void showAskPositionMenu()
{
    cursor = 0;
    int submenu = 0;
    float value = 0.0;
    float new_x_coord = 0.0;
    float new_y_coord = 0.0;
    float new_z_coord = 0.0;
    float new_actuator_state = 0.0;
    while(true)
    {
        LCD.setCursor(0, 0);
        LCD.print("Introduce valor de");
        LCD.setCursor(0, 1);
        LCD.print("las coordenadas ");
        LCD.setCursor(16, 1);
        if(submenu == 0) LCD.print("X");
        if(submenu == 1) LCD.print("Y");
        if(submenu == 2) LCD.print("Z");
        if(submenu == 3) LCD.print("A");
        LCD.setCursor(0, 2);
        LCD.print("X:");
        LCD.setCursor(3, 2);
        LCD.print(new_x_coord);
        LCD.setCursor(10, 2);
        LCD.print("Y:");
        LCD.setCursor(13, 2);
        LCD.print(new_y_coord);
        LCD.setCursor(0, 3);
        LCD.print("Z:");
        LCD.setCursor(3, 3);
        LCD.print(new_z_coord);
        LCD.setCursor(10, 3);
        LCD.print("A:");
        LCD.setCursor(13, 3);
        LCD.print(new_actuator_state);
        if(encoder_moved_right)
        {
            value += 0.5;
            if(value > 35.0) value = 35.0;
            encoder_moved_right = false;
        }
        if(encoder_moved_left)
        {
            value -= 0.5;
            if(value < 0.0) value = 0.0;
            encoder_moved_left = false;
        }
        if(submenu == 0) new_x_coord = value;
        if(submenu == 1) new_y_coord = value;
        if(submenu == 2) new_z_coord = value;
        if(submenu == 3) new_actuator_state = value;
        if(button_pressed)
        {
            LCD.clear();
            button_pressed = false;
            value = 0.0;
            submenu++;
            if(submenu > 3)
            {
                float new_x_angle, new_y_angle, new_z_angle, new_rotation;
                CI(
                    new_x_coord, new_y_coord, new_z_coord,
                    L1, L2, L3, 0, 180,
                    new_rotation, new_x_angle, new_y_angle, new_z_angle
                );
                moveStepperAutomatic(new_rotation);
                moveServosAutomatic(new_x_angle, new_y_angle, new_z_angle, actual_micros_4);
                moveActuatorAutomatic(new_actuator_state);
                actual_micros_1 = map(new_x_angle, 0, 180, 500, 2500);
                actual_micros_2 = map(new_y_angle, 0, 180, 500, 2500);
                actual_micros_3 = map(new_z_angle, 0, 180, 500, 2500);
                actuator_state = new_actuator_state;
                storePosition(
                    base_rotation,
                    actual_micros_1,
                    actual_micros_2,
                    actual_micros_3,
                    actual_micros_4,
                    actuator_state
                );
                break;
            }
        }
    }
}

// Execute the Servos movements -----------------
void moveServosAutomatic(float angle_1, float angle_2, float angle_3, float angle_4)
{
    last_micros_1 = actual_micros_1;
    last_micros_2 = actual_micros_2;
    last_micros_3 = actual_micros_3;
    last_micros_4 = actual_micros_4;
    actual_micros_1 = map(angle_1, 0, 180, 500, 2500);
    actual_micros_2 = map(angle_2, 0, 180, 500, 2500);
    actual_micros_3 = map(angle_3, 0, 180, 500, 2500);
    actual_micros_4 = angle_4;
    moverSuavemente(0, last_micros_1, actual_micros_1);
    moverSuavemente(1, last_micros_2, actual_micros_2);
    moverSuavemente(2, last_micros_3, actual_micros_3);
    moverSuavemente(3, last_micros_4, actual_micros_4);
    delay(100);
}

// Execute the Stepper motor movements ----------
void moveStepperAutomatic(float angle)
{
    float target_steps = getStepsFromAngle(angle);
    float steps_to_move = target_steps - stepper_steps;
    bool move_right = steps_to_move > 0;
    digitalWrite(DIR, move_right ? HIGH : LOW);
    for(int i = 0 ; i < abs(steps_to_move) ; i++)
    {
      digitalWrite(STEP, HIGH);
      delay(25);
      digitalWrite(STEP, LOW);
      delay(25);
      stepper_steps += move_right ? 1 : -1;
      base_rotation = getAngleFromSteps(stepper_steps);
    }
}

// Execute the actuator movement ----------------
void moveActuatorAutomatic(float state)
{
    if(state == 1.0)
    {
        PCA.writeMicroseconds(4, 2500);
        digitalWrite(MOTOR, LOW);
        actuator_state = 1.0;
    } else {
        PCA.writeMicroseconds(4, 500);
        // TODO: Search if this variable can be constrain between 0 and 180 instead of 0 to 30 like the coords
        digitalWrite(MOTOR, map(state, 0, 30, 0, 255));
        actuator_state = 0.0;
    }
}

// Convert the given steps to an angle ----------
float getAngleFromSteps(int steps)
{
    return 1.8 * steps;
}

// Convert the given angle to steps ----------
float getStepsFromAngle(int angle)
{
    return angle / 1.8;
}

// Store a position in the positions array ------
void storePosition(float b, float x, float y, float z, float r, float a)
{
    positions_array[positions_stored][0] = b;
    positions_array[positions_stored][1] = x;
    positions_array[positions_stored][2] = y;
    positions_array[positions_stored][3] = z;
    positions_array[positions_stored][4] = r;
    positions_array[positions_stored][5] = a;
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("Posicion guardada");
    LCD.setCursor(0, 1);
    LCD.print("con ID: ");
    LCD.setCursor(7, 1);
    LCD.print(positions_stored + 1);
    positions_stored++;
    cursor = 0;
    delay(2000);
    LCD.clear();
}

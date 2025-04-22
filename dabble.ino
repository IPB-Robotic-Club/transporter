/*
  This code is designed for use with the Dabble mobile application by STEMpedia, Arduino ESP Bluetooth - Dabble
  Dabble is a Bluetooth-based control app that allows users to send commands from a smartphone to Arduino-based robots.
  Make sure to install the Dabble library from STEMpedia and pair your Bluetooth module (like HC-05 or HC-06) with your phone.
*/


#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <ESP32Servo.h>

// Right motor
int enableRightMotor = 12;
int rightMotorPin1 = 14;
int rightMotorPin2 = 27;

// Left motor
int enableLeftMotor = 33;
int leftMotorPin1 = 26;
int leftMotorPin2 = 25;

// Gripper servos
const int servoPin1 = 15; // Gripper servo
const int servoPin2 = 5;  // Lift servo
Servo servo1;
Servo servo2;

bool precisionMode = false; // Precision mode flag

// Servo positions and limits
int position1 = 130; // Initial gripper position
int position2 = 30;  // Initial lift position
const int GRIPPER_OPEN = 130;
const int GRIPPER_CLOSED = 30;
const int LIFT_UP = 30;
const int LIFT_DOWN = 135;
const int LIFT_DOWN1 = 120;
const int SERVO_DELAY = 15;
int UP = 30;
int BOTTOM = 135;

// Motor settings
#define MAX_MOTOR_SPEED 255
const int PWMFreq = 1000;
const int PWMResolution = 8;
const int rightMotorPWMSpeedChannel = 4;
const int leftMotorPWMSpeedChannel = 5;

// Joystick deadzone threshold
const float JOYSTICK_THRESHOLD = 2.0;

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
    if (precisionMode)
    {
        rightMotorSpeed = rightMotorSpeed * 0.7;
        leftMotorSpeed = leftMotorSpeed * 0.7;
    }

    // Right motor direction
    if (rightMotorSpeed < 0)
    {
        digitalWrite(rightMotorPin1, LOW);
        digitalWrite(rightMotorPin2, HIGH);
    }
    else if (rightMotorSpeed > 0)
    {
        digitalWrite(rightMotorPin1, HIGH);
        digitalWrite(rightMotorPin2, LOW);
    }
    else
    {
        digitalWrite(rightMotorPin1, LOW);
        digitalWrite(rightMotorPin2, LOW);
    }

    // Left motor direction
    if (leftMotorSpeed < 0)
    {
        digitalWrite(leftMotorPin1, LOW);
        digitalWrite(leftMotorPin2, HIGH);
    }
    else if (leftMotorSpeed > 0)
    {
        digitalWrite(leftMotorPin1, HIGH);
        digitalWrite(leftMotorPin2, LOW);
    }
    else
    {
        digitalWrite(leftMotorPin1, LOW);
        digitalWrite(leftMotorPin2, LOW);
    }

    ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
    ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));
}

void moveServoSlowly(Servo &servo, int startPos, int endPos, int stepDelay)
{
    if (startPos < endPos)
    {
        for (int pos = startPos; pos <= endPos; pos += 4)
        {
            servo.write(pos);
            delay(stepDelay);
        }
    }
    else
    {
        for (int pos = startPos; pos >= endPos; pos -= 4)
        {
            servo.write(pos);
            delay(stepDelay);
        }
    }
}

void setUpPinModes()
{
    pinMode(enableRightMotor, OUTPUT);
    pinMode(rightMotorPin1, OUTPUT);
    pinMode(rightMotorPin2, OUTPUT);

    pinMode(enableLeftMotor, OUTPUT);
    pinMode(leftMotorPin1, OUTPUT);
    pinMode(leftMotorPin2, OUTPUT);

    ledcSetup(rightMotorPWMSpeedChannel, PWMFreq, PWMResolution);
    ledcSetup(leftMotorPWMSpeedChannel, PWMFreq, PWMResolution);
    ledcAttachPin(enableRightMotor, rightMotorPWMSpeedChannel);
    ledcAttachPin(enableLeftMotor, leftMotorPWMSpeedChannel);

    rotateMotor(0, 0);
}

void setup()
{
    Serial.begin(115200);
    setUpPinModes();
    Dabble.begin("GalaxyA35");

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);

    servo1.attach(servoPin1, 500, 2400);
    servo2.attach(servoPin2, 500, 2400);

    servo1.write(position1);
    servo2.write(position2);
}

void loop()
{
    int rightMotorSpeed = 0;
    int leftMotorSpeed = 0;

    Dabble.processInput();

    // Joystick control
    float xAxis = GamePad.getXaxisData();
    float yAxis = GamePad.getYaxisData();

    // Calculate motor speeds based on joystick position
    if (abs(xAxis) > JOYSTICK_THRESHOLD || abs(yAxis) > JOYSTICK_THRESHOLD)
    {
        float forwardSpeedMultiplier = MAX_MOTOR_SPEED / 7.0;      
        float turnSpeedMultiplier = (MAX_MOTOR_SPEED / 7.0) * 0.5; //for turning

        // Separate calculations for forward/backward and turning
        float forwardComponent = yAxis * forwardSpeedMultiplier;
        float turnComponent = xAxis * turnSpeedMultiplier;

        // Combine components
        rightMotorSpeed = forwardComponent - turnComponent;
        leftMotorSpeed = forwardComponent + turnComponent;

        // Ensure speeds don't exceed limits
        rightMotorSpeed = constrain(rightMotorSpeed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
        leftMotorSpeed = constrain(leftMotorSpeed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    }

    // Button controls (original)
    if (GamePad.isUpPressed())
    {
        rightMotorSpeed = MAX_MOTOR_SPEED;
        leftMotorSpeed = MAX_MOTOR_SPEED;
    }
    if (GamePad.isDownPressed())
    {
        rightMotorSpeed = -MAX_MOTOR_SPEED;
        leftMotorSpeed = -MAX_MOTOR_SPEED;
    }
    if (GamePad.isLeftPressed())
    {
        rightMotorSpeed = MAX_MOTOR_SPEED;
        leftMotorSpeed = -MAX_MOTOR_SPEED;
    }
    if (GamePad.isRightPressed())
    {
        rightMotorSpeed = -MAX_MOTOR_SPEED;
        leftMotorSpeed = MAX_MOTOR_SPEED;
    }

    // Gripper controls
    if (GamePad.isSquarePressed())
    {
        servo1.write(GRIPPER_OPEN);
        position1 = GRIPPER_OPEN;
        Serial.println("Gripper opened instantly");
    }

    if (GamePad.isCirclePressed())
    {
        moveServoSlowly(servo1, position1, GRIPPER_CLOSED, SERVO_DELAY);
        position1 = GRIPPER_CLOSED;
    }

    if (GamePad.isTrianglePressed())
    {
        servo2.write(LIFT_UP);
        position2 = LIFT_UP;
        Serial.println("Gripper lifted instantly");
    }

    if (GamePad.isCrossPressed())
    {
        moveServoSlowly(servo2, position2, LIFT_DOWN, SERVO_DELAY);
        position2 = LIFT_DOWN;
    }

    if (GamePad.isStartPressed())
    {
        moveServoSlowly(servo1, position1, GRIPPER_CLOSED, SERVO_DELAY);
        position1 = GRIPPER_CLOSED;
        servo2.write(LIFT_UP);
        position2 = LIFT_UP;
        Serial.println("Gripper lifted instantly");
    }

    if (GamePad.isSelectPressed())
    {
        precisionMode = !precisionMode;
        Serial.printf("Precision mode: %s\n", precisionMode ? "ON" : "OFF");
    }

    // Debug joystick values
    Serial.print("X: ");
    Serial.print(xAxis);
    Serial.print("\tY: ");
    Serial.print(yAxis);
    Serial.print("\tRight Motor: ");
    Serial.print(rightMotorSpeed);
    Serial.print("\tLeft Motor: ");
    Serial.println(leftMotorSpeed);

    rotateMotor(rightMotorSpeed, leftMotorSpeed);
}

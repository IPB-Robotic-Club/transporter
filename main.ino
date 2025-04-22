#include <ESP32Servo.h>
#include <Ps3Controller.h>

// Motor pins
int enableRightMotor = 33;
int rightMotorPin1 = 26;
int rightMotorPin2 = 25;
int enableLeftMotor = 12;
int leftMotorPin1 = 14;
int leftMotorPin2 = 27;

// Servo pins
const int servoPin1 = 19; // Gripper servo
const int servoPin2 = 5;  // Lift servo
Servo servo1;
Servo servo2;

bool gripperOpen = false; // Gripper status (open or closed)
bool liftUp = false;      // Lift status (up or down)

// Servo positions and limits
int gripperPosition = 55; // Initial gripper position
int liftPosition = 45;    // Initial lift position
const int GRIPPER_OPEN = 115;
const int GRIPPER_CLOSED = 55;
const int LIFT_UP = 45;
const int LIFT_DOWN = 145;
const int SERVO_DELAY = 15;
const int LIFT_DOWN1 = 125;

const int slowSpeed = 150; // Slow speed for motors

// Motor settings
#define MAX_MOTOR_SPEED 255
const int PWMFreq = 1000;
const int PWMResolution = 8;
const int rightMotorPWMSpeedChannel = 4;
const int leftMotorPWMSpeedChannel = 5;
const float STICK_THRESHOLD = 0.1;

// Additional variables for enhanced control
int currentSpeed = MAX_MOTOR_SPEED; // Variable speed control
bool precisionMode = false;         // Precision mode flag
int battery = 0;                    // Battery level tracking
unsigned long lastMoveTime = 0;     // For movement timeout
const int MOVE_TIMEOUT = 400;       // Movement timeout in ms

void handleMovement()
{
    float x = (float)Ps3.data.analog.stick.lx / 128.0;
    float y = -(float)Ps3.data.analog.stick.ly / 128.0;

    // Check if the stick is in a neutral position (close to 0)
    if (abs(x) < STICK_THRESHOLD && abs(y) < STICK_THRESHOLD)
    {
        // Stick released, stop the motors
        rotateMotor(0, 0);
        return;
    }

    // Convert to tank drive
    float leftMotorSpeed = y + x;
    float rightMotorSpeed = y - x;

    // Normalize speeds
    float maxSpeed = max(abs(leftMotorSpeed), abs(rightMotorSpeed));
    if (maxSpeed > 1.0)
    {
        leftMotorSpeed /= maxSpeed;
        rightMotorSpeed /= maxSpeed;
    }

    // Apply current speed setting
    rotateMotor(rightMotorSpeed * currentSpeed, leftMotorSpeed * currentSpeed);
}

void notify()
{

    handleMovement();

    // Gripper controls
    if (Ps3.event.button_down.left)
    {
        moveServoSlowly(servo1, gripperPosition, GRIPPER_CLOSED, SERVO_DELAY);
        gripperPosition = GRIPPER_CLOSED;
    }
    if (Ps3.event.button_down.right)
    {
        servo1.write(GRIPPER_OPEN);
        gripperPosition = GRIPPER_OPEN;
    }

    // Lift controls
    if (Ps3.event.button_down.up)
    {
        moveServoSlowly(servo2, liftPosition, LIFT_UP, SERVO_DELAY);
        liftPosition = LIFT_UP;
    }
    if (Ps3.event.button_down.down)
    {
        moveServoSlowly(servo2, liftPosition, LIFT_DOWN, SERVO_DELAY);
        liftPosition = LIFT_DOWN;
    }

    if (Ps3.event.button_down.cross)
    {
        if (currentSpeed == slowSpeed)
        {
            // If the current speed is slow, set to maximum
            currentSpeed = MAX_MOTOR_SPEED;
            Serial.println("Speed set to maximum");
        }
        else
        {
            // If the current speed is maximum, set to slow
            currentSpeed = slowSpeed;
            Serial.println("Speed set to slow");
        }
    }
    if (Ps3.event.button_down.square)
    {
        moveServoSlowly(servo2, liftPosition, LIFT_DOWN, SERVO_DELAY);
        liftPosition = LIFT_DOWN;
        servo1.write(GRIPPER_OPEN);
        gripperPosition = GRIPPER_OPEN;
    }
    if (Ps3.event.button_down.triangle)
    {
        moveServoSlowly(servo1, gripperPosition, GRIPPER_CLOSED, SERVO_DELAY);
        gripperPosition = GRIPPER_CLOSED;
        moveServoSlowly(servo2, liftPosition, LIFT_UP, SERVO_DELAY);
        liftPosition = LIFT_UP;
    }
    // Rotation
    if (Ps3.event.button_down.circle)
    {
        rotateMotor(MAX_MOTOR_SPEED, -MAX_MOTOR_SPEED);
    }

    if (Ps3.event.button_down.l1)
    {
        Serial.println("Started pressing the left trigger button");
        toggleGripper();
    }
    if (Ps3.event.button_down.r1)
    {
        Serial.println("Started pressing the right trigger button");
        toggleLift();
    }

    if (Ps3.event.button_down.l2)
    {
        precisionMode = !precisionMode;
        Serial.printf("Precision mode: %s\n", precisionMode ? "ON" : "OFF");
    }

    // Precision mode
    if (Ps3.event.button_down.r2)
    {
        Serial.println("Started pressing the right trigger button to 5 cm");
        toggleLift1();
    }

    // Battery monitoring
    if (battery != Ps3.data.status.battery)
    {
        battery = Ps3.data.status.battery;
        Serial.print("Battery: ");
        if (battery == ps3_status_battery_charging)
            Serial.println("Charging");
        else if (battery == ps3_status_battery_full)
            Serial.println("Full");
        else if (battery == ps3_status_battery_high)
            Serial.println("High");
        else if (battery == ps3_status_battery_low)
            Serial.println("Low");
        else if (battery == ps3_status_battery_dying)
            Serial.println("Critical");
        else if (battery == ps3_status_battery_shutdown)
            Serial.println("Shutting down");
    }
}

void stopMotors()
{
    digitalWrite(leftMotorPin1, 0);
    digitalWrite(leftMotorPin2, 0);
    digitalWrite(rightMotorPin1, 0);
    digitalWrite(rightMotorPin2, 0);
}

// Function to toggle the gripper (open and close the gripper)
void toggleGripper()
{
    if (!gripperOpen)
    {
        moveServoSlowly(servo1, gripperPosition, GRIPPER_OPEN, SERVO_DELAY);
        gripperOpen = true;
    }
    else
    {
        moveServoSlowly(servo1, gripperPosition, GRIPPER_CLOSED, SERVO_DELAY);
        gripperOpen = false;
    }
}

// Function to toggle the lift (raise and lower the lift)
void toggleLift1()
{
    if (!liftUp)
    {
        moveServoSlowly(servo2, liftPosition, LIFT_UP, SERVO_DELAY);
        liftUp = true;
    }
    else
    {
        moveServoSlowly(servo2, liftPosition, LIFT_DOWN1, SERVO_DELAY);
        liftUp = false;
    }
}

// Function to toggle the lift (raise and lower the lift)
void toggleLift()
{
    if (!liftUp)
    {
        moveServoSlowly(servo2, liftPosition, LIFT_UP, SERVO_DELAY);
        liftUp = true;
    }
    else
    {
        moveServoSlowly(servo2, liftPosition, LIFT_DOWN, SERVO_DELAY);
        liftUp = false;
    }
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
    // Apply precision mode if enabled
    if (precisionMode)
    {
        rightMotorSpeed = rightMotorSpeed * 0.5;
        leftMotorSpeed = leftMotorSpeed * 0.5;
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

    // Set motor speeds
    ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
    ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));
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

void onConnect()
{
    Serial.println("PS3 Controller Connected.");
    currentSpeed = MAX_MOTOR_SPEED;
    precisionMode = false;
}

void setup()
{
    Serial.begin(115200);
    setUpPinModes();

    // Servo setup
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo1.attach(servoPin1, 500, 2400);
    servo2.attach(servoPin2, 500, 2400);

    servo1.write(gripperPosition);
    servo2.write(liftPosition);

    // PS3 Controller setup
    Ps3.attach(notify);
    Ps3.attachOnConnect(onConnect);
    Ps3.begin("c0:5d:89:b1:55:02"); // Replace with your ESP32's MAC address

    Serial.println("Setup completed. Waiting for PS3 controller connection...");
}

void loop()
{
    if (!Ps3.isConnected())
    {
        rotateMotor(0, 0); // Stop motors if controller disconnected
        return;
    }

    // Movement timeout - stop motors if no input received
    if (millis() - lastMoveTime > MOVE_TIMEOUT)
    {
        rotateMotor(0, 0);
    }

    delay(20); // Small delay to prevent overwhelming the system
}

/*
 * Arduino Wireless Communication Tutorial
 * Example 1 - Receiver Code
 *
 * by Dejan Nedelkovski, www.HowToMechatronics.com
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#include <Arduino.h>
#include <constants.h>
#include <Servo.h>
#include <printf.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(constantsPinsShip::cePin, constantsPinsShip::csnPin); // CE, CSN
Servo servo;

const byte address[6] = "00005";

// Shared variable to keep track of steering for the motor mixer (127 is center)
uint8_t currentSteerValue = 127;
uint8_t currentThrottleValue = 127;

bool readRadio(uint8_t *buffer, RF24 *radio);
bool getData(uint8_t *buffer, uint8_t *steer, uint8_t *throttle, bool *foilStab);
void changeFoilStab(bool foilStab);
void changeSteer(uint8_t steer);
void changeThrottle(uint8_t throttle);

void setup()
{
    Serial.begin(9600);
    printf_begin(); // Initialize printf functionality for Serial
    // radio.setDataRate(RF24_250KBPS);
    if (!radio.begin())
    {
        Serial.println("Hardware not responding! Check your wiring.");
        radio.printDetails();
        while (1)
            ; // Stop here if radio isn't found
    }
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MAX);
    radio.startListening();
    Serial.println("Receiver Initialized...");

    pinMode(constantsPinsShip::foilPin, OUTPUT);
    pinMode(constantsPinsShip::servoSteerPin, OUTPUT);
    pinMode(constantsPinsShip::in1Pin, OUTPUT);
    pinMode(constantsPinsShip::in2Pin, OUTPUT);
    pinMode(constantsPinsShip::in3Pin, OUTPUT);
    pinMode(constantsPinsShip::in4Pin, OUTPUT);
    servo.attach(constantsPinsShip::servoSteerPin);

    // Steering servo calibration test: left -> right -> center
    servo.write(70);
    delay(500);
    servo.write(110);
    delay(500);
    servo.write(90);
    delay(500);
    Serial.println("Calibration complete.");
}

void loop()
{
    static long lastKeepAlive = 0;
    static bool foilStab = false;
    uint8_t steer = 0;
    uint8_t throttle = 0;
    static uint8_t buffer[constantsCom::bufferSize];

    memset(buffer, 0, sizeof(buffer));
    bool read = readRadio(buffer, &radio);
    bool tempFoilStab = foilStab;
    bool keepAlive = getData(buffer, &steer, &throttle, &tempFoilStab);
    Serial.println(lastKeepAlive);

    if ((millis() - lastKeepAlive) > constantsShip::keepAliveTime)
    {
        currentSteerValue = 127;
        currentThrottleValue = 127; // Reset saved throttle on disconnect
        changeSteer(127);
        changeThrottle(127);
        while (true)
        {
            readRadio(buffer, &radio);
            keepAlive = getData(buffer, &steer, &throttle, &tempFoilStab);
            if (keepAlive)
            {
                break;
            }
            delay(100);
        }
    }

    if (!read)
    {
        return;
    }

    if (keepAlive)
    {
        lastKeepAlive = millis();
    }
    if (tempFoilStab != foilStab)
    {
        foilStab = tempFoilStab;
        changeFoilStab(foilStab);
    }

    // Flag to check if we need to update the motors this frame
    bool activeUpdateNeeded = false;

    // Process Steering Packet
    if (steer != 0)
    {
        steer = constrain(steer, constantsCom::steerMin, constantsCom::steerMax);
        steer = map(steer, constantsCom::steerMin, constantsCom::steerMax, 0, 255);

        currentSteerValue = steer; // Save to global
        changeSteer(steer);        // Move physical servo
        activeUpdateNeeded = true; // Trigger motor remix
    }

    // Process Throttle Packet
    if (throttle != 0)
    {
        throttle = constrain(throttle, constantsCom::throttleMin, constantsCom::throttleMax);
        throttle = map(throttle, constantsCom::throttleMin, constantsCom::throttleMax, 0, 255);

        currentThrottleValue = throttle; // Save to global
        activeUpdateNeeded = true;       // Trigger motor remix
    }

    // If either steering OR throttle changed, update the motors together
    if (activeUpdateNeeded)
    {
        changeThrottle(currentThrottleValue);
    }
}

bool readRadio(uint8_t *buffer, RF24 *radio)
{
    if (radio->available())
    {
        radio->read(buffer, constantsCom::bufferSize);
        Serial.println("Received: " + String(buffer[0], HEX) + "|" + String(buffer[1], HEX) + "|" + String(buffer[2], HEX));
    }
    else
        return false;
    if (buffer[0] == 0)
        return false;
    return true;
}

bool getData(uint8_t *buffer, uint8_t *steer, uint8_t *throttle, bool *foilStab)
{
    int i = 0;
    bool keepAlive = false;
    while (buffer[i] != 0)
    {
        if (i == constantsCom::bufferSize)
            break;
        if (buffer[i] == 1)
        {
            keepAlive = true;
        }
        else if (buffer[i] >= constantsCom::steerMin && buffer[i] <= constantsCom::steerMax)
            *steer = buffer[i];
        else if (buffer[i] == constantsCom::foilStabToggle)
            *foilStab = !(*foilStab);
        else if (constantsCom::throttleToggle)
        {
            if (buffer[i] == constantsCom::throttleOn)
                *throttle = constantsCom::throttleMax;
            if (buffer[i] == constantsCom::throttleOff)
                *throttle = (constantsCom::throttleMin + constantsCom::throttleMax) / 2;
            if (buffer[i] == constantsCom::throttleRev)
                *throttle = constantsCom::throttleMin;
        }
        else if (buffer[i] >= constantsCom::throttleMin && buffer[i] <= constantsCom::throttleMax)
            *throttle = buffer[i];
        i++;
    }
    return keepAlive;
}

void changeFoilStab(bool foilStab)
{
    digitalWrite(constantsPinsShip::foilPin, foilStab);
}

void changeSteer(uint8_t steer)
{
    steer = map(steer, 0, 255, (constantsShip::steerMidpoint - constantsShip::steerAngleMax), (constantsShip::steerMidpoint + constantsShip::steerAngleMax));
    steer = constrain(steer, 0, 180);
    servo.write(steer);
}

void changeThrottle(uint8_t throttle)
{
    // 1. Deadzone Handling (Stop everything if centered)
    if (throttle >= 126 && throttle <= 128)
    {
        analogWrite(constantsPinsShip::in1Pin, 0);
        analogWrite(constantsPinsShip::in2Pin, 0);
        analogWrite(constantsPinsShip::in3Pin, 0);
        analogWrite(constantsPinsShip::in4Pin, 0);
        return;
    }

    // 2. Turn currentSteerValue into a normalized factor from -1.0 to 1.0
    // 127 is center (0.0). 0 is hard left (-1.0). 255 is hard right (1.0).
    float steerModifier = (currentSteerValue - 127.0) / 127.0;

    // Adjust steering strength (0.0 to 1.0)
    // 0.7 means the inside motor can drop by up to 70% of its power during a hard turn
    float steerStrength = constantsShip::steeringBalance;

    // 3. Forward Logic
    if (throttle > 128)
    {
        int baseThrottle = map(throttle, 127, 255, 0, 255);
        int leftThrottle = baseThrottle;
        int rightThrottle = baseThrottle;

        if (steerModifier > 0)
        {
            // Turning Right: Keep Left motor at max base power, slow down Right motor
            rightThrottle = baseThrottle * (1.0 - (steerModifier * steerStrength));
        }
        else if (steerModifier < 0)
        {
            // Turning Left: Keep Right motor at max base power, slow down Left motor
            // (Using fabs() or multiplying by -1 to keep the math positive)
            leftThrottle = baseThrottle * (1.0 - (-steerModifier * steerStrength));
        }

        // Apply your original physical motor balance calibration to the right side
        rightThrottle = rightThrottle * constantsShip::motorBalance;

        // Keep values safely bounded in PWM limits
        leftThrottle = constrain(leftThrottle, 0, 255);
        rightThrottle = constrain(rightThrottle, 0, 255);

        analogWrite(constantsPinsShip::in1Pin, leftThrottle);
        analogWrite(constantsPinsShip::in2Pin, 0);
        analogWrite(constantsPinsShip::in3Pin, rightThrottle);
        analogWrite(constantsPinsShip::in4Pin, 0);
    }
    // 4. Reverse Logic
    else if (throttle < 126)
    {
        int baseThrottle = map(throttle, 125, 0, 0, 255);
        int leftThrottle = baseThrottle;
        int rightThrottle = baseThrottle;

        if (steerModifier > 0)
        {
            // Reversing and Steering Right: Slow down Left motor, Right motor at max base power
            leftThrottle = baseThrottle * (1.0 - (steerModifier * steerStrength));
        }
        else if (steerModifier < 0)
        {
            // Reversing and Steering Left: Slow down Right motor, Left motor at max base power
            rightThrottle = baseThrottle * (1.0 - (-steerModifier * steerStrength));
        }

        rightThrottle = rightThrottle * constantsShip::motorBalance;

        leftThrottle = constrain(leftThrottle, 0, 255);
        rightThrottle = constrain(rightThrottle, 0, 255);

        analogWrite(constantsPinsShip::in1Pin, 0);
        analogWrite(constantsPinsShip::in2Pin, leftThrottle);
        analogWrite(constantsPinsShip::in3Pin, 0);
        analogWrite(constantsPinsShip::in4Pin, rightThrottle);
    }
}
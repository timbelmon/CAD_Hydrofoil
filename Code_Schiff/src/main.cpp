/*
 * Arduino Wireless Communication Tutorial
 *       Example 1 - Receiver Code
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

const byte address[6] = "00001";

bool readRadio(uint8_t *buffer, RF24 *radio);
bool getData(uint8_t *buffer, uint8_t *steer, uint8_t *throttle, bool *foilStab);
void changeFoilStab(bool foilStab);
void changeSteer(uint8_t steer);
void changeThrottle(uint8_t throttle);

void setup()
{
    Serial.begin(9600);
    printf_begin(); // Initialize printf functionality for Serial
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
    servo.write(0);
    delay(500);
    servo.write(180);
    delay(500);
    servo.write(90);
    delay(500);
    Serial.println("Calibration complete.");
}

void loop()
{
    static bool foilStab = false;
    uint8_t steer = 0;
    uint8_t throttle = 0;
    static uint8_t buffer[constantsCom::bufferSize];

    // Check for serial input to manually control steering
    /*
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        int steerVal = input.toInt();
        if (steerVal >= 0 && steerVal <= 180)
        {
            changeSteer(steerVal);
            delay(100); // Small delay to ensure servo has time to move
            Serial.println("Manual steer: " + String(steerVal));
        }
    }
    */

    memset(buffer, 0, sizeof(buffer));
    bool read = readRadio(buffer, &radio);
    if (!read)
    {
        return;
    }
    bool tempFoilStab = foilStab;
    bool keepAlive = getData(buffer, &steer, &throttle, &tempFoilStab);
    if (tempFoilStab != foilStab)
    {
        foilStab = tempFoilStab;
        changeFoilStab(foilStab);
    }
    if (steer != 0)
    {
        steer = constrain(steer, constantsCom::steerMin, constantsCom::steerMax);
        steer = map(steer, constantsCom::steerMin, constantsCom::steerMax, 0, 255);
        changeSteer(steer);
    }
    if (throttle != 0)
    {
        throttle = constrain(throttle, constantsCom::throttleMin, constantsCom::throttleMax);
        throttle = map(throttle, constantsCom::throttleMin, constantsCom::throttleMax, 0, 255);
        changeThrottle(throttle);
    }
}

bool readRadio(uint8_t *buffer, RF24 *radio)
{
    if (radio->available())
    {
        radio->read(buffer, constantsCom::bufferSize);
        Serial.println("Received: "+ String(buffer[0], HEX)+"|"+ String(buffer[1], HEX)+"|"+ String(buffer[2], HEX));
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
        else if (buffer[i] == constantsCom::foilStabOn)
            *foilStab = true;
        else if (buffer[i] == constantsCom::foilStabOff)
            *foilStab = false;
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
    if (throttle >= 126 && throttle <= 128)
    {
        analogWrite(constantsPinsShip::in1Pin, 0);
        analogWrite(constantsPinsShip::in2Pin, 0);
        analogWrite(constantsPinsShip::in3Pin, 0);
        analogWrite(constantsPinsShip::in4Pin, 0);
    }
    else if (throttle > 128)
    {
        throttle = map(throttle, 127, 255, 0, 255);
        analogWrite(constantsPinsShip::in1Pin, throttle);
        analogWrite(constantsPinsShip::in2Pin, 0);
        analogWrite(constantsPinsShip::in3Pin, throttle);
        analogWrite(constantsPinsShip::in4Pin, 0);
    }
    else if (throttle < 126)
    {
        throttle = map(throttle, 125, 0, 0, 255);
        analogWrite(constantsPinsShip::in1Pin, 0);
        analogWrite(constantsPinsShip::in2Pin, throttle);
        analogWrite(constantsPinsShip::in3Pin, 0);
        analogWrite(constantsPinsShip::in4Pin, throttle);
    }
}
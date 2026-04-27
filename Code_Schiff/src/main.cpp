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
#include <printf.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

bool getData(uint8_t *buffer, uint8_t *steer, uint8_t *throttle, uint8_t *foilStab);
bool readRadio(uint8_t *buffer, RF24 *radio);

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
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    Serial.println("Receiver Initialized...");
}

void loop()
{
    uint8_t *buffer = (uint8_t *)calloc(constants::bufferSize, sizeof(uint8_t));
}

bool readRadio(uint8_t *buffer, RF24 *radio)
{
    if (radio->available())
    {
        radio->read(buffer, sizeof(buffer));
    }
    else
        return false;
    return true;
}

bool getData(uint8_t *buffer, uint8_t *steer, uint8_t *throttle, bool *foilStab)
{
    int i = 0;
    bool keepAlive = false;
    while (buffer[i] != 0)
    {
        if (i = constants::bufferSize)
            return;
        if (buffer[i] = 1)
        {
            keepAlive = true;
        }
        else if (buffer[i] >= constants::steerMin && buffer[i] <= constants::steerMax)
            *steer = buffer[i];
        else if (buffer[i] == constants::foilStabOn)
            *foilStab = true;
        else if (buffer[i] == constants::foilStabOff)
            *foilStab = false;
        else if (constants::throttleToggle)
        {
            if (buffer[i] == 2)
                *throttle = constants::throttleMax;
            if (buffer[i] == 3)
                *throttle = constants::throttleMin;
        }
        else if (buffer[i] >= constants::throttleMin && buffer[i] <= constants::throttleMax)
            *throttle = buffer[i];
        i++;
    }
    return keepAlive;
}
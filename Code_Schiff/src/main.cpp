/*
 * Arduino Wireless Communication Tutorial
 *       Example 1 - Receiver Code
 *
 * by Dejan Nedelkovski, www.HowToMechatronics.com
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */
#include <Arduino.h>
#include <printf.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

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
    if (radio.available())
    {
        char text[32] = {0}; // Initialize with all zeros
        radio.read(&text, sizeof(text));

        // Only print if there is actually content
        if (strlen(text) > 0)
        {
            Serial.print("Received: ");
            Serial.println(text);
        }
    }
}
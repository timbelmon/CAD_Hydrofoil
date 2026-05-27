/*
 * Arduino Wireless Communication Tutorial
 *     Example 1 - Transmitter Code
 *
 * by Dejan Nedelkovski, www.HowToMechatronics.com
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */

#include <Arduino.h>
#include <Print.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <constants.h>

#define THRESHOLD 10 // change if needed

// Radio module setup
RF24 radio(constantsPinsRemote::cePin, constantsPinsRemote::csnPin);

const byte address[6] = "00001";

// Buffer for sending data
uint8_t buffer[constantsCom::bufferSize];

// Pins
const int joyX = constantsPinsRemote::joyXPin;
const int speedPin = constantsPinsRemote::speedPin;
const int foilPin = constantsPinsRemote::foilPin;

// safe data for sending
int lastX = 0;
int lastSpeed = 0;
bool lastbutton = false;

void setup()
{
  Serial.begin(9600);

  if (!radio.begin())
  {
    Serial.println("Radio error!");
    while (1)
      ;
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.printDetails();
  if (!radio.begin())
  {
    Serial.println("Hardware not responding! Check your wiring.");
    radio.printDetails();
    while (true)
    {
      /* code */
    }
  }
  Serial.println("Remote Initialized...");

  pinMode(foilPin, INPUT_PULLUP);
}

void loop()
{
  // Clear buffer if needed
  memset(buffer, 0, constantsCom::bufferSize);

  int i = 0;

  // Keep Alive start byte
  buffer[i++] = 1;

  // Steering
  int x = analogRead(joyX);
  // Serial.println(x);

  // Send data when the change is significant
  if (abs(x - lastX) > THRESHOLD)
  {
    // TODO: THIS VALUE NEEDS CALEBRATION WHEN THE PCB IS READY
    int maxReadSteer = 1023;
    uint8_t steer = map(x, 0, maxReadSteer,
                        constantsCom::steerMin,
                        constantsCom::steerMax);
    buffer[i++] = steer;
    lastX = x;
  }

  // Speed
  int speed = analogRead(speedPin);
  // Serial.println(speed);
  // Serial.println(lastSpeed);

  if (abs(speed - lastSpeed) > THRESHOLD)
  {
    lastSpeed = speed;
    // TODO: THIS VALUE NEEDS CALEBRATION WHEN THE PCB IS READY
    int maxRead = 674;
    speed = (int)map(speed, 0, maxRead, constantsCom::throttleMin, constantsCom::throttleMax);
    buffer[i++] = speed;
  }

  // Motor button
  bool button = !digitalRead(foilPin);
  // Serial.println(button);

  if (button != lastbutton && button)
  {
    buffer[i++] = constantsCom::foilToggle;
  }

  for (int j = 0; j < constantsCom::bufferSize; j++)
  {
    Serial.print(buffer[j]);
  }
  Serial.println("");

  // Send
  radio.write(buffer, constantsCom::bufferSize);

  delay(20);
}
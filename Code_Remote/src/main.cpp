/*
 * Arduino Wireless Communication Tutorial
 *     Example 1 - Transmitter Code
 *
 * by Dejan Nedelkovski, www.HowToMechatronics.com
 *
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */

#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

//  Data structure for transmission

struct Data {
  int x;
  bool motorOn;
};

Data data;

const int joyX = A0;
const int buttonPin = 2;



void setup()
{
  Serial.begin(9600);

  if (!radio.begin()){
    Serial.println("Radio error!");
    while(1);
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  pinMode(buttonPin, INPUT_PULLUP);
}

void loop()
{
  data.x = analogRead(joyX);                    // Read Joystick X-axis (0-1023)
  data.motorOn = digitalRead(buttonPin) == LOW; // Read button -> true / false

  radio.write(&data, sizeof(data));             // send data    
  delay(20);
}
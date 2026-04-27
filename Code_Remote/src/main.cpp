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
#include <constants.h>

#define THRESHOLD 10 //change if needed

// Radio module setup
RF24 radio(constantsPinsRemote::cePin, constantsPinsRemote::csnPin);

const byte address[6] = "00001";

// Buffer for sending data
uint8_t buffer[constantsCom::bufferSize]; 

// Pins
const int joyX = A0;
const int buttonPin = 2;

// safe data for sending
int lastX = 0;
bool lastbutton = false;

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
  // Clear buffer if needed
  memset(buffer, 0, constantsCom::bufferSize); 
  
  int i = 0;

  // Keep Alive start byte
  buffer[i++] = 1; 

  // Steering
  int x = analogRead(joyX);

  // Send data when the change is significant
  if (abs(x-lastX) > THRESHOLD){ 
    uint8_t steer = map(x, 0, 1023,
      constantsCom::steerMin, 
      constantsCom::steerMax);
      buffer[i++] = steer;
      lastX = x;
  }

  // Motor button
  bool button = !digitalRead(buttonPin);

  if (button != lastbutton){
    if (button){
      buffer[i++] = constantsCom::throttleOn;
    }
    else{
      buffer[i++] = constantsCom::throttleOff;
    }
    lastbutton = button;
  }

  // End of data
  buffer[i] = 0; 

  // Send
  radio.write(buffer, constantsCom::bufferSize); 

  delay(20);


  
}
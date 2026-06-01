/*
 * Arduino Wireless Communication Tutorial
 * Example 1 - Transmitter Code
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
#include <constants.h>

#define THRESHOLD 10 // change if needed

// Radio module setup
//RF24 radio(constantsPinsRemote::cePin, constantsPinsRemote::csnPin);
// Explicitly type your physical pins here (e.g., 7 and 8)
RF24 radio(7, 8);

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

// --- DEBUGGING VARIABLES ---
unsigned long lastLogTime = 0;
const unsigned long logInterval = 250; // Print debug info every 250ms

void setup()
{
  pinMode(foilPin, INPUT_PULLUP);
  // 1. IMMEDIATELY turn the LED on so we know the Nano is running
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  Serial.begin(9600);
  printf_begin();

  Serial.println("Booting up... testing radio next.");
  delay(500); // Give you a moment to see it solid ON

  // 2. Now try the radio. If it freezes here, the LED will stay SOLID ON.
  if (!radio.begin())
  {
    Serial.println("Hardware not responding! Check your wiring.");
    radio.printDetails();
    
    // If it fails but doesn't freeze, it will start blinking
    SPI.end();
    while (true)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  
  Serial.println("Remote Initialized successfully!");
  
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
  uint8_t steer = 0; // Temp variable to store mapped value for debugging

  // Send data when the change is significant
  if (abs(x - lastX) > THRESHOLD)
  {
    // TODO: THIS VALUE NEEDS CALEBRATION WHEN THE PCB IS READY
    int maxReadSteer = 1023;
    steer = map(x, 0, maxReadSteer,
                    constantsCom::steerMin,
                    constantsCom::steerMax);
    buffer[i++] = steer;
    lastX = x;
  }
  else 
  {
    // If not updated this frame, calculate what it currently sits at for debug visibility
    steer = map(x, 0, 1023, constantsCom::steerMin, constantsCom::steerMax);
  }

 // Speed / Throttle
  int speed = analogRead(speedPin);
  uint8_t mappedSpeed = 0; 

  // 1. Constrain raw readings to our defined physical limits (0 to 655).
  // This ensures any value >= 655 stays exactly 655, preventing out-of-bounds mapping.
  int maxRead = 655;
  int constrainedSpeed = constrain(speed, 0, maxRead);

  // 2. Check threshold against the constrained value
  if (abs(constrainedSpeed - lastSpeed) > THRESHOLD)
  {
    lastSpeed = constrainedSpeed;
    
    // 3. Reverse the mapping order:
    // maxRead (655) maps to throttleMin
    // 0 maps to throttleMax
    speed = (int)map(constrainedSpeed, maxRead, 0, constantsCom::throttleMin, constantsCom::throttleMax);
    buffer[i++] = speed;
    mappedSpeed = speed;
  }
  else
  {
    // If not updated this frame, calculate the inverted value for debug visibility
    mappedSpeed = (int)map(constrainedSpeed, maxRead, 0, constantsCom::throttleMin, constantsCom::throttleMax);
  }

 // Motor button
  // Standard INPUT_PULLUP logic: 
  // LOW means the button is physically making contact with Ground (PRESSED)
  // HIGH means the internal resistor is pulling it up (RELEASED)
  int pinReading = digitalRead(foilPin);
  bool button = (pinReading == LOW); 

  // Only trigger the event on the transition from RELEASED to PRESSED
  if (button && !lastbutton)
  {
    buffer[i++] = constantsCom::foilToggle;
    Serial.println("--- BUTTON TOGGLE EVENT TRIGGERED ---"); // Temporary action alert
  }
  
  // Update your tracking state so it doesn't continuously trigger
  lastbutton = button;

  // --- SERIAL DEBUGGING OUTPUT ---
  if (millis() - lastLogTime >= logInterval)
  {
    lastLogTime = millis();
    
    Serial.print("--- DEBUG DATA ---");
    Serial.print(" | Joystick X (Raw): ");  Serial.print(x);
    Serial.print(" -> Mapped: ");           Serial.print(steer);
    
    Serial.print(" | Throttle (Raw): ");    Serial.print(analogRead(speedPin)); // Fresh raw read
    Serial.print(" -> Mapped: ");           Serial.print(mappedSpeed);
    
    Serial.print(" | Button: ");           Serial.println(button ? "PRESSED" : "RELEASED");
  }

  // Send
  radio.write(buffer, constantsCom::bufferSize);

  delay(20);
}
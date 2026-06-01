#include <Arduino.h>
#include <Servo.h>

// --- Pin Definitions ---
const int ANALOG_PIN_1 = A2;  // Pin 21 (Resistor 1)
const int ANALOG_PIN_2 = A1;  // Pin 20 (Resistor 2)
const int SERVO_PIN_1  = 2;   // Pin 5  (D2)
const int SERVO_PIN_2  = 3;   // Pin 6  (D3)

// --- Hardware Constants ---
const float R_REF = 10000.0;  // Value of your reference resistors to GND (10k Ohms)

// --- PID Tuning Parameters ---
const float Kp = 0.1;  // Proportional gain
const float Ki = 0.05; // Integral gain
const float Kd = 0.01; // Derivative gain

// --- PID Target / Setpoint ---
const float SETPOINT = 5000.0; 

// --- Servo Objects ---
Servo servo1;
Servo servo2;

// --- PID State Variables ---
struct PIDState {
    float integral = 0.0;
    float lastError = 0.0;
};

PIDState pid1;
PIDState pid2;

unsigned long lastTime = 0;

// --- Function Declarations ---
float readResistance(int pin);
float updatePID(float currentInput, PIDState &state, float dt);

void setup() {
    Serial.begin(115200);
    
    servo1.attach(SERVO_PIN_1);
    servo2.attach(SERVO_PIN_2);
    
    // Initialize servos to center position (90 degrees)
    servo1.write(90);
    servo2.write(90);
    
    lastTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0; // Delta time in seconds
    
    // Fallback for timing anomalies or initial loops
    if (dt <= 0.0) dt = 0.01; 
    lastTime = currentTime;

    // 1. Measure actual resistances
    float r1 = readResistance(ANALOG_PIN_1);
    float r2 = readResistance(ANALOG_PIN_2);

    // 2. Compute PID outputs
    float output1 = updatePID(r1, pid1, dt);
    float output2 = updatePID(r2, pid2, dt);

    // 3. Map PID output to Servo degrees (0 to 180)
    // Assumes PID output ranges between -90 and 90 relative to the 90-degree midpoint
    int servoPos1 = constrain(90 + (int)output1, 0, 180);
    int servoPos2 = constrain(90 + (int)output2, 0, 180);

    // 4. Actuate Servos
    servo1.write(servoPos1);
    servo2.write(servoPos2);

    // Debugging output over Serial Monitor
    Serial.print("R1: "); Serial.print(r1);
    Serial.print(" -> S1: "); Serial.print(servoPos1);
    Serial.print(" | R2: "); Serial.print(r2);
    Serial.print(" -> S2: "); Serial.println(servoPos2);

    delay(20); // Small delay to decouple loop speed and prevent serial spam
}

/**
 * Calculates the resistance of the upper resistor in the voltage divider.
 */
float readResistance(int pin) {
    int adcVal = analogRead(pin);
    
    // Handle edge cases to avoid division by zero or negative calculation bounds
    if (adcVal >= 1023) return 0.0; 
    if (adcVal <= 0) return 999999.0; 

    // Voltage divider calculation
    float resistance = R_REF * ((1023.0 / (float)adcVal) - 1.0);
    return resistance;
}

/**
 * Standard positional PID algorithm loop.
 */
float updatePID(float currentInput, PIDState &state, float dt) {
    // Calculate Error
    float error = SETPOINT - currentInput;

    // Proportional term
    float Pout = Kp * error;

    // Integral term
    state.integral += error * dt;
    float Iout = Ki * state.integral;

    // Derivative term
    float derivative = (error - state.lastError) / dt;
    float Dout = Kd * derivative;

    // Save current error for the next iteration step
    state.lastError = error;

    // Combined output
    float output = Pout + Iout + Dout;

    // Optional: Constrain PID output to prevent integral windup 
    // limits the modification range to +/- 90 degrees
    if (output > 90.0) {
        output = 90.0;
        state.integral -= error * dt; // Anti-windup clamping
    } else if (output < -90.0) {
        output = -90.0;
        state.integral -= error * dt;
    }

    return output;
}
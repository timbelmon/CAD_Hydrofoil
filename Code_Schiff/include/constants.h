#pragma once
#include <stdint.h>

namespace constantsCom
{
    const uint8_t bufferSize = 32;
    const uint8_t steerMin = 64;
    const uint8_t steerMax = 127;
    const bool throttleToggle = true;
    const uint8_t throttleMin = 129;
    const uint8_t throttleMax = 255;
    const uint8_t throttleOn = 2;
    const uint8_t throttleOff = 3;
    const uint8_t throttleRev = 4;
    const uint8_t foilStabOn = 5;
    const uint8_t foilStabOff = 6;
}

namespace constantsPinsShip
{
    const uint8_t foilPin = 10;
    const uint8_t servoSteerPin = 2;
    const uint8_t csnPin = 8;
    const uint8_t cePin = 7;
    const uint8_t in1Pin = 3;
    const uint8_t in2Pin = 5;
    const uint8_t in3Pin = 6;
    const uint8_t in4Pin = 9;
}

namespace constantsShip
{
    const uint8_t steerAngleMax = 0;
    const uint8_t steerMidpoint = 90;
}
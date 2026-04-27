#pragma once
#include <stdint.h>

namespace constants
{
    const uint8_t bufferSize = 32;
    const uint8_t steerMin = 64;
    const uint8_t steerMax = 127;
    const bool throttleToggle = true;
    const uint8_t throttleMin = 128;
    const uint8_t throttleMax = 195;
    const uint8_t throttleOn = 2;
    const uint8_t throttleOff = 3;
    const uint8_t foilStabOn = 4;
    const uint8_t foilStabOff = 5;
}
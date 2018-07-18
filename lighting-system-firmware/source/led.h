#ifndef CONNECTED_LIGHTS_LED_H_
#define CONNECTED_LIGHTS_LED_H_

#define GROVE_CHAINABLE    1
#define TRICOLOR_ANODE     2
#define TRICOLOR_CATHODE   3

#if MBED_CONF_APP_LED_TYPE == GROVE_CHAINABLE
#include "ChainableLED.h"       // Library to talk to the Grove Chainable LED
static ChainableLED led(MBED_CONF_APP_GROVE_CLOCK_PIN, MBED_CONF_APP_GROVE_DATA_PIN, 1);
#else
static PwmOut redLed(MBED_CONF_APP_LED_PIN_RED);
static PwmOut greenLed(MBED_CONF_APP_LED_PIN_GREEN);
static PwmOut blueLed(MBED_CONF_APP_LED_PIN_BLUE);
#endif

void setRgbColor(float red, float green, float blue) {
#if MBED_CONF_APP_LED_TYPE == GROVE_CHAINABLE
    led.setColorRGB(0, static_cast<uint8_t>(red * 255.0f), static_cast<uint8_t>(green * 255.0f), static_cast<uint8_t>(blue * 255.0f));
#elif MBED_CONF_APP_LED_TYPE == TRICOLOR_ANODE
    redLed = 1.0f - red;
    greenLed = 1.0f - green;
    blueLed = 1.0f - blue;
#else
    redLed = red;
    greenLed = green;
    blueLed = blue;
#endif
}

#endif // CONNECTED_LIGHTS_LED_H_

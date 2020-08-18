#ifndef CONNECTED_LIGHTS_LED_H_
#define CONNECTED_LIGHTS_LED_H_

#define GROVE_CHAINABLE    1
#define GROVE_LED_BAR      2
#define TRICOLOR_ANODE     3
#define TRICOLOR_CATHODE   4

#if MBED_CONF_APP_LED_TYPE == GROVE_CHAINABLE
#include "ChainableLED.h"       // Library to talk to the Grove Chainable LED
static ChainableLED led(MBED_CONF_APP_GROVE_CLOCK_PIN, MBED_CONF_APP_GROVE_DATA_PIN, 1);
#elif MBED_CONF_APP_LED_TYPE == GROVE_LED_BAR
#include "LED_Bar.h"            // Library to talk to the Grove LED Bar
static LED_Bar led(MBED_CONF_APP_GROVE_DATA_PIN, MBED_CONF_APP_GROVE_CLOCK_PIN);
#else
static PwmOut redLed(MBED_CONF_APP_LED_PIN_RED);
static PwmOut greenLed(MBED_CONF_APP_LED_PIN_GREEN);
static PwmOut blueLed(MBED_CONF_APP_LED_PIN_BLUE);
#endif

void setRgbColor(float red, float green, float blue) {
#if MBED_CONF_APP_LED_TYPE == GROVE_CHAINABLE
    led.setColorRGB(0, static_cast<uint8_t>(red * 255.0f), static_cast<uint8_t>(green * 255.0f), static_cast<uint8_t>(blue * 255.0f));
#elif MBED_CONF_APP_LED_TYPE == GROVE_LED_BAR
    // Instead of setting the color, the LED Bar's level get changed
    // between 0-10 depending on the red percentage out of 255.0f
    led.setLevel((int)(red * 10.0f));
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

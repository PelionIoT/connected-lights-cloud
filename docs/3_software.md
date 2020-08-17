# Writing the software

Now you can write some software to verify that the circuit works. The software will enable the LED whenever it sees movement. You can either build locally by using Mbed CLI, or build in the cloud by using the Mbed Online Compiler. This document first shows you how to build the application using Mbed CLI.

## Setting up Mbed CLI

Part of the Arm Mbed IoT Platform is Mbed CLI, an offline compilation tool, which you will use to write and compile software. To get started:

1. [Install Mbed CLI](https://github.com/armmbed/mbed-cli#installation).
1. Open a terminal or command prompt.
1. Import the `connected-lights-cloud` project via:

    ```
    $ mbed import https://github.com/armmbed/connected-lights-cloud
    ```

This program contains a configuration file and all the source code, which you will use to configure the pins the software uses.

## Adding the code

Mbed OS comes with a powerful [configuration system](https://docs.mbed.com/docs/mbedmicro-api/en/latest/api/md_docs_config_system.html) that makes it easy to separate configuration and application code. In this application, you will separate the configuration of the LED (cathode, anode or a Grove LED), pins and connectivity method (next section).

In the 'connected-lights-cloud' folder, open `mbed_app.json`. Edit the file to reflect your LED choice and the pins you used to connect the LED and the PIR sensor:

1. If you have a common cathode LED, set the `value` of `led-type` to `TRICOLOR_CATHODE`.
1. If you have a common anode LED, set the `value` of `led-type` to `TRICOLOR_ANODE`.
1. If you have a Grove Chainable LED, set the `value` of `led-type` to `GROVE_CHAINABLE`.
1. If you have a Grove LED Bar, set the `value` of `led-type` to `GROVE_LED_BAR`.
1. If you are using a user button instead of a PIR sensor, set the `value` of `pir-or-btn` to `SW2` or your board's user button pin name.
1. Replace the pins D2, D5, D6 and D7 with the pins you used when building the circuit.

```js
/* mbed_app.json */

/* snip */

        "pir-or-btn": {
            "help": "Pin to which the PIR sensor or user button is connected",
            "macro_name": "PIR_OR_BTN",
            "value": "D2"
        },

        "led-type": {
            "help": "options are TRICOLOR_ANODE,TRICOLOR_CATHODE,GROVE_CHAINABLE,GROVE_LED_BAR",
            "value": "GROVE_CHAINABLE"
        },

        "led-pin-red": {
            "help": "Only used for TRICOLOR_* LED types",
            "value": "D5"
        },
        "led-pin-green": {
            "help": "Only used for TRICOLOR_* LED types",
            "value": "D6"
        },
        "led-pin-blue": {
            "help": "Only used for TRICOLOR_* LED types",
            "value": "D7"
        },

        "grove-clock-pin": {
            "help": "Only used for GROVE_CHAINABLE & GROVE_LED_BAR LED types",
            "value": "D5"
        },
        "grove-data-pin": {
            "help": "Only used for GROVE_CHAINABLE & GROVE_LED_BAR LED types",
            "value": "D6"
        },

/* snip */
```

Next, create a file called `main.cpp` in the `lighting-system-firmware/source/` directory:

```cpp
/* lighting-system-firmware/source/main.cpp */

#include "mbed.h"
#include "led.h"    // Abstracts away the differences between the LED types

// The PIR sensor/user button acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn sensor(PIR_OR_BTN);   // This pin value comes out mbed_app.json

// Whenever movement is not detected
void sensor_fall() {
    setRgbColor(0.0f, 0.0f, 0.0f);
}

// Whenever movement is detected
void sensor_rise() {
    setRgbColor(1.0f, 0.0f, 0.0f);
}

int main(int, char**) {
    // Blink the LED when the application starts
    setRgbColor(1.0f, 0.0f, 0.0f);
    wait(0.1);
    setRgbColor(0.0f, 0.0f, 0.0f);

    // The PIR sensor/user button uses interrupts, no need to poll
    // debounce to mbed_event_queue to avoid running this in an interrupt service routine
    sensor.fall(mbed_event_queue()->event(&sensor_fall));
    sensor.rise(mbed_event_queue()->event(&sensor_rise));
}
```

## Compiling and flashing

### Compiling

To compile the code, go back to your terminal, and run:

```
# go into the connected-lights-cloud folder
cd connected-lights-cloud

# detect which board you are using
$ mbed detect

# build the project, you'll need the GCC ARM cross-compilation toolchain installed
# optionally, you can also build with ARMCC or IAR
# the --flash option auto flashes the program onto the board
$ mbed compile -t GCC_ARM -m YOUR_BOARD_NAME --flash
```

When the compilation has completed a `connected-lights-cloud.bin` file appears in the `BUILD\YOUR_BOARD_NAME\GCC_ARM` folder and is auto flashed onto the board.

<span class="notes">**Note:** On some boards, you might need to press the *Reset* button to load the program.</span>

### Testing the application

After flashing the application, you can test it by waving your hand in front of the PIR sensor or pressing the user button; the red LED lights up.

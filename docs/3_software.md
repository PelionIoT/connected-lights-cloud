### Writing the software

Now you can write some software to verify that the circuit works. The software will enable the LED whenever it sees movement. You can either build locally by using Mbed CLI, or build in the cloud by using the Mbed Online Compiler. This document will first show you how to build the application using Mbed CLI.

#### Setting up Mbed CLI

Part of the ARM Mbed IoT Device Platform is Mbed CLI, an offline compilation tool, which you will use to write and compile software. To get started:

1. [Install Mbed CLI](https://github.com/armmbed/mbed-cli#installation).
1. Open a terminal or command prompt.
1. Import the `connected-lights-cloud` project via:

    ```
    $ mbed import git@github.com:armmbed/connected-lights-cloud.git connected-lights-cloud
    ```

This program contains a configuration file and all the source code, which you will use to configure the pins the software uses.

#### Adding the code

Mbed OS comes with a powerful [configuration system](https://docs.mbed.com/docs/mbedmicro-api/en/latest/api/md_docs_config_system.html) that makes it easy to separate configuration and application code. In this application, you will separate the configuration of the LED (cathode, anode or a Grove LED), pins and connectivity method (next section).

In the 'connected-lights-cloud' folder, open `mbed_app.json`. Edit the file to reflect your LED choice and the pins you used to connect the LED and the PIR sensor:

1. If you have a common cathode LED, set the `value` of `led-type` to `TRICOLOR_CATHODE`.
1. If you have a Grove Chainable LED, set the `value` of `led-type` to `TRICOLOR_ANODE`.
1. Replace the pins D2, D5, D6 and D7 with the pins you used when building the circuit.

```js
/* mbed_app.json */

/* snip */

        "pir-pin": {
            "help": "Pin to which the PIR sensor is connected",
            "macro_name": "PIR_PIN",
            "value": "D2"
        },

        "led-type": {
            "help": "options are TRICOLOR_ANODE,TRICOLOR_CATHODE,GROVE_CHAINABLE",
            "value": "TRICOLOR_ANODE"
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
            "help": "Only used for GROVE_CHAINABLE LED types",
            "value": "D5"
        },
        "grove-data-pin": {
            "help": "Only used for GROVE_CHAINABLE LED types",
            "value": "D6"
        },

/* snip */
```

Next, create a file called `main.cpp` in the `source` directory:

```cpp
/* lighting-system-firmware/source/main.cpp */

#include "mbed.h"
#include "led.h"    // Abstracts away the differens between the LED types

// PIR sensor acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn pir(PIR_PIN);   // This pin value comes out mbed_app.json

// Whenever movement is not detected
void pir_fall() {
    setRgbColor(0.0f, 0.0f, 0.0f);
}

// Whenever movement is detected
void pir_rise() {
    // set the color to red
    setRgbColor(1.0f, 0.0f, 0.0f);
}

int main(int, char**) {
    // Blink the LED when the application starts
    setRgbColor(0.0f, 1.0f, 0.0f);
    Thread::wait(500);
    setRgbColor(0.0f, 0.0f, 0.0f);

    // The PIR sensor uses interrupts, no need to poll
    pir.fall(&pir_fall);
    pir.rise(&pir_rise);
}
```

#### Compiling and flashing

##### Compiling

To compile the code, go back to your terminal, and run:

```
# go into the connected-lights-cloud folder
cd connected-lights-cloud

# detect which board you are using
$ mbed detect

# build the project, you'll need the GCC ARM cross-compilation toolchain installed
# optionally, you can also build with ARMCC or IAR
$ mbed compile -t GCC_ARM -m YOUR_BOARD_NAME
```

After compilation a `connected-lights-cloud.bin` file was generated in the `BUILD\YOUR_BOARD_NAME\GCC_ARM` folder.

##### Flashing

When you connect your board to your computer, it mounts as a USB mass storage device, like a USB drive. To flash the new application onto the board, drag and drop the firmware file onto the mass storage device:

<span class="images">![Flashing the application on Windows](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights8.png)<span>Drag the firmware file onto the mass storage device to flash the application.</span></span>

<span class="notes">**Note:** On some boards, you might need to press the *Reset* button to load the program.</span>

##### Testing the application

After flashing the application, you can test it by waving your hand in front of the PIR sensor; the red LED lights up.

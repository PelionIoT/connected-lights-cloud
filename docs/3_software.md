### Writing the software

Now we can write some software to verify that the circuit works. We will enable the LED whenever we see movement. We can either build locally by using mbed CLI, or in the cloud by using the mbed Online Compiler. We will first show how to use the online compiler, and at the end of this section we will show the corresponding commands in mbed CLI.

#### Setting up the online compiler

Part of the ARM mbed IoT Device Platform is an online compiler, which we will use to write and compile software in the browser. To get started:

1. Find your development board's [platform page](https://developer.mbed.org/platforms/).
1. In the right hand panel, choose **Add to your mbed Compiler**.
1. When prompted sign up (or sign in). You are redirected to the platform page.
1. Choose **Open mbed Compiler**. The online compiler opens.
1. Click the **Import** button.

    <span class="images">![Import button](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights17.png)</span>

1. Click the "Click here to import from URL" link:

    <span class="images">![Import from URL](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights18.png)</span>

1. Under **Source URL** enter `https://github.com/armmbed/connected-lights-cloud`.

    Do not check the **Update all libraries to the latest revision** box.

    <span class="images">![Creating a program in the online compiler](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights6.png)</span>

This program contains a configuration file, which we will use to configure the pins our software uses. 

#### Adding the code

mbed OS comes with a powerful [configuration system](https://docs.mbed.com/docs/mbedmicro-api/en/latest/api/md_docs_config_system.html) that makes it easy to separate configuration and application code. In this application we will separate the configuration of the LED (cathode, anode or a Grove LED), pins, and connectivity method (next section).

From the mbed Online Compiler's tree, open ``mbed_app.json``. Edit the file to reflect your LED choice, and the pins you used to connect the LED and the PIR sensor:

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

Next, create a file called ``main.cpp`` in the `source` directory:

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

Compile the code by clicking the *Compile* button at the top of the screen:

<span class="images">![The compile button](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights7.png)</span>

A successful compilation downloads a `.bin` file to your computer. This is the compiled firmware for your development board.

##### Flashing

When you connect your board to your computer, it mounts as a USB mass storage device, like a USB drive. To flash the new application onto the board, drag and drop the firmware file onto the mass storage device:

<span class="images">![Flashing the application on Windows](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights8.png)<span>Drag the firmware file onto the mass storage device to flash the application.</span></span>

<span class="notes">**Note:** On some boards you might need to press the *Reset* button to load the program.</span>

##### Testing the application

After flashing the application, you can test it by waving your hand in front of the PIR sensor; the red LED should light up.

#### Developing using mbed CLI

You can also develop locally using [mbed CLI](http://github.com/armmbed/mbed-cli), a command line tool for mbed OS. First follow [the installation steps](https://github.com/ARMmbed/mbed-cli#installing-mbed-cli), then use the following commands to recreate the flow above:

```bash
# import the connected-lights-cloud project
$ mbed import connected-lights-cloud

# go into the folder
cd connected-lights-cloud/

# now edit the mbed_app.json file, and create the main.cpp file

# detect which board you are using
$ mbed detect

# build the project, you'll need the GCC ARM cross-compilation toolchain installed
# optionally, you can also build with ARMCC or IAR
$ mbed compile -t GCC_ARM -m YOUR_BOARD_NAME

# … building …
# ends with something like:
# Image: ./BUILD/K64F/GCC_ARM/connected-lights-cloud.bin
```

Copy the binary file that was generated to your board using drag-and-drop programming (as shown under [Compiling and flashing](#compiling-and-flashing)).


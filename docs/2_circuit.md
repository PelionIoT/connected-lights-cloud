# Building the circuit

First, build a basic circuit by connecting the RGB LED and the PIR sensor to the development board.

## SD card

<span class="notes">**Note:** Before you start, put the SD card in the SD card reader on your development board.</span>

Pelion Device Management requires an external storage to store the identity of the device, and to store firmware updates. This external storage does not necessarily have to be an SD card (which is too expensive to put in a production device), you can also use internal flash or external SPI flash. To modify the storage layer, you'll need to modify the bootloader (see 'Section 8 - Firmware updates') and the application (see `mbed_app.json` in the `connected-lights-cloud` application later in this tutorial); which is not covered in this tutorial. More information is in the [Device Management documentation](https://cloud.mbed.com/docs/).

## Finding suitable pins

For the circuit, you need four digital pins. Three of these need to support pulse width modulation (PWM). Through PWM, you can control the amount of current flowing through a circuit, and you can use this to dim the colors of the LED on one of the three RGB channels.

To find pins that you can use, look at the [platform page](https://developer.mbed.org/platforms/) for your board, and find the pinout. The pinout defines the characteristics of the pins. For example, here is the pinout for the FRDM-K64F. In this example, you can use D5, D6 and D7 as our PWM pins:

<span class="images">![FRDM-K64F pinout showing PWM pins](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights3.png)<span>The pinout for the NXP FRDM-K64F, showing that you can use pins D5, D6 and D7 for PWM.</span></span>

You also need a pin for the PIR sensor. This can be any digital pin, as long as it's not marked as UART (D0 and D1 on the pinout above). In this example, you can use pin D2.

<span class="notes">**Note:** In general, it's a good idea not to use any of the I2C and SPI pins for LEDs and basic sensors because connectivity shields (such as Wi-Fi) might need them.</span>

## Connecting the peripherals on a breadboard

Here is a diagram of hooking up the PIR sensor and the RGB LED to your board. Replace the pins D2, D5, D6 and D7 with the pins you found for your board. If you have a four-pin RGB LED, you must position the LED so that the longest pin is the second from the left. (Hold it [like this](http://howtomechatronics.com/wp-content/uploads/2015/09/RGB-LED.png?28ea0f).)

<span class="images">![PIR sensor and RGB LED Fritzing diagram](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights4.png)</span>

<span class="notes">**Note - common anode or common cathode LED:** Four pin RGB LEDs come in two different types: *common anode* and *common cathode*. If you have a common cathode LED, connect the second pin to `GND` instead of `3.3V`. If you are unsure, try both circuits and see which one works.</span>

<span class="notes">**Note:** If you're unsure of the pins on the PIR sensor and you have a sensor with a 'dome' on it, remove the dome. The PCB describes the pins.</span>

When you have connected everything, the circuit looks something like this:

<span class="images">![PIR sensor and RGB LED connected to FRDM-K64F](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights5.png)</span>

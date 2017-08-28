## Tutorial: Building an internet connected lighting system

In this tutorial, you'll use the ARM mbed IoT Device Platform to build a complete connected lighting system that you can deploy in your house or office. The system consists of one (or more) extra-bright RGB LEDs hooked up to an mbed OS development board. A motion sensor can turn the lights on and off. The lights are also connected to the internet, so you can change their color. The system uses mbed Cloud to connect to the internet. It is secured end-to-end, and you can update it through over-the-air firmware updates.

You'll learn all the steps required to build the hardware, the cloud integration and the application. At the end of the tutorial, you'll have a firm understanding of building complete connected IoT solutions on the mbed IoT Device Platform. You'll also have a cool light.

### Requirements

You need the following hardware:

* A [development board](https://developer.mbed.org/platforms/?software=16) capable of connecting to mbed Cloud. Either:
    * [NXP FRDM-K64F](https://developer.mbed.org/platforms/FRDM-K64F/) using Ethernet.
    * [u-blox EVK-ODIN-W2](https://developer.mbed.org/platforms/ublox-EVK-ODIN-W2/) with Ethernet and Wi-Fi.
* A breadboard to hook up the components.
* A PIR sensor to detect motion.
* An RGB LED - preferably an extra-bright one.
    * For a better effect, you can also use a [Grove Chainable LED](http://wiki.seeed.cc/Grove-Chainable_RGB_LED/).
    * After building the original application, you can exchange the LED for something [fancier](https://www.adafruit.com/product/1138).
* Jumper wires, both male-male and male-female.
* Resistors: 1x 100 Ohm, 2x 220 Ohm.

<span class="images">![Components needed](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights2.png)<span>Components required to build our lighting system. Top row: RGB LED, PIR sensor, Grove Chainable LED. Bottom row: breadboard, NXP FRDM-K64F, jumper wires.</span></span>

You also need:

* An account on [developer.mbed.org](https://developer.mbed.org/) to access the Online Compiler.
* [An account](https://portal.us-east-1.mbedcloud.com) to access to the mbed Cloud Portal.

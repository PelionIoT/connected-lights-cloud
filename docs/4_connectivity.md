### Adding connectivity

Now that you've built the basic circuit and written the code to control that circuit, you can add connectivity to the project. Part of the ARM mbed IoT Device Platform is mbed Cloud, a unified solution to connect devices to the internet and communicate with them, regardless of *how* these devices connect to the internet. Libraries are available for a variety of connectivity methods, including Ethernet, Wi-Fi and cellular. You also can add new connectivity methods with the [unified networking APIs](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/network_sockets/) in mbed OS 5.

#### Obtaining a device certificate

[mbed TLS](https://tls.mbed.org) encrypts all data that goes from the device to mbed Cloud (and from mbed Cloud to the device. You need a security certificate to set up secure communication, which you can get from the mbed Cloud Portal:

1. Go to the [mbed Cloud Portal](https://portal.mbedcloud.com/login), and sign in.
1. If prompted for your login credentials, use your mbed Cloud credentials. These are different from your credentials for the mbed Online Compiler.
1. Go to *Developer Tools* > *Certificate*.
1. Click *Get new device security credentials*.
1. Copy the contents of the white box by clicking *Copy to clipboard*. This is your certificate.

    <span class="images">![The certificate is located in the white box](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights16.png)</span>

1. Go back to the mbed Online Compiler.
1. Create a new file `identity_dev_security.c` in your application's `source` directory.
1. Paste the certificate into this file.

<span class="notes">**Note:** You can only download the certificate once. It's not stored in the mbed Cloud Portal.</span>

#### Adding connectivity to the board

This example assumes that the network has DHCP enabled and the firewall does not block connections to *https://mbedcloud.com*.

If you have a development board that connects over Ethernet, just plug in an Ethernet cable. If you have a board that connects over cellular or Wi-Fi, no actions are required.

#### Adding libraries with the mbed Online Compiler

For the device and mbed Cloud to talk, you need the [mbed Cloud Client library](https://cloud.mbed.com/docs/latest/mbed-cloud-client/index.html). This is a cross-platform library that runs on mbed OS and Linux, and you can port it to other RTOS. In this example, you will use an additional library built on top of mbed Cloud Client: SimpleCloudClient. This library is designed specifically to be used with mbed OS 5 and makes it easy to expose variables and resources to the cloud.

You will also use [EasyConnect](https://github.com/ARMmbed/easy-connect) to handle connectivity.

To add these libraries to your project:

1. Go back to the mbed Online Compiler.
1. Right click on your program in the tree, and select *Import Library* > *From URL*.
1. Under *Source URL*, enter: `https://github.com/armmbed/easy-connect`.
1. Do **not** tick 'Update all sub-libraries to the latest version'.
1. Click *Import*.
1. Again, right click on your program, and select *Import Library* > *From URL*.
1. Under *Source URL* enter: `https://github.com/armmbed/simple-cloud-client/`.
1. Click *Import*.

#### Adding libraries with mbed CLI

If you are using mbed CLI, run the following commands to add the libraries:

```bash
$ mbed add easy-connect
$ mbed add simple-cloud-client
```

#### Updating configuration

You need to tell EasyConnect which connectivity method to use. Open `mbed_app.json`, and locate the `network-interface` field. Change the `value` to the connectivity method used:

```json
/* mbed_app.json */

/* snip */

        "network-interface":{
            "help": "options are ETHERNET,WIFI_ESP8266,MESH_LOWPAN_ND,MESH_THREAD",
            "value": "ETHERNET"
        },
        "esp8266-tx": {
            "help": "Pin used as TX (connects to ESP8266 RX)",
            "value": "D1"
        },
        "esp8266-rx": {
            "help": "Pin used as RX (connects to ESP8266 TX)",
            "value": "D0"
        },
        "esp8266-ssid": {
            "value": "\"SSID\""
        },
        "esp8266-password": {
            "value": "\"Password\""
        },
        "esp8266-debug": {
            "value": true
        }
    }

/* snip */
```

If you are using Wi-Fi, you also need to set your Wi-Fi SSID and your password.

#### Writing code

##### Setting up a connection

You need to add some code to the application, so it connects to the internet and sets up a connection to mbed Cloud.

Replace `main.cpp` with:

```cpp
#include "mbed.h"
#include "led.h"        // Abstracts away the differens between the LED types
#include "easy-connect.h"
#include "simple-cloud-client.h"

EventQueue eventQueue;  // An event queue
Thread eventThread;     // An RTOS thread to process events in

SimpleMbedClient client;

// PIR sensor acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn pir(PIR_PIN);   // This pin value comes out mbed_app.json


// YOUR CODE HERE
void pir_rise() {}
// END OF YOUR CODE HERE


// Use the built-in LED as a status LED
DigitalOut statusLed(LED1);
int        statusLedBlinkId;    // Callback ID
void blink_builtin_led() {
    statusLed = !statusLed;
}

void registered() {
    // When we registered with mbed Cloud, blink faster
    eventQueue.cancel(statusLedBlinkId);

    statusLedBlinkId = eventQueue.call_every(300, &blink_builtin_led);

    printf("Registered\n");
}

int main(int, char**) {
    // Using an event queue is a very useful abstraction around many microcontroller 'problems', like dealing with ISRs
    // see https://developer.mbed.org/blog/entry/Simplify-your-code-with-mbed-events/
    eventThread.start(callback(&eventQueue, &EventQueue::dispatch_forever));

    // Blink the built-in LED every 1000ms. After registering we'll blink every 300ms.
    statusLedBlinkId = eventQueue.call_every(1000, &blink_builtin_led);

    // Disable the LED
    setRgbColor(0.0f, 0.0f, 0.0f);

    // The PIR sensor uses interrupts, no need to poll
    pir.rise(eventQueue.event(&pir_rise));

    NetworkInterface* network = easy_connect(true);
    if (!network) {
        printf("Connect to internet failed... See serial output.\r\n");
        return 1;
    }

    if (!client.setup(network)) {
      printf("Setting up mbed_client failed...\r\n");
      return 1;
    }

    client.on_registered(&registered);

    wait(osWaitForever);
}
```

##### Program logic

The code sample above sets up the connection. You can now define some logic for this program:

1. The color of the LED should be configurable.
1. The period between the moment of motion detection to the moment lights go out should be configurable.
1. There should be a permanent-on mode for the lights.
1. You should notify mbed Cloud whenever you detect movement.

You can implement these actions by defining *resources*: pieces of information the device makes available. You can read or write to them from the cloud, and the device can use a resource's value to determine the correct action to perform. You can reach a resource with a URI and access modifier (for example, only write allowed), and you can also subscribe to them, so you receive a notification when a resource changes.

Define a resource for each action:

* `led/0/color` - the color of the LED.
* `led/0/timeout` - the timeout (in seconds) after detection; lights are disabled when this period ends.
* `led/0/permanent_status` - whether you should have the lights permanently on (or off).
* `pir/0/count` - the number of times the PIR sensor was triggered. Read only, and should allow notifications.

You can use SimpleCloudClient to define these resources and attach actions to each resource.

Replace the following section in `main.cpp`:

```cpp
// YOUR CODE HERE
void pir_rise() { }
// END OF YOUR CODE HERE
```

with (comments inline):

```cpp
// Fwd declaration
void putLightsOn();
void colorChanged(int newColor);

// Variable that holds whether the light is on because the PIR sensor triggered (and timeout didn't happen yet)
bool ledOnBecauseOfPir = false;

// Timeout based on led/0/timeout, disables the light after a set interval
Timeout pirTimeout;

// Permanent statuses (set by led/0/permanent_status)
enum PermanentStatus {
    STATUS_NONE = 0,
    STATUS_ON   = 1,
    STATUS_OFF  = 2
};

// clear the lights
void putLightsOff() {
    setRgbColor(0.0f, 0.0f, 0.0f);
}

// Status changes
void statusChanged(int newStatus) {
    switch (newStatus) {
        case STATUS_ON: // Permanently on? OK.
            putLightsOn();
            break;
        case STATUS_NONE: // Otherwise listen to PIR sensor
        case STATUS_OFF:  // Or be off forever
            putLightsOff();
            break;
    }
}

// Here are our resources:
// We encode color in 3 bytes [R, G, B] and put it in an integer by providing the color as an hex value (default color: green)
SimpleResourceInt ledColor = client.define_resource("led/0/color", 0x00ff00, &colorChanged);
SimpleResourceInt ledTimeout = client.define_resource("led/0/timeout", 5);
SimpleResourceInt ledStatus = client.define_resource("led/0/permanent_status", STATUS_NONE, &statusChanged);
SimpleResourceInt pirCount = client.define_resource("pir/0/count", 0, M2MBase::GET_ALLOWED);

// As said above, color is encoded in three bytes
void putLightsOn() {
    // parse the individual channels
    int redCh   = ledColor >> 16 & 0xff;
    int greenCh = ledColor >> 8 & 0xff;
    int blueCh  = ledColor & 0xff;

    // our color is 0..255, but we need a float between 0..1, cast it.
    float red = static_cast<float>(redCh) / 255.0f;
    float green = static_cast<float>(greenCh) / 255.0f;
    float blue = static_cast<float>(blueCh) / 255.0f;
    setRgbColor(red, green, blue);
}

// Color updated from the cloud,
// if the LED is on because of the PIR, or if the LED is on permanently -> Set the color.
void colorChanged(int newColor) {
    if (ledOnBecauseOfPir || ledStatus == STATUS_ON) {
        putLightsOn();
    }
}

// Timeout (from led/0/timeout) happened after PIR sensor was triggered...
void onPirTimeout() {
  // if we're not permanent on
    if (ledStatus != STATUS_ON) {
        // clear the lights
        putLightsOff();

        ledOnBecauseOfPir = false;
    }
}

// When the PIR sensor fires...
void pir_rise() {
    // Update the resource
    pirCount = pirCount + 1;

    // Permanent off? Don't put the lights on...
    if (ledStatus == STATUS_OFF) return;

    // Otherwise do it!
    ledOnBecauseOfPir = true;
    putLightsOn();

    // And attach the timeout
    pirTimeout.attach(eventQueue.event(&onPirTimeout), static_cast<float>(ledTimeout));
}
```

When you compile and flash this program, you'll see that when you wave your hand in front of the PIR sensor, the color of the LED changes to green, and the LED always turns off after 5 seconds.

When the connection to mbed Cloud is created, the onboard LED blinks faster. You can now control this device from the cloud.

<span class="notes">**Note:** No connection? [Inspect the logs on the device](https://docs.mbed.com/docs/mbed-os-handbook/en/latest/debugging/printf/).</span>


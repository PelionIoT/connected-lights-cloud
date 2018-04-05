### Adding connectivity

Now that you've built the basic circuit and written the code to control that circuit, you can add connectivity to the project. Part of the ARM Mbed IoT Device Platform is Mbed Cloud, a unified solution to connect devices to the internet and communicate with them, regardless of *how* these devices connect to the internet. Libraries are available for a variety of connectivity methods, including Ethernet, Wi-Fi and cellular. You also can add new connectivity methods with the [unified networking APIs](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/network_sockets/) in Mbed OS 5.

#### Obtaining a device certificate

[Mbed TLS](https://tls.mbed.org) encrypts all data that goes from the device to Mbed Cloud (and from Mbed Cloud to the device). You need a security certificate to set up secure communication, which you can get from the Mbed Cloud Portal:

1. Go to the [Mbed Cloud Portal](https://portal.us-east-1.mbedcloud.com), and sign in.
1. If prompted for your login credentials, use your Mbed Cloud credentials. These are different from your credentials for the Mbed Online Compiler.
1. Select **Device Identity** > **Certificates**.
1. Click **Actions** > **Create a developer certificate**.
1. Enter a name for the certificate, and click **Create certificate**.
1. Click **Download Developer C file**. Your certificate file downloads.

    <span class="images">![The certificate is located in the white box](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights16.png)</span>

1. Go to the 'connected-lights-cloud' folder on your computer.
1. Create a new file `identity_dev_security.c` in your application's `source` directory.
1. Paste the certificate into this file.

#### Adding connectivity to the board

**Built-in Ethernet, Wi-Fi or Cellular**

This example assumes that the network has DHCP enabled and the firewall does not block connections to *https://mbedcloud.com*.

If you have a development board that connects over Ethernet, just plug in an Ethernet cable. If you have a board that connects over cellular or Wi-Fi, no actions are required.

**ESP8266 Wi-Fi module**

To wire the ESP8266 module to your development board, look at the [ESP8266 Cookbook page](https://os.mbed.com/users/4180_1/notebook/using-the-esp8266-with-the-mbed-lpc1768/). This means hooking up the ESP8266's TX pin to `D0` and RX pin to `D1`.

<span class="notes">**Note about ESP8266 on NUCLEO boards:** The NUCLEO boards reserve pins D0 and D1 for serial communication with the computer. Use pins `D8` (to ESP8266 TX) and `D2` (to ESP8266 RX) instead.</span>

#### Adding libraries with Mbed CLI

For the device and Mbed Cloud to talk, you need the [Mbed Cloud Client library](https://cloud.mbed.com/docs/latest/mbed-cloud-client/index.html). This is a cross-platform library that runs on Mbed OS and Linux and that you can port to other RTOSes. This example uses an additional library built on top of Mbed Cloud Client: SimpleM2MClient. We created this library specifically to use Mbed OS 5, so you can expose variables and resources to the cloud.

You will also use [EasyConnect](https://github.com/ARMmbed/easy-connect) to handle connectivity.

These libraries are already in the project (see the `.lib` files in the project directory).

#### Updating configuration

You need to tell **EasyConnect** which connectivity method to use. Open `mbed_app.json`, and locate the `network-interface` field. Change the `value` to the connectivity method used:

```json
/* mbed_app.json */

/* snip */

        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_ODIN, MESH_LOWPAN_ND, MESH_THREAD",
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
        "wifi-ssid": {
            "value": "\"SSID\""
        },
        "wifi-password": {
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

You need to add some code to the application, so it connects to the internet and sets up a connection to Mbed Cloud.

Replace `connected-lights-cloud/source/main.cpp` with:

```cpp
#include "mbed.h"
#include "easy-connect.h"
#include "led.h"        // Abstracts away the differens between the LED types
#include "simplem2mclient.h"
#include "storage-selector.h"

EventQueue eventQueue;                  // An event queue
Thread eventThread;                     // An RTOS thread to process events in
FileSystem* fs = filesystem_selector(); // Mbed Cloud requires a filesystem, mount it (based on parameters in mbed_app.json)

SimpleM2MClient *client;
M2MObjectList obj_list;

// PIR sensor acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn pir(SW2);   // This pin value comes out mbed_app.json


// YOUR CODE HERE
void pir_rise() { }
// END OF YOUR CODE HERE


// Use the built-in LED as a status LED
DigitalOut statusLed(LED1);
int        statusLedBlinkId;    // Callback ID
void blink_builtin_led() {
    statusLed = !statusLed;
}

void registered(const ConnectorClientEndpointInfo *endpoint) {
    // When we registered with Mbed Cloud, blink faster
    eventQueue.cancel(statusLedBlinkId);

    statusLedBlinkId = eventQueue.call_every(300, &blink_builtin_led);

    printf("Connected to Mbed Cloud. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
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
        printf("Connect to internet failed... See serial output.\n");
        return 1;
    }

    client = new SimpleM2MClient(network);
    client->on_registered(&registered);

    int init_rt = client->init();
    if (init_rt != 0) {
        printf("Failed to initialize Mbed Cloud Client (%d)", init_rt);
        return 1;
    }

    printf("Connecting to Mbed Cloud...\n");

    // Add resources, they will register after call_register() is called.
    client->add_objects(obj_list);

    // Start registering to the cloud.
    client->call_register();

    wait(osWaitForever);
}
```

##### Program logic

The code sample above only sets up the connection. You can now define some logic for this program:

- The color of the LED should be configurable.
- The period between the moment of motion detection to the moment lights go out should be configurable.
- There should be a permanent-on mode for the lights.
- You should notify Mbed Cloud whenever you detect movement.

To implement these actions, you need to define *resources*: pieces of information the device makes available. You can read or write to them from the cloud, and the device can use a resource's value to determine the correct action to perform. You can reach a resource with a URI and access modifier (for example, only write allowed), and you can also subscribe to them, so you receive a notification when a resource changes.

Define a resource for each action:

* `3311/0/5706` - the color of the LED.
* `3311/0/5853` - the timeout (in seconds) after detection; lights are disabled when this period ends.
* `3311/0/5850` - whether you should have the lights permanently on (or off).
* `3201/0/5700` - the number of times the PIR sensor was triggered. Read only, and should allow notifications.

You can use the `add_resource` function to define these resources and attach actions to each resource.

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
void colorChanged(const char*);
void statusChanged(const char*);

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

// Here are our resources:
// We encode color in 3 bytes [R, G, B] and put it in an integer by providing the color as an hex value (default color: green)
M2MResource *ledColor = add_resource(&obj_list, 3311, 0, 5706, "LED_Color", M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED, 0x00ff00, false, &colorChanged);
M2MResource *ledTimeout = add_resource(&obj_list, 3311, 0, 5853, "LED_Timeout", M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED, 5, false);
M2MResource *ledStatus = add_resource(&obj_list, 3311, 0, 5850, "LED_Permanent_status", M2MResourceInstance::INTEGER, M2MBase::GET_PUT_ALLOWED, STATUS_NONE, false, &statusChanged);
M2MResource *pirCount = add_resource(&obj_list, 3201, 0, 5700, "PIR_Count", M2MResourceInstance::INTEGER, M2MBase::GET_ALLOWED, 0, true);

// As said above, color is encoded in three bytes
void putLightsOn() {
    int color = ledColor->get_value_int();

    // parse the individual channels
    int redCh   = color >> 16 & 0xff;
    int greenCh = color >> 8 & 0xff;
    int blueCh  = color & 0xff;

    // our color is 0..255, but we need a float between 0..1, cast it.
    float red = static_cast<float>(redCh) / 255.0f;
    float green = static_cast<float>(greenCh) / 255.0f;
    float blue = static_cast<float>(blueCh) / 255.0f;
    setRgbColor(red, green, blue);
}

// Color updated from the cloud,
// if the LED is on because of the PIR, or if the LED is on permanently -> Set the color.
void colorChanged(const char*) {
    int status = ledStatus->get_value_int();

    if (ledOnBecauseOfPir || status == STATUS_ON) {
        putLightsOn();
    }
}

// Status changes from the cloud
void statusChanged(const char*) {
    int newStatus = ledStatus->get_value_int();

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

// Timeout (from led/0/timeout) happened after PIR sensor was triggered...
void onPirTimeout() {
  // if we're not permanent on
    if (ledStatus->get_value_int() != STATUS_ON) {
        // clear the lights
        putLightsOff();

        ledOnBecauseOfPir = false;
    }
}

// When the PIR sensor fires...
void pir_rise() {
    // Update the resource
    pirCount->set_value(pirCount->get_value_int() + 1);

    // Permanent off? Don't put the lights on...
    if (ledStatus->get_value_int() == STATUS_OFF) return;

    // Otherwise do it!
    ledOnBecauseOfPir = true;
    putLightsOn();

    // And attach the timeout
    pirTimeout.attach(eventQueue.event(&onPirTimeout), static_cast<float>(ledTimeout->get_value_int()));
}
```

When you compile and flash this program, you'll see that when you wave your hand in front of the PIR sensor, the color of the LED changes to green, and the LED always turns off after 5 seconds.

When the connection to Mbed Cloud is created, the onboard LED blinks faster. You can now control this device from the cloud.

<span class="notes">**Note:** No connection? [Inspect the logs on the device](https://docs.mbed.com/docs/mbed-os-handbook/en/latest/debugging/printf/). Use baud rate 115,200 to communicate with your device.</span>

<span class="notes">**Note:** If you receive an `fcc_init` error, re-format the SD card (FAT). If your computer does not have an SD-card slot, see the [format-sd-card](https://os.mbed.com/users/janjongboom/code/format-sd-card/) Mbed program.</span>


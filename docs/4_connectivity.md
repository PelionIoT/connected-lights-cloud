## Adding connectivity

Now that you've built the basic circuit and written the code to control that circuit, you can add connectivity to the project. Part of Arm's IoT Platform is Pelion Device Management, a unified solution to connect devices to the internet and communicate with them, regardless of *how* these devices connect to the internet. Libraries are available for a variety of connectivity methods, including Ethernet, Wi-Fi and cellular. You also can add new connectivity methods with the [unified networking APIs](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/network_sockets/) in Mbed OS 5.

### Obtaining a device certificate

[Mbed TLS](https://tls.mbed.org) encrypts all data that goes from the device to Device Management (and from Device Management to the device). You need a security certificate to set up secure communication, which you can get from the Device Management Portal:

1. Go to the [Device Management Portal](https://portal.us-east-1.mbedcloud.com), and sign in.
1. If prompted for your login credentials, use your Device Management credentials. These are different from your credentials for the Mbed Online Compiler.
1. Select **Device identity** > **Certificates**.
1. Click **New certificate** > **Create a developer certificate**.
1. Enter a name for the certificate, and click **Create certificate**.
1. Go to **Device identity** > **Certificates** again.
1. Click on your new certificate.
1. Click **Download developer C file**. Your certificate file downloads.

    <span class="images">![The certificate is located in the white box](https://s3-us-west-2.amazonaws.com/cloud-docs-images/lights16.png)</span>

1. Copy the file (named `mbed_cloud_dev_credentials.c`) into your application's `source` directory.

### Adding connectivity to the board

**Built-in Ethernet, Wi-Fi or Cellular**

This example assumes that the network has DHCP enabled and the firewall does not block connections to *https://mbedcloud.com*.

If you have a development board that connects over Ethernet, just plug in an Ethernet cable. If you have a board that connects over cellular or Wi-Fi, no actions are required.

**ESP8266 Wi-Fi module**

To wire the ESP8266 module to your development board, look at the [ESP8266 Cookbook page](https://os.mbed.com/users/4180_1/notebook/using-the-esp8266-with-the-mbed-lpc1768/). This means hooking up the ESP8266's TX pin to `D0` and RX pin to `D1`.

<span class="notes">**Note about ESP8266 on NUCLEO boards:** The NUCLEO boards reserve pins D0 and D1 for serial communication with the computer. Use pins `D8` (to ESP8266 TX) and `D2` (to ESP8266 RX) instead.</span>

### Adding libraries with Mbed CLI

For the device and Device Management to talk, you need the [Device Management Client library](https://cloud.mbed.com/docs/latest/mbed-cloud-client/index.html). This is a cross-platform library that runs on Mbed OS and Linux and that you can port to other RTOSes. This example uses an additional library built on top of Device Management Client: SimpleM2MClient. We created this library specifically to use Mbed OS 5, so you can expose variables and resources to the cloud.

You will also use [EasyConnect](https://github.com/ARMmbed/easy-connect) to handle connectivity.

These libraries are already in the project (see the `.lib` files in the project directory).

### Updating configuration

You need to tell **EasyConnect** which connectivity method to use. Open `mbed_app.json`, and locate the `network-interface` field. Change the `value` to the connectivity method used:

```json
/* mbed_app.json */

/* snip */

        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW0XX1, WIFI_ODIN, WIFI_RTW, WIFI_WIZFI310, WIFI_ISM43362, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
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

### Writing code

#### Setting up a connection

You need to add some code to the application, so it connects to the internet and sets up a connection to Device Management.

Replace `connected-lights-cloud/source/main.cpp` with:

```cpp
#include "mbed.h"
#include "easy-connect.h"
#include "led.h"                                      // Abstracts away the differences between the LED types
#include "simple-mbed-cloud-client.h"
#include "storage-selector.h"

EventQueue eventQueue;                                // An event queue
FileSystem *fs = filesystem_selector();               // Device Management requires a filesystem, mount it (based on parameters in mbed_app.json)

SimpleMbedCloudClient *client;

// PIR sensor acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn pir(PIR_PIN);   // This pin value comes out mbed_app.json

// Resource declarations, they're assigned a value in `main`.
MbedCloudClientResource *ledColor;
MbedCloudClientResource *ledStatus;
MbedCloudClientResource *ledTimeout;
MbedCloudClientResource *pirCount;

// Variable that holds whether the light is on because the PIR sensor triggered (and timeout didn't happen yet)
bool ledOnBecauseOfPir = false;

// Timeout based on led/0/timeout, disables the light after a set interval
Timeout pirTimeout;

// // Permanent statuses (set by led/0/permanent_status)
enum PermanentStatus {
    STATUS_NONE = 0,
    STATUS_ON   = 1,
    STATUS_OFF  = 2
};

// clear the lights
void putLightsOff() {
    setRgbColor(0.0f, 0.0f, 0.0f);
}

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
void colorChanged(MbedCloudClientResource *resource, m2m::String newValue) {
    int status = ledStatus->get_value_int();

    if (ledOnBecauseOfPir || status == STATUS_ON) {
        putLightsOn();
    }
}

// Status changes from the cloud
void statusChanged(MbedCloudClientResource *resource, m2m::String newValue) {
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

// Use the built-in LED as a status LED
DigitalOut statusLed(LED1);
int        statusLedBlinkId;    // Callback ID
void blink_builtin_led() {
    statusLed = !statusLed;
}

void registered(const ConnectorClientEndpointInfo *endpoint) {
    // When we registered with Device Management, blink faster
    eventQueue.cancel(statusLedBlinkId);

    statusLedBlinkId = eventQueue.call_every(300, &blink_builtin_led);

    printf("Connected to Device Management. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
}

int main(int, char**) {
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

    client = new SimpleMbedCloudClient(network, storage_selector(), fs);
    client->on_registered(&registered);

    int init_rt = client->init();
    if (init_rt != 0) {
        printf("Failed to initialize Device Management Client (%d)", init_rt);
        return 1;
    }

    // Resource declarations
    ledColor = client->create_resource("3311/0/5706", "LED_Color");
    // We encode color in 3 bytes [R, G, B] and put it in an integer by providing the color as an hex value (default color: green)
    ledColor->set_value(0x00ff00);
    ledColor->methods(M2MMethod::GET | M2MMethod::PUT);
    ledColor->attach_put_callback(&colorChanged);

    ledTimeout = client->create_resource("3311/0/5853", "LED_Timeout");
    ledTimeout->set_value(5);
    ledTimeout->methods(M2MMethod::GET | M2MMethod::PUT);

    ledStatus = client->create_resource("3311/0/5850", "LED_Status");
    ledStatus->set_value(STATUS_NONE);
    ledStatus->methods(M2MMethod::GET | M2MMethod::PUT);
    ledStatus->attach_put_callback(&statusChanged);

    pirCount = client->create_resource("3201/0/5700", "PIR_Count");
    pirCount->set_value(0);
    pirCount->methods(M2MMethod::GET);
    pirCount->observable(true);

    printf("Connecting to Device Management...\n");

    // Start registering to the cloud.
    client->register_and_connect();

    // eventually you can also run this in a separate thread
    eventQueue.dispatch_forever();
}
```

#### Resources

The code sample above sets up the connection and declares some resources. You define a resource for every part of the program that needs to be available from the cloud:

- The color of the LED should be configurable.
- The period between the moment of motion detection to the moment lights go out should be configurable.
- There should be a permanent-on mode for the lights.
- You should notify Device Management whenever you detect movement.

Think of resources as pieces of information the device makes available. You can read or write to them from the cloud, and the device can use a resource's value to determine the correct action to perform. You can reach a resource with a URI and access modifier (for example, only write allowed), and you can also subscribe to them, so you receive a notification when a resource changes.

This application defines the following resources (under `// Resource declarations`):

* `3311/0/5706` - the color of the LED.
* `3311/0/5853` - the timeout (in seconds) after detection; lights are disabled when this period ends.
* `3311/0/5850` - whether you should have the lights permanently on (or off).
* `3201/0/5700` - the number of times the PIR sensor was triggered. Read only, and should allow notifications.

You can use the `create_resource` function to define extra resources and attach actions to each resource.

When you compile and flash this program, you'll see that when you wave your hand in front of the PIR sensor, the color of the LED changes to green, and the LED always turns off after 5 seconds.

When the connection to Device Management is created, the onboard LED blinks faster. You can now control this device from the cloud.

<span class="notes">**Note:** No connection? [Inspect the logs on the device](https://os.mbed.com/docs/v5.7/tutorials/serial-comm.html). Use baud rate 115,200 to communicate with your device.</span>

<span class="notes">**Note:** If you receive an `fcc_init` error, re-format the SD card (FAT). If your computer does not have an SD-card slot, see the [format-sd-card](https://os.mbed.com/users/janjongboom/code/format-sd-card/) Mbed program.</span>

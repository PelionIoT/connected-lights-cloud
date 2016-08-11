# Adding connectivity

Now that we built our basic circuit and wrote the code to control the circuit, we can start adding connectivity to the project. Part of the ARM mbed IoT Device Platform is mbed Cloud, a unified solution to connect devices to the internet and communicate with them regardless of *how* these devices connect to the internet. There are libraries available for a variety of connectivity methods, including Ethernet, WiFi, 6LoWPAN, Thread and Cellular.

## Obtaining a device certificate

All data that goes from the device to mbed Cloud (and vice-versa) is end-to-end encrypted by [mbed TLS](https://tls.mbed.org). To set up secure communications we need a security certificate. We can get one from the mbed Cloud website:

1. Go to the [mbed Cloud homepage](https://connector.mbed.com) and click *Sign in to get connected*.
1. If prompted for your login credentials, use your mbed developer account (the same account you use to log into the mbed Online Compiler).
1. Go to *My Devices* > *Security Credentials*.
1. Click *Get my security credentials*.
1. Copy the contents of the gray box. This is your certificate.


![The certificate is located in the gray box](assets/lights16.png)

Now go back to the online compiler, and create a new file ``security.h``. Paste the certificate in this file.

## Adding connectivity to the board

### Ethernet

If you have a development board that connects over Ethernet, just plug in an Ethernet cable. We’re assuming that the network has DHCP enabled and the firewall does not block connections to http://connector.mbed.com.

### ESP8266 WiFi module

To wire the ESP8266 module up to your development board, look at the [ESP8266 Cookbook page]().

### 6LoWPAN

First connect your 6LoWPAN gateway to an IPv6-enabled network by following the steps under 'Gateway Configuration' on [this page](https://github.com/ARMmbed/mbed-client-example-6lowpan#gateway-configuration). Then just click the 6LoWPAN shield on top of your development board.

## Adding libraries

For the device and mbed Cloud to talk we need the [mbed Client library](https://docs.mbed.com/docs/mbed-client-guide/en/latest/). This library is already included in mbed OS, and is very powerful, but can also be daunting for new users. In this example we'll use an additional library built on top of mbed Client: SimpleClient. This library is designed to easily expose variables and resources to the cloud.

We will also use [EasyConnect](https://github.com/ARMmbed/easy-connect) to handle connectivity.

To add these libraries to your project go back to the online compiler, and:

1. Right click on your program in the tree and select *Import Library > From URL*.
1. Under 'Source URL' enter: ``https://github.com/armmbed/easy-connect``.
1. Click *Import*.
1. Again, right click on your program and select *Import Library > From URL*.
1. Under 'Source URL' enter: ``https://developer.mbed.org/teams/sandbox/code/simple-mbed-client/``.
1. Click *Import*.

## Adding libraries with mbed CLI

If you are using mbed CLI, run the following commands to add the libraries:

```bash
$ mbed add https://github.com/ARMmbed/easy-connect
$ mbed add http://developer.mbed.org/teams/sandbox/code/simple-mbed-client/
```

## Updating configuration

We need to tell EasyConnect which connectivity method to use. Create a new file - ``mbed_app.json`` - and fill it with:

```json
{
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI,MESH_LOWPAN_ND,MESH_THREAD",
            "value": ""
        },
        "wifi-ssid": {
            "help": "WiFi SSID",
            "value": "\"SSID\""
        },
        "wifi-password": {
            "help": "WiFi Password",
            "value": "\"Password\""
        }
    }
}
```

Under `config.network-interface.value` select your connectivity method.

## Writing code

We need to add some code to the application so it connects to the internet and sets up a connection to mbed Cloud.

Replace ``main.cpp`` with:

```cpp
#include "mbed.h"
#include "rtos.h"
#include "config.h"
#include "security.h"
#include "easy-connect.h"
#include "simple-mbed-client.h"

SimpleMbedClient client;

// Declare our peripherals here
PwmOut red(PIN_RED);
PwmOut green(PIN_GREEN);
PwmOut blue(PIN_BLUE);

InterruptIn pir(PIN_PIR);

// We need a way to signal from an interrupt context -> main thread, use a Semaphore for it...
Semaphore updates(0);

// YOUR CODE HERE

// Registered callback for mbed Client
DigitalOut statusLed(LED1, 1);
void registered() {
  statusLed = 0;
}

int main(int, char**) {
  // Clear the channels on startup
  setIntensity(red, 0.0f);
  setIntensity(green, 0.0f);
  setIntensity(blue, 0.0f);

  // The PIR sensor uses interrupts, no need to poll
  pir.rise(&pir_rise);

  NetworkInterface* network = easy_connect(true);
  if (!network) {
    printf("Connect to internet failed... See serial output.\n");
    return 1;
  }

  struct MbedClientOptions options = client.get_default_options();
  options.DeviceType = "light-system";
  if (!client.setup(options, network)) {
    printf("Setting up mbed_client failed...\n");
    return 1;
  }

  client.on_registered(&registered);

  while (1) {
    int v = updates.wait(25000);
    if (v == 1) {
      pirCount = pirCount + 1;
    }
    client.keep_alive();
  }
}
```

The code sample above does not do much, except for setting up the connection. We can now define some logic for this program:

1. The color of the LED should be configurable.
1. The period between the moment of motion detection to the moment lights go out should be configurable.
1. There should be a permanent-on mode for the lights.
1. We should notify mbed Cloud whenever we detect movement.

We can implement these actions by defining resources. Resources are actions that can be read or written from the cloud, and which we can subscribe to (when a resource changes) on both device and cloud side. Resources are addressable under a path and an access modifier (for example, only write allowed). Let's define a resource for each of our actions:

* led/0/color - the color of the LED.
* led/0/timeout - the timeout (in seconds) after detection; lights are disabled when this period ends.
* led/0/permanent_status - whether we should have the lights permanently on (or off).
* pir/0/count - the number of times the PIR sensor was triggered. Read only, and should allow notifications.

We can use SimpleClient to define these resources and attach actions to each resource.

Replace `YOUR CODE HERE` in ``main.cpp`` with (comments inline):

```cpp
// Fwd declaration
void putLightsOn();
void colorChanged(int newColor);

// Variable that holds whether the light is on because the PIR sensor triggered (and timeout didn't happen yet)
bool ledOnBecauseOfPir = false;

// Timeout based on led/0/timeout, disables the light after a set interval
Timeout pirTimeout;

// Permanent statuses (set by led/0/permanent_status)
#define STATUS_NONE     0
#define STATUS_ON       1
#define STATUS_OFF      2

// clear the lights
void putLightsOff() {
  setIntensity(red, 0.0f);
  setIntensity(green, 0.0f);
  setIntensity(blue, 0.0f);
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
  setIntensity(red, static_cast<float>(redCh) / 255.0f);
  setIntensity(green, static_cast<float>(greenCh) / 255.0f);
  setIntensity(blue, static_cast<float>(blueCh) / 255.0f);
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
  // go back to main thread to update the resource (we can't do this in interrupt context)
  updates.release();

  // Permanent off? Don't put the lights on...
  if (ledStatus == STATUS_OFF) return;

  // Otherwise do it!
  ledOnBecauseOfPir = true;
  putLightsOn();

  // And attach the timeout
  pirTimeout.attach(&onPirTimeout, static_cast<float>(ledTimeout));
}
```

When you compile and flash this program, you'll see that when you wave your hand in front of the PIR sensor the color of the LED changes to green, and the LED always goes off after 5 seconds.

When the connection to mbed Cloud is created, the onboard LED will turn on. We can now control this device from the cloud.

<span class="notes">**Note:** No connection? [Inspect the logs on the device](https://developer.mbed.org/handbook/SerialPC#host-interface-and-terminal-applications).</span>


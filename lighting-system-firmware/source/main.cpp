#include "mbed.h"
#include "config.h"
#include "security.h"

#if CONNECTIVITY_ESP8266 == 1
#include "ESP8266Interface.h"
ESP8266Interface iface(UART_TX, UART_RX, true);
#elif CONNECTIVITY_ETHERNET == 1
#include "LWIPInterface.h"
LWIPInterface iface;
#else
#error "CONNECTIVITY_ESP8266 or CONNECTIVITY_ETHERNET should be declared"
#endif

#include "simple-mbed-client.h"
SimpleMbedClient client;

// Declare our peripherals here
PwmOut red(PIN_RED);
PwmOut green(PIN_GREEN);
PwmOut blue(PIN_BLUE);

InterruptIn pir(PIN_PIR);

// We need a way to signal from an interrupt context -> main thread, use a Semaphore for it...
Semaphore updates(0);

// Fwd declaration
void putLightsOn();
void colorChanged(int newColor);

// Variable that holds whether the light is on because the PIR sensor triggered (and timeout didnt happen yet)
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
// We encode color in 3 bytes [R, G, B] and put it in an Int by bit shifting the colors (default color: green)
SimpleResourceInt ledColor = client.define_resource("led/0/color", (0 << 16) + (255 << 8) + 0, &colorChanged);
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

  // This is where we connect to the internet
#if CONNECTIVITY_ESP8266 == 1
  int connect = iface.connect(WIFI_SSID, WIFI_PASSWORD);
#elif CONNECTIVITY_ETHERNET == 1
  int connect = iface.connect();
#endif

  if (connect != 0) {
    printf("Connect to internet failed... %d\n", connect);
    return 1;
  }

  // hmm.. why does Ethernet connection doesn't like this?
  // printf("IP address %s\r\n", iface.get_ip_address());

  struct MbedClientOptions options = client.get_default_options();
  options.DeviceType = "light-system";
  if (!client.setup(options, &iface)) {
    printf("Setting up mbed_client failed...\n");
    return 1;
  }

  client.on_registered(&registered);

  while (1) {
    int v = updates.wait(25000);
    if (v == 1) {
      // YOUR CODE HERE (2)
      pirCount = pirCount + 1;
    }
    client.keep_alive();
  }
}

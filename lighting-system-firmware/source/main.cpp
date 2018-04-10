#include "mbed.h"
#include "easy-connect.h"
#include "led.h"        // Abstracts away the differens between the LED types
#include "simple-mbed-cloud-client.h"
#include "storage-selector.h"

EventQueue eventQueue;                  // An event queue
Thread eventThread;                     // An RTOS thread to process events in
FileSystem* fs = filesystem_selector(); // Mbed Cloud requires a filesystem, mount it (based on parameters in mbed_app.json)
BlockDevice* bd = storage_selector();   // This is where the update client stores new firmware images

// PIR sensor acts as an interrupt - signals us whenever it goes high (or low)
InterruptIn pir(SW2);   // This pin value comes out mbed_app.json

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
MbedCloudClientResource *ledColor;
MbedCloudClientResource *ledTimeout;
MbedCloudClientResource *ledStatus;
MbedCloudClientResource *pirCount;

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
void colorChanged(MbedCloudClientResource*, m2m::String) {
    int status = ledStatus->get_value_int();

    if (ledOnBecauseOfPir || status == STATUS_ON) {
        putLightsOn();
    }
}

// Status changes from the cloud
void statusChanged(MbedCloudClientResource*, m2m::String) {
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

    SimpleMbedCloudClient client(network, bd, fs);
    client.on_registered(&registered);

    int init_rt = client.init();
    if (init_rt != 0) {
        printf("Failed to initialize Mbed Cloud Client (%d)", init_rt);
        return 1;
    }

    printf("Connecting to Mbed Cloud...\n");

    ledColor = client.create_resource("3311/0/5706", "LED_Color");
    ledColor->set_value(0x00ff00); // We encode color in 3 bytes [R, G, B] and put it in an integer by providing the color as an hex value (default color: green)
    ledColor->methods(M2MBase::GET_PUT_ALLOWED);
    ledColor->attach_put_callback(&colorChanged);

    ledTimeout = client.create_resource("3311/0/5853", "LED_Timeout");
    ledTimeout->set_value(5); // 5 seconds
    ledTimeout->methods(M2MBase::GET_PUT_ALLOWED);

    ledStatus = client.create_resource("3311/0/5850", "LED_Permanent_Status");
    ledStatus->set_value(STATUS_NONE);
    ledStatus->methods(M2MBase::GET_PUT_ALLOWED);
    ledStatus->attach_put_callback(&statusChanged);

    pirCount = client.create_resource("3201/0/5700", "PIR_Count");
    pirCount->set_value(0);
    pirCount->methods(M2MBase::GET_ALLOWED);
    pirCount->observable(true);

    // Start registering to the cloud.
    client.register_and_connect();

    wait(osWaitForever);
}

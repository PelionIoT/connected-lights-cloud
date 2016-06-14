#define LED_TYPE_ANODE    1
#define LED_TYPE_CATHODE  0

#define PIN_RED         D5
#define PIN_GREEN       D6
#define PIN_BLUE        D7
#define PIN_PIR         D2

#define CONNECTIVITY_ETHERNET     1

// #define CONNECTIVITY_ESP8266   1
// #define UART_TX                D1
// #define UART_RX                D0
// #define ESP8266_RESET          D3
// #define WIFI_SSID              "H368N66CCBC"
// #define WIFI_PASSWORD          "93FA2273AD79"

void setIntensity(PwmOut led, float value) {
#if LED_TYPE_ANODE == 1
  led = 1.0f - value;
#elif LED_TYPE_CATHODE == 1
  led = value;
#else
  printf("LED_TYPE should be ANODE or CATHODE\n");
#endif
}

#include <neopixel.h>

// -------
// IO Pins
// -------

#define PPM_DATA_PIN D5
#define LED_DATA_PIN D6

// ---------------
// Hardware Values
// ---------------

#define LED_COUNT 24
#define RX_CHANNEL_COUNT 8
#define LED_TYPE SK6812RGBW // Protocol for my specific LED ring setup

// --------------
// Radio Settings
// --------------

/*
  Each PPM frame can be of variable length, depending on the values of each
  channel. The start of a frame is indicated with a high state that lasts over
  2ms (2000μs). The delay between channel pulses is typically 300μs, and each
  channel pulse is normally between 700-1700μs, to give a channel range of
  1000-2000.
*/

#define RX_INITIAL_VALUE 0
#define SIGNAL_METHOD FALLING
#define START_FRAME_LENGTH 2100

// -----------
// Code Begins
// -----------

// Initialize adafruit neopixel ring using the particle neopixel library
Adafruit_NeoPixel ring(LED_COUNT, LED_DATA_PIN, LED_TYPE);

/*
  We need one additional "channel" that will be used to capture the start frame
  along with any delay between PPM frames. Channel 0 is used, which handily
  makes all the channels correspond with their array index value.
*/
int rx_channel_data[RX_CHANNEL_COUNT + 1];

void setup()
{
  for (int i = 0; i <= RX_CHANNEL_COUNT; i++)
  {
    rx_channel_data[i] = RX_INITIAL_VALUE;
  }
  ring.begin();
  pinMode(PPM_DATA_PIN, INPUT_PULLUP);
  attachInterrupt(
    PPM_DATA_PIN,
    ppm_interrupt,
    SIGNAL_METHOD
  );
}

void loop()
{
  // Clamp each value to it's expected range, then normalize it to be 0-100.
  uint16_t red = (
    (uint16_t)(constrain(rx_channel_data[2], 1500, 2000) - 1500) / 5
  );
  uint16_t green = (
    (uint16_t)(constrain(rx_channel_data[3], 1000, 2000) - 1000) / 10
  );
  uint16_t blue = (
    (uint16_t)(constrain(rx_channel_data[4], 1500, 2000) - 1500) / 5
  );
  uint16_t white = (
    (uint16_t)(constrain(rx_channel_data[5], 1000, 2000) - 1000) / 10
  );
  set_mono_color(red, green, blue, white);
  // Keep refresh rate of LEDs sane
  delay(25);
}

void set_mono_color(uint16_t red, uint16_t green, uint16_t blue, uint16_t white)
{
  // Clamp LEDs to 100 out of 255, both to prevent accidental overload if all
  // values are maxed, and to keep LEDs within a reasonable brightness limit.
  uint32_t clamped_color = ring.Color(
    constrain(red,   0, 100),
    constrain(green, 0, 100),
    constrain(blue,  0, 100),
    constrain(white, 0, 100)
  );
  for(int i = 0; i < LED_COUNT; i++) {
    ring.setPixelColor(i, clamped_color);
  }
  ring.show();
}

void ppm_interrupt()
{
  // TODO: Find efficient way to wait for start frame before running loop
  static byte current_channel = 0;
  // Fake shortest expected signal length for initial value
  static unsigned long int previous_time = micros() - 1000;
  unsigned long int current_time = micros();
  unsigned long int delta_time = current_time - previous_time;
  previous_time = current_time;

  if ((delta_time > START_FRAME_LENGTH) || (current_channel > RX_CHANNEL_COUNT))
  {
    current_channel = 0;
  }
  rx_channel_data[current_channel] = delta_time;
  current_channel++;
}

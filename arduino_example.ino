// -------
// IO Pins
// -------

#define PPM_DATA_PIN 2

// ---------------
// Hardware Values
// ---------------

#define RX_CHANNEL_COUNT 8

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

// -----------------
// Serial Monitoring
// -----------------

#define BAUD_RATE 115200

// -----------
// Code Begins
// -----------

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
  Serial.begin(BAUD_RATE);
  pinMode(PPM_DATA_PIN, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(PPM_DATA_PIN),
    ppm_interrupt,
    SIGNAL_METHOD
  );
}

void loop()
{
  write_ppm_to_serial();
  delay(25); // Keep the refresh rate of the serial monitor sane
}

void write_ppm_to_serial()
{
  Serial.print(rx_channel_data[0]);
  Serial.print("\t");
  for (byte i = 1; i <= RX_CHANNEL_COUNT; i++)
  {
    int parsed_output = constrain(rx_channel_data[i], 1000, 2000) - 1000;
    Serial.print(parsed_output);
    Serial.print("\t");
  }
  Serial.print("\n");
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

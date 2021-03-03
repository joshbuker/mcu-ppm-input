# -------
# IO Pins
# -------

# These should be the same pin (example given is Pin 5)

PPM_PIN = DigitalPin.P5
PPM_PIN_EVT_ID = EventBusSource.MICROBIT_ID_IO_P5

# ---------------
# Hardware Values
# ---------------

RX_CHANNEL_COUNT = 8

# --------------
# Radio Settings
# --------------

# Each PPM frame can be of variable length, depending on the values of each
# channel. The start of a frame is indicated with a high state that lasts over
# 2ms (2000μs). The delay between channel pulses is typically 300μs, and each
# channel pulse is normally between 700-1700μs, to give a channel range of
# 1000-2000.

SIGNAL_METHOD = EventBusValue.MICROBIT_PIN_EVT_FALL
START_FRAME_LENGTH = 2100

# -----------
# Code Begins
# -----------

rx_channel_data = [2000,0,0,0,0,0,0,0,0]

current_channel = 0
previous_time = input.running_time_micros()

# Unused in this example, but useful to have
def constrain(val, min_val, max_val):
    return min(max_val, max(min_val, val))

def ppm_interrupt():
    global current_channel
    global previous_time
    current_time = input.running_time_micros()
    delta_time = current_time - previous_time
    previous_time = current_time

    if (delta_time > START_FRAME_LENGTH) or (current_channel > RX_CHANNEL_COUNT):
        current_channel = 0
    rx_channel_data[current_channel] = delta_time
    current_channel += 1

def write_ppm_to_serial():
    serial.write_numbers(rx_channel_data)

def setup():
    pins.set_events(PPM_PIN, PinEventType.EDGE)
    pins.set_pull(PPM_PIN, PinPullMode.PULL_UP)
    control.on_event(PPM_PIN_EVT_ID, SIGNAL_METHOD, ppm_interrupt)

def loop():
    write_ppm_to_serial()
    basic.pause(25) # Keep the refresh rate of the serial monitor sane

setup()
basic.forever(loop)

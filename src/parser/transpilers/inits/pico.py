import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS as KeyboardLayout
from adafruit_hid.keycode import Keycode
kbd=Keyboard(usb_hid.devices)
layout=KeyboardLayout(kbd)

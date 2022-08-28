# vusb-keyboard

HID keyboard prototype with V-USB + ATmega328.

## circuit

ATmega328p 
  - PIN B0 -> switch for 'a'
  - PIN B1 -> switch for 'Shift'
  - PIN C0 -> LED through 10M register

USB
  - D+ -> PIN D2
  - D- -> PIN D4
  - ["USB Level conversion : Solution B"](http://vusb.wikidot.com/hardware)

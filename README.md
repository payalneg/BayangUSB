# Bayang USB receiver
Use Arduino pro micro + nrf24l01 as receiver andd Joystick for fly simulators.

Connection:

D15 - SCLK

D14 - MISO

D13 - MOSI

A9 - CE (Chip Enable)

A10 - CSn (Chip Select)


Don't forget to connect capacitor 100uF on VCC on nrf24l01.
If Arduino pro micro is 5V version, use an external 3.3V voltage regulator for nrf24l01.

# RFID_Scanner_OLED
Simple and cheap RFID/NFC scanner project

YouTube video:
https://youtu.be/RnljdPqzepk

## Connections:
### SSD1306 OLED

|SSD1306|Arduino|
|-----|-------|
| SDA | A4 |
| SCL | A5 |


### RC522 RFID module:

|RC522|Arduino|
|-----|-------|
|SDA| D10 (SS)|
|SCK | D13|
|MOSI | D11|
|MISO | D12|
|IRQ  | NC|
|GND | GND|
|RST | D9|
|3V3  | 3V3 (or 5V)|

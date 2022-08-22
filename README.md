# Smart wrist watch

![Display on](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/photo00.jpg)


The name is ironic as the watch can only display time and nothing else, not even an alarm feature.

The idea was to take a modern CPU and combine it with obsolete display technology, in this case [Vacuum fluorescent display (VFD)](https://en.wikipedia.org/wiki/Vacuum_fluorescent_display). The display has a very nice glow, which is for me impossible to capture with a camera, but in person the neon-like glowing digits really stand out (similar to nixie tubes).


![Display off](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/photo01.jpg)


# Code

Because the watch doesn't do anything else, the code is fairly trivial and currently just keeping it in [a single file](https://github.com/AntonKrug/smart_watch_mk2/blob/main/main.c).

It requires CodeVisionAVR C compiler for compilation (this will be fixed in the future when it will be converted to a GCC project).

# Schematic

The MCU can turn on/off the DC2DC section which produces the higher voltage needed for the VFD to operate,
this significantly decreases power consumption, because even at idle the DC2DC consumes significant current (from a battery-powered perspective). The filament heating can be turned off separately as well.

The RTC `DS3231M` chip connected through I2C keeps track of the current time when the MCU is in sleep mode.

The DC2DC calculations are based on `MC34063A` chip. I'm using `NCP3064` which is a similar device, but with extra features such as the ability to turn it off through a pin and a higher max frequency (150kHz instead of 100kHz).

The `MAX6920AWP` officially is not an SPI device but has DataIn `DIN` and DataClk `CLK` inputs. Connecting it as SPI peripheral and sending it two 8-bit SPI data packets works well. The device will truncate the higher (12-15) bits while having useful data in lower bits (0-11) as it's just a 12-bit device, but my SPI peripheral can only send 8-bit packets.

![schematic](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/schematic.png)


# TODO

- Add protection to sensitive corners of the glass VFD
- Add a strap so it can be used as a wrist watch
- Switch from CodeVisionAVR C compiler to GCC compiler
- Replace the bundled AVR C HAL with C++ implementation
- Take advantage of `WS2812B` neopixel and provide under-glow
- Add vibration motor to function as a alarm
- Try if the heating filament can be used for capacitive touch sensing
- Switch from AVR mega MCU to a [ESP32-C3](https://www.espressif.com/en/products/socs/esp32-c3)
- Then attempt to display Twitter messages. The 7-segment display is limited, but can 'render' alphanumerical characters.


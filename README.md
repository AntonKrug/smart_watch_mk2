# Smart wrist watch

![Display on](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/photo00.jpg)


Video demo (click on the thumbnail to play the video):

[![DIY Watch YT demo](https://img.youtube.com/vi/cu711Lb3NEY/0.jpg)](https://youtu.be/cu711Lb3NEY)


The name is ironic as the watch can only display time and can't do anything else, doesn't even have an alarm feature.

The idea was to take a CPU and combine it with obsolete display technology, in this case [Vacuum fluorescent display (VFD)](https://en.wikipedia.org/wiki/Vacuum_fluorescent_display). The `IVL2-7_5` display has a very nice glow, which is for me impossible to capture with a camera, the neon-like glowing digits stand out in person.


![Display off](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/photo01.jpg)


# Code

The watch just displays the time and doesn't do anything else, therefore the code is fairly trivial and currently kept in a handful of files:

- [main.c](https://github.com/AntonKrug/smart_watch_mk2/blob/main/main.c)
- [main.h](https://github.com/AntonKrug/smart_watch_mk2/blob/main/main.h)
- [reset.c](https://github.com/AntonKrug/smart_watch_mk2/blob/main/reset.c)
- [reset.h](https://github.com/AntonKrug/smart_watch_mk2/blob/main/reset.h)
- [vfd.c](https://github.com/AntonKrug/smart_watch_mk2/blob/main/vfd.c)
- [vfd.h](https://github.com/AntonKrug/smart_watch_mk2/blob/main/vfd.h)
- [neopixel.c](https://github.com/AntonKrug/smart_watch_mk2/blob/main/neopixel.c)
- [neopixel.h](https://github.com/AntonKrug/smart_watch_mk2/blob/main/neopixel.h)

It requires CodeVisionAVR C compiler for compilation (this will be fixed in the future when it will be converted to a GCC project).


# Resource utilization

| AVR Mega88 PA resources  | Used by this project  | Utilization  |
| ------------------------ | --------------------- | ------------ |
| 1024 bytes of RAM | 60 bytes used of the allocated 128 byte stack + 24 bytes global variables | 14%   |
| 8 KB | 3096 bytes | 38% |
| 512 bytes of EEPROM | 0 bytes | 0% |

Even when the AVR Mega88 doesn't have many resources, still most of them are free and available for future features.


# Compilation

Currently doesn't support GCC or any other free tools, these files need to be imported as a CodeVisionAVR project. I have a TODO to fix this.

# Schematic

The MCU can turn on/off the DC2DC section which produces the higher voltage needed for the VFD `IVL2-7_5` to operate,
this significantly decreases power consumption, because even at idle the DC2DC consumes significant current (from a battery-powered perspective). The filament heating can be turned off separately as well.

The RTC `DS3231M` chip connected through I2C keeps track of the current time when the MCU is in sleep mode.

The DC2DC calculations are based on `MC34063A` chip. I'm using `NCP3064` which is a similar device, but with extra features such as the ability to turn it off through a pin and a higher max frequency (150kHz instead of 100kHz).

The `MAX6920AWP` officially is not an SPI device but has DataIn `DIN` and DataClk `CLK` inputs. Connecting it as SPI peripheral and sending it two 8-bit SPI data packets works well. The device will truncate the higher (12-15) bits while having useful data in lower bits (0-11) as it's just a 12-bit device, but my SPI peripheral can only send 8-bit packets.

![schematic](https://raw.githubusercontent.com/AntonKrug/smart_watch_mk2/assets/images/schematic.png)


# TODO

- Add protection to sensitive corners of the glass VFD
- Add a strap so it can be used as a wristwatch
- Switch from CodeVisionAVR C compiler to GCC compiler
- Replace the bundled AVR C HAL with C++ implementation
- Add vibration motor to function as an alarm
- Try if the heating filament can be used for capacitive touch sensing
- Switch from AVR mega MCU to a [ESP32-C3](https://www.espressif.com/en/products/socs/esp32-c3)
- Then attempt to display Twitter messages. The 7-segment display is limited, but can 'render' alphanumerical characters.


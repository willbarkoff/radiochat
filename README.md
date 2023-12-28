# Radiochat

Radiochat is an embedded application that provides a WiFi interface to LoRa transmissions. It's designed to run on an RP2040 with an RFM 96W, and uses the Pico SDK, as well as the RadioLib library.

The RadioLib library doesn't natively support the RP2040, so I added a Hardware Abstraction Library, which may be of use in other projects: [`PicoHAL.hpp`](./PicoHAL.hpp)

This was my final project for the class [ECE 4760/5730: Digital Systems Design Using Microcontrollers](https://ece4760.github.io). You can read my lab report for this project [here](https://willbarkoff.dev/radiochat) (the source code for that website is in the [`docs/`](./docs/) subdirectory here).

## Building

First, install the Pico SDK (see Chapter 2 of [_Getting Started with Pico_](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)).

Next, the only other dependency, [RadioLib](https://github.com/jgromes/radiolib), is included as a [Git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules) in [lib/RadioLib](./lib/RadioLib). To make sure that RadioLib is downloaded, either use the `--recursive` flag while cloning. If you didn't do that, you can clone submodules by running this command:

```
$ git submodule update --init --recursive
```

Next, setup the build system.

```
$ mkdir build
$ cd build
$ cmake ..
```

And finally, you can build the project:
```
$ make
```

To upload it to a Pico, you can put it in BOOTSEL mode by holding down the BOOTSEL button and plugging in a USB cable. The Pico will mount as a drive. You can then drag the UF2 file that is produced directly onto the drive to upload the code.

When Radiochat is installed on a Pico, you can connect a momentary pushbutton between the RUN pin and ground. One press of this button will reset the program, and two presses of the button will put the Pico in BOOTSEL mode.

## Hardware

- Use a Pico W
- Use an RFM 96W. I used [this](https://www.adafruit.com/product/3073) adafruit breakout, but you will probably have success with other breakouts or even the module by itself.
    - DIO0 to GIPO 7
    - CS to GPIO 8
    - RESET to GPIO 9
    - DIO1 to GPIO 10
    - MISO to GPIO 16
    - SCK to GPIO 18
    - MOSI to GPIO 19
- (_Optional_) Connect a momentary pushbutton between RUN and GND on the Pico.

## Known issues

I will address these if/when I have time. PRs welcome.

- Sending messages doesn't work on Firefox on iOS. I'm not sure why. I think that it sends too many headers. Tentative plan is to use mitmproxy to inspect the request and figure out why.
- Sometimes the messages have extra characters on the end.
- I would love to implement websockets so you don't need to refresh, but I didn't have time to before the project was due.
- The UI is ugly. I should make it less ugly.

## License

Source code is under the [MIT License](./LICENSE.md). The writeup (in `docs/`) is under the [CC-BY-NC-ND 4.0 license](https://willbarkoff.dev/licenses/cc-by-nc-nd.html). Third-party code in `dhcpserver/` and `dnsserver/` are under the licenses included in LICENSE files in their respective directories, or at comments at the tops of files.
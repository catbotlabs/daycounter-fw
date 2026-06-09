# DayCounter K001 firmware

Source code for the firmware on the ATtiny84 that runs at the heart of the CatBotLabs K001 DayCounter Kit.

The DayCounter has a three-digit numeric display which displays the number of days until a set target date. This firmware makes the ATtiny84 calculate this number and then operate as a matrix driver to display it on the numeric display. The firmware also lets the user change the Target Date, RTC clock, and the Count Unit / Count Direction using a minimal-frustration 5-button OLED user interface.

The code ties these hardware elements together (internal name in bold):

* **Display**: Three-digit matrix-encoded seven segment display driven by a 74AHCT595 shift register. The active digit is selected by both the "DG0" GPIO and the QA output on the shift register.
* **Buttons**: Four directional buttons and one "Set" button. Software debounced using ATtiny84's timers.
* **Screen**: OLED screen breakout board, 128x32 interfaced via I2C bus implemented using ATtiny84's USI. **The OLED breakout provides the pullup resistors on SDA and SCL.**
* **RTC**: Battery-backed RTC (MCP7940N) on same I2C bus as the OLED screen.
* **Storage**: The ATtiny84's internal EEPROM is used for settings retention.

* **SW UART**: Optional one-wire software debug UART, 57600 baud.

### Usage

This project uses PlatformIO as the build / dependency / upload framework.

You'll need to use the default `attiny84` PlatformIO environment.

To build:

```
pio run
```

To upload via a `stk500v1` compatible programmer (default port `/dev/ttyUSB0`, baud 115200):

```
pio run -t upload
```

### Unit testing

This project aims to be tested as much as reasonably possible for stability.

As opposed to using simulation / emulation methods, we decided to achieved this by unit testing the more abstract code sections. **Specifically, the `lib/` directory contains source code that is unit-tested.**  No AVR-specific code is tested; we have not endeavored to "mock" hardware-adjacent functions/macros. The unit tests are run on your computer and not the microcontroller. 

The unit tests are contained in the `test/` directory.

**To run the unit tests, you must specify a different PlatformIO environment,** because the default environment would build for the target AVR microcontroller.

To build and run all standard unit tests:

```
pio test -e test_native
```

Some extra-thorough, brute-force unit tests take more time to run than the others. These are enabled by the `DC_MAX_TESTING` define, which is set if you instead use the `test_native_max` PlatformIO environment.

To build and run all unit tests, including the sluggish ones:

```
pio test -e test_native_max
```

### Libraries

#### External dependencies

* [`ssd1306`](https://github.com/cbl-winston/ssd1306) (CatBotLabs fork): Ultra lightweight SSD1306 driver library. The fork lets us disable unused features, saving valuable program space.

* [`tiny-i2c`](https://github.com/technoblogy/tiny-i2c): Ultra lightweight AVR I2C implementation.

#### Private library compontents (lib/)

* `datetime/`: Defines Date and Time structures, date math algorithms, string formatting, and RTC register data conversion.
* `display/`: Defines the numeric display character constants and the integer / display character conversion function.
* `util/`: Uncategorized general-purpose functions

### GPIO usage

This diagram shows how we are using the GPIO's on the ATtiny84.

We were rather constrained by GPIO count, so we had to come up with some creative ways to fit multiple functions on one GPIO in order to use only 11 GPIOs.

The Reset pin is not used as a GPIO pin because it significantly complicates the upload process.

```

O/I   SIG    PIN       PIN    SIG      I/O   
                  _ _
                 | U |
<-    SRC    PB0 |   | PA0    BTVT     <-
<-    SRD    PB1 |   | PA1    BTSET    <-
       NC    PB3 |   | PA2    UART/OE  <->
 ->   PPS    PB2 |   | PA3    BTHZ     <-
<-    SRL    PA7 |   | PA4    SCL      <->
<->   SDA    PA6 |___| PA5    DG0       ->

```

* BTVT - PA0: Button signal shared by the two vertical arrow buttons, Down and Up.
If either button is pressed, the signal is read as low. The ADC is used to
tell which of the two buttons is pressed. If Up is pressed, voltage is ~0V,
or if Down is pressed, voltage is about 0.45V. If both are pressed,
the Up button will "dominate" (signal at ~0V).
The PCINT interrupt signals when a button is pressed.

* BTSET - PA1: Active-low "Set" button signal with internal pullup enabled.
The PCINT interrupt signals when this button is pressed.

* UART/OE - PA2: This pin is primarily the OE pin for the display shift register.
The OE function blanks the display when HIGH and enables output on LOW. The firmware uses
the OE pin to eliminate ghosting during the transitions between the digits.
Pull up this pin at startup to enable the debug UART interface, disabling the digital display.
When used as a UART pin, this is a shared TX/RX pin.

* BTHZ - PA3: Button signal shared by the two horizontal arrow buttons, Left and Right.
If either button is pressed, the signal is read as low. The ADC is used to
tell which of the two buttons is pressed. If Right is pressed, voltage is ~0V,
or if Left is pressed, voltage is about 0.45V. If both are pressed,
the Right button will "dominate" (signal at ~0V).
The PCINT interrupt signals when a button is pressed.

* SCL - PA4: SCL signal for the I2C bus shared with the RTC and the OLED screen.

* SDA - PA6: SDA signal for the I2C bus shared with the RTC and the OLED screen.

* DG0 - PA5: Digital display digit 0 (ones place) sink enable pin. When DG0 is high, then the
shift register output is sunk through the ones place common anode in the digital display.
Note that the DG1 (tens place) signal works the same way, but it is one of the shift register outputs.
If both DG0 and DG1 are low, then DG2 (hundreds place) is sunk.

* SRC - PB0: Shift Register Clock for the digital display shift register.

* SRD - PB1: Shift Register Data for the digital display shift register.

* SRL - PA7: Shift Register Latch for the digital display shift register.

* PPS - PB2: Pulse-per-second from RTC's MFP pin. Since the MFP pin is open-drain, the
pullup resistor must be set on PB2. The ATtiny reads the time from the RTC upon startup,
then keeps track of time using only the PPS signal.

### Code standards

The C++ code largely follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html). Code in the `src/`, `include/`, `lib/`, and `test/` directories is linted using [cpplint](https://github.com/cpplint/cpplint). See the `tools/lint.sh` helper script.

### License

This firmware and its source code is free and open source, under the terms of the MIT License in the included LICENSE file.

Note that the `src/uart.cpp` file has some embedded BSD 2-Clause License code from [attiny13-software-uart-library](https://github.com/lpodkalicki/attiny13-software-uart-library) by lpodkalicki.

Copyright (c) 2026, Winston Malinowski

# DayCounter K001 firmware

This repository contains the source code for the firmware that runs on the microcontroller in the CatBotLabs K001 DayCounter Kit.


# GPIO usage:

```

O/I   SIG    PIN       PIN    SIG      I/O   
                  _ _
                 | U |
<-    SRC    PB0 |   | PA0    
<-    SRD    PB1 |   | PA1    
       NC    PB3 |   | PA2    UART/OE  <->
 ->   PPS    PB2 |   | PA3    
<-    SRL    PA7 |   | PA4    SCL      <->
<->   SDA    PA6 |___| PA5    DG0       ->

```

* UART/OE - PA2: This pin is primarily the OE pin for the display shift register.
The OE function blanks the display when HIGH and enables output on LOW. The firmware uses
the OE pin to eliminate ghosting during the transitions between the digits.
Pull up this pin at startup to enable the debug UART interface, disabling the digital display.
When used as a UART pin, this is a shared TX/RX pin.

* SCL - PA4: SCL pin for the I2C bus shared with the RTC and the OLED screen.

* SDA - PA6: SDA pin for the I2C bus shared with the RTC and the OLED screen.

* DG0 - PA5: Digital display digit 0 (ones' place) sink enable pin. When DG0 is high, then the
shift register output is sunk through the ones' place common anode in the digital display.
Note that the DG1 (tens' place) signal works the same way, but it is one of the shift register outputs.
If both DG0 and DG1 are low, then DG2 (hundreds' place) is sunk.

* SRC - PB0: Shift Register Clock for the digital display shift register.

* SRD - PB1: Shift Register Data for the digital display shift register.

* SRL - PA7: Shift Register Latch for the digital display shift register.

* PPS - PB2: Pulse-per-second from RTC's MFP pin. Since the MFP pin is open-drain, the
pullup resistor must be set on PB2. The ATtiny reads the time from the RTC upon startup,
then keeps track of time using only the PPS signal.

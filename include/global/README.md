The `include/global/` directory is given as an include path for all sources
during compilation, including external PlatformIO libraries.

Specifically, this is here so that thy TinyI2C "arduino" library include have
its "Arduino.h" file and be happy. TinyI2C does not require arduino in this
case, but does need a few defines.
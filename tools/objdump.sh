#!/bin/bash

avr-objdump -S .pio/build/attiny84/firmware.elf | less

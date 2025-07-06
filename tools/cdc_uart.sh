#!/bin/bash

PORT=/dev/ttyACM0

if [[ "$1" != "" ]]; then
	PORT="$1"
fi

minicom -D $PORT -b 57600

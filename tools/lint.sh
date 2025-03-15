#!/bin/bash

echo "--- TODO list:"
grep --color=auto -r TODO src/ include/ lib/ test/
echo "--- end TODO list ---"

cpplint --recursive \
    src/ include/ lib/ test/ 2>&1 |
    grep --color=always ".*:[0-9]*:\|$"


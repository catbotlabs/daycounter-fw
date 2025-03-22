#!/bin/bash

echo "--- TODO list:"
grep --color=auto -r -n TODO src/ include/ lib/ test/
echo "--- end TODO list ---"

echo "--- cpplint issues:"

cpplint --recursive \
    src/ include/ lib/ test/ 2>&1 |
    grep -v "^Done"  |   # Exclude "Done processing" lines
    grep --color=always ".*:[0-9]*:\|$"  # Color the filenames red


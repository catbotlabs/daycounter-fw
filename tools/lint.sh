#!/bin/bash
cpplint --recursive \
    --filter -build/include_subdir \
    src/ include/ lib/

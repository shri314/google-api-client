#!/bin/bash

CDPATH= && cd "$(dirname "$0")"

( [ -f build/.cmade ] || mkdir -p build && cd build && cmake .. && touch .cmade )

set -e
BOOST_TEST_COLOR_OUTPUT=0
make -s -C build
make -s -C build/test all test

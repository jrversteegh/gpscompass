#!/bin/sh

rm -rf build
echo "constexpr time_t compile_time_epoch = $(date +%s);" > src/compile_time.h
west build .
west flash

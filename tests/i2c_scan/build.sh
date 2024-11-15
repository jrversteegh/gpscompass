#!/bin/sh

rm -rf build
west build .
west flash

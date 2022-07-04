#!/bin/bash

rm -rf build
mkdir build
cd build

cmake -D RELEASE=1 ..
make

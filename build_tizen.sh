#!/bin/bash

ARCH=i586
# ARCH=x86_64

BUILD_TYPE="DEBUG"

gbs --conf gbs/gbs_tizen6.5.conf build -P tizen6.5 -A $ARCH --incremental --clean --include-all --threads 1 --define 'build_type '$BUILD_TYPE


#!/bin/bash
# download and extract intel pin
PIN_ARCHIVE="pin-external-3.31-98869-gfa6f126a8-gcc-linux"
PIN_URL="https://software.intel.com/sites/landingpage/pintool/downloads/${PIN_ARCHIVE}.tar.gz"

pushd ..

if [ ! -d "$PIN_ARCHIVE" ]; then
    curl "$PIN_URL" | tar zxf -
fi
if [ ! -d "$PIN_ARCHIVE" ]; then
    echo "Error: Failed to download or extract Intel Pin archive." >&2
    exit 1
fi

export PIN_ROOT=$(realpath `ls -d pin*`)
popd
# builds the custom library (pintool)
make
echo "export PIN_ROOT=$PIN_ROOT" > env
# build code coverage target example
#pushd example && make && popd
# run code coverage tool
#$PIN_ROOT/pin -t ./obj-intel64/FuncTracer.so -- example/cov_sample 7 3




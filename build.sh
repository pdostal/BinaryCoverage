# download and extract intel pin
pushd ..
test -d pin* || curl https://software.intel.com/sites/landingpage/pintool/downloads/pin-external-3.31-98869-gfa6f126a8-gcc-linux.tar.gz | tar zxf -
export PIN_ROOT=$(realpath `ls -d pin*`)
popd

# builds the custom library (pintool)

make

# build code coverage target example
pushd example && make && popd

# run code coverage tool

#$PIN_ROOT/pin -t ./obj-intel64/FuncTracer.so -- example/cov_sample 7 3




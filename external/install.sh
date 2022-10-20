#! /bin/bash

# elevate to sudo
if [ $EUID != 0 ]; then
    sudo "$0" "$@"
    exit $?
fi

echo "copying .so files from ./lib to /usr/lib"
cp -v ./lib/* /usr/lib/

echo # add a blank line to output
echo "copying .h files from ./include to /usr/include"
cp -v ./include/* /usr/include

echo # add a blank line to output
echo "copying config files from ./config to /etc/qrng-service"
mkdir -p /etc/qrng-service
cp -v ./config/* /etc/qrng-service
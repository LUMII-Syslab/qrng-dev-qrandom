#! /bin/bash

# elevate to sudo
if [ $EUID != 0 ]; then
    sudo "$0" "$@"
    exit $?
fi

rm_file () {
    if [ -f $1 ]; then rm -vf $1
    else echo "$1 doesn't exist"
    fi
}

echo "removing qrng-service .so files from /usr/lib"
LIB_PREFIX=/usr/lib
for FILE in ./lib/*; do
    NAME=$(basename $FILE)
    FILE_PATH=${LIB_PREFIX}/${NAME}
    rm_file $FILE_PATH
done

echo # add a blank line to output
echo "removing qrng-service .h files from /usr/include"
INC_PREFIX=/usr/include
for FILE in ./include/*; do
    NAME=$(basename $FILE)
    FILE_PATH=${INC_PREFIX}/${NAME}
    rm_file $FILE_PATH
done

echo # add a blank line to output
echo "removing /etc/qrng-service directory"
if [ -d /etc/qrng-service ]; then rm -rvf /etc/qrng-service
else echo "directory /etc/qrng-service doesn't exist"
fi
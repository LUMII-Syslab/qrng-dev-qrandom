# QRNG service - replacement for `/dev/random`

The repository contains a makefile which will build and install a character device and a userspace service. The userspace service feeds random bytes into the kernelspace character device driver through `/dev/qrandom0`. The bytes can then be read through `/dev/qrandom0` instead of those provided by `/dev/random`.

The project is meant to fetch random bytes from [Remote Quantum Random Number Generator](https://qrng.lumii.lv/) but can adapted to suit other requirements.

## requirements

Building the project requires kernel header files. Acquiring them is described in [LKMPG#headers](https://sysprog21.github.io/lkmpg/#headers).

## usage

To build and install the service, `cd` into the root of the repo and run `make install`.

The file can then be read from. Reading operation will be blocked untill data is available unless `O_NONBLOCK` flag is specified when opening the file with [open](https://man7.org/linux/man-pages/man2/open.2.html).

The random bytes generated can be seen by running the `od -vAn -N256 -tu1 < /dev/qrandom0` command.

## kernel module

The kernel module is installed into `/usr/lib/modules/$(shell uname -r)/extra` and listed in `/etc/modules-load.d/qrng-driver.conf` for automatic load during boot.

The module registers and creates a character device file `/dev/qrandom0`. The file can be read from, written to, polled with sudo privileges.

## userspace service

The userspace service executable is installed into `/opt/qrng-service` and a .service file placed in `/etc/systemd/system/qrng.service` for access by [Systemd](https://en.wikipedia.org/wiki/Systemd). 

The userspace qrng service polls `/dev/qrandom0` and is blocked in an interruptible manner until data can be written to the file. 

Its logs can be viewed through the `systemctl status qrng` command.

The service keeps a buffer of random bytes fetched through `qrng.lumii.lv` for use in the future.

## qrng.lumii.lv

`qrng-client` is already compiled into a shared object and placed in the `./lib` directory. It can be manually compiled and replaced by following the instructions in its [repo](https://github.com/LUMII-Syslab/qrng-client). Provided are also header files for `qrng-client` placed into `./include` directory.

For more information visit [qrng.lumii.lv](https://qrng.lumii.lv/).

To use QRNG web service 3 files are required to be placed into `./config` directory:
* ca.truststore (the root CA certificate used to sign the QRNG server HTTPS certificate and client sertificates)
* token.keystore (your client certificate, signed by the CA that serves the QRNG server)
* qrng.properties (key passwords and other settings)

The files are currently provided upon request by mailing to syslab_services at lumii.lv
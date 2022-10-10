# QRNG service

This repo contains a `makefile` which builds and can also install a systemd service that replaces `/dev/random` random numbers with those fetched by [qrng-client](https://github.com/LUMII-Syslab/qrng-client).

Systemd provides a system and service manager that runs as PID 1 and starts the rest of the system.

`qrng-client` is already compiled into a shared object and placed in the `./lib` directory. It can be manually compiled and replaced by following the instructions in its [repo](https://github.com/LUMII-Syslab/qrng-client). Provided are also header files for `qrng-client` placed into `./include` directory.

## Prerequisites

In order to connect to our QRNG web service, you will need the QRNG client native library and these files:

ca.truststore (the root CA certificate used to sign the QRNG server HTTPS certificate and client sertificates)
token.keystore (your client certificate, signed by the CA that serves the QRNG server)
qrng.properties (key passwords and other settings)

## Usage

To build and install the service, cd into the root of the repo and run `make install`.
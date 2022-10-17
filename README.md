(not functional W.I.P.)
# QRNG service

This repo contains a `makefile` which builds and can also install a systemd service that replaces `/dev/random` random numbers with those fetched by [qrng-client](https://github.com/LUMII-Syslab/qrng-client).

Systemd provides a system and service manager that runs as PID 1 and starts the rest of the system.

`qrng-client` is already compiled into a shared object and placed in the `./lib` directory. It can be manually compiled and replaced by following the instructions in its [repo](https://github.com/LUMII-Syslab/qrng-client). Provided are also header files for `qrng-client` placed into `./include` directory.

For more information visit [qrng.lumii.lv](https://qrng.lumii.lv/).

## Diagram

[](./diagram.png)

## Prerequisites

To use QRNG web service 3 files are required to be placed into `./config` directory:
* ca.truststore (the root CA certificate used to sign the QRNG server HTTPS certificate and client sertificates)
* token.keystore (your client certificate, signed by the CA that serves the QRNG server)
* qrng.properties (key passwords and other settings)

The files are currently provided upon request by mailing to syslab_services at lumii.lv

## Usage

To build and install the service, `cd` into the root of the repo and run `make install`.
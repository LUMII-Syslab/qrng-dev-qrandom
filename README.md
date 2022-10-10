# QRNG service

This repo contains a makefile which builds and can also install a systemd service that replaces /dev/random random numbers with those fetched by [qrng-client](https://github.com/LUMII-Syslab/qrng-client).

Systemd provides a system and service manager that runs as PID 1 and starts the rest of the system.

## Usage

To install the service, cd into the root of the repo and run `make install`.
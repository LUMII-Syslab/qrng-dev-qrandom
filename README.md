# QRNG service - replacement for `/dev/random`

The repository contains a `Makefile` which will build and install both a character device
and a [userspace](https://en.wikipedia.org/wiki/User_space_and_kernel_space) service.

The userspace service feeds random bytes into the
kernelspace [character device driver](https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html).
These bytes can then be read from `/dev/qrandom0` which serves as a replacement for `/dev/random`.

The solution is intended to fetch random bytes from a
remote [QRNG](https://en.wikipedia.org/wiki/Hardware_random_number_generator)
provided by [qrng.lumii.lv](https://qrng.lumii.lv/)
but can be adapted to suit other requirements. By default, it uses a [PRNG](https://en.wikipedia.org/wiki/Pseudorandom_number_generator).

## Requirements

Building the project requires kernel header files.
Acquiring them is described in [LKMPG#headers](https://sysprog21.github.io/lkmpg/#headers).

To fetch random bytes from the remote [QRNG](https://en.wikipedia.org/wiki/Hardware_random_number_generator)
provided by [qrng.lumii.lv](https://qrng.lumii.lv/),
the project also requires [qrng-client](https://github.com/LUMII-Syslab/qrng-client) library
as well as swapping out `/userspace/src/main.cpp` `PRNG` in line `RNG *rng = new PRNG();` with `QRNG`.

`qrng-client` is provided in the repo with both install and uninstall scripts for ease of use.
However, it is not included in the `Makefile` and has to be installed manually.

## Usage

To build and install the service, `cd` into the root of the repo and run `make install`.
The `/dev/qrandom0` file can then be read from.

Reading operation will be blocked until data is available unless `O_NONBLOCK`
flag is specified when opening the file with [open](https://man7.org/linux/man-pages/man2/open.2.html).

The random bytes retrieved can be seen by running the `od -vAn -N256 -tu1 < /dev/qrandom0` command.

## Kernel module

The kernel module is installed into `/usr/lib/modules/$(shell uname -r)/extra`
and listed in `/etc/modules-load.d/qrng-driver.conf` for automatic load during boot.

The module registers and creates a character device file `/dev/qrandom0`.
The file can be read from, written to, polled with sudo privileges.

As of now on linux kernel version 6.4.3 the module compiles successfully.
This might not be the case in the near future as there were breaking changes recently.
To check the kernel version one can run `uname -r`.

## Userspace service

The userspace service executable is installed into `/opt/qrng-service` and a `.service` file placed in `/etc/systemd/system/qrng.service` for access by [Systemd](https://en.wikipedia.org/wiki/Systemd) and automatic load during boot. 

The userspace qrng service polls `/dev/qrandom0` and is blocked in an interruptible manner until data can be written to the file. 

Its logs can be viewed through the `systemctl status qrng` command.

The userspace process can crash either because it can't find shared object file
or because it lacks permissions to write to `/dev/qrandom0`.

To fix the former issue one can create `.conf` file in `/etc/ld.so.conf.d` that includes `/usr/lib`.

The service keeps a buffer of random bytes fetched through RNG interface for use in the future.


## qrng.lumii.lv

As mentioned above the service is by default configured to use PRNG that sits behind an RNG interface. To swap it out for QRNG provided by [qrng.lumii.lv](https://qrng.lumii.lv/) additional requirements have to be satisfied.

To use QRNG web service 3 files are required to be placed into `/etc/qrng-service` directory:
* ca.truststore (the root CA certificate used to sign the QRNG server HTTPS certificate and client sertificates)
* token.keystore (your client certificate, signed by the CA that serves the QRNG server)
* qrng.properties (key passwords and other settings)

The files are currently provided upon request by mailing to syslab_services at lumii.lv

## Benchmarks

Reading byte by byte leads to the following results:

| `DEVICE_PATH`   | 32 bit integers | time (seconds) |
|---------------|-------------|----------------|
| /dev/random   | 1000000     | 0.244270 sec   |
| /dev/qrandom0 | 1000000     | 0.592094 sec   |

There is also `/testing/distribution.py` which plots the distribution of bytes among fetched.

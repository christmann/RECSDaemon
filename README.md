# RECSDaemon

## Introduction

The RECSDaemon is a small program that can be installed on compute modules in a christmann RECS®|Box system to be able to forward OS-level monitoring data to the integrated management system of the RECS®|Box. It is written to be cross-platform, running on Microsoft Windows as well as Linux and on x86, x64 and ARM systems. To be able to adapt to different platforms, the RECSDaemon uses plugins for different purposes. To configure these plugins and other settings an .ini file is used. The RECSDaemon is also able to execute commands sent by the management system to the node (e.g. for shutting down the OS gracefully).

## Building

### Install necessary build libraries and build system

On a Debian/Ubuntu based system, install the following libraries:

```bash
sudo apt-get install build-essential libssl-dev pkg-config cmake
```

### Compile

RECSDaemon uses CMake as it's build system.

To build:
```bash
mkdir build
cd build
cmake ..
make
```

## User manual

The user manual detailing installation and configuration of the RECSDaemon can be found in the RECS|Box Wiki:
https://recswiki.christmann.info/wiki/doku.php?id=documentation:recsdaemon

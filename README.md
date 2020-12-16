# SndToWAV
A utility to convert `'snd '` Apple Sound Manager sounds to WAV.

# Features
* Easily extract sound samples from `.rsrc` files.
* Supports all sound header formats.
* Supports IMA4 compression.
* (Should be) platform independent.

# Limitations
* Currently only supports IMA4 compression.
* Can only output `.wav` files.

# Installation
### Dependencies
* [git](https://git-scm.com/downloads) (recommended)
* [cmake>=3.7.0](https://cmake.org/download/)
* Your favorite C++11 compliant compiler

### Building
To download and build, execute:

    git clone --recurse-submodules https://github.com/fordcars/SndToWAV
    cd SndToWAV && mkdir build && cd build
    cmake ../src
    make
  

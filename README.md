# Multi-thread OTDR server

Multi-thread OTDR server on Qt. Designed to work with OTDR "Tapaz"

## Requirements

* C++11 compiler support
* Qt5

## Build

    cd 
    mkdir build
    qmake ..
    make
    
## At Linux

    qmake -DENABLE_EPOOL=ON ..

## Run

    OTDERServer -t 4 -p 30014
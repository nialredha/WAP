#!/bin/bash

mkdir -p build
mkdir -p lib

cd build/

clang++ -c ../src/SoundSim.cpp ../src/WaveIO.cpp -I ../include
ar cr ../lib/libsound.a SoundSim.o WaveIO.o

clang++ ../src/main.cpp -lSound -lSDL2 -I ../include -L ../lib -o Audio

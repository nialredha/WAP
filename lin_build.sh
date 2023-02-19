#!/bin/bash

mkdir -p build
mkdir -p lib

cd build/

clang++ ../src/main.cpp ../src/audio.cpp ../src/graphics.cpp -lSDL2 -lSDL2_image -l SDL2_ttf -I ../src -o wap 

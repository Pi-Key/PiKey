#!/bin/bash

mkdir -p dist

gcc -o ./dist/pikey ./src/*.c -lm

#!/bin/bash

mkdir -p dist

gcc -o ./dist/pikey ./src/pikey.c ./src/*/*.c -lm

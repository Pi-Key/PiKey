#!/bin/bash

#files=($(echo ../src/*/*.c | tr ' ' '\n' | tac | tr '\n' ' '))

gcc -o ./dist/pikey ../src/pikey.c ../src/*/*.c -lm

if [ $? -eq 0 ]; then
  echo "Build done."
else
  echo "There was an error during compilation."
fi

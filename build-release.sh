#!/bin/bash

clang src/main.cpp src/bf_compiler.cpp -std=c++20 -O3 -Wall -Wextra -o out/bf

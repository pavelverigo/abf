#!/bin/bash

clang src/main.cpp src/bf_compiler.cpp -std=c++20 -g  -Wall -Wextra -Werror -o out/bf

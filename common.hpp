#pragma once

#include <iostream>
#include <cstdlib>
#include <string>

inline void panic(const std::string& message) {
    std::cerr << "PANIC: " << message << "\n";
    std::exit(EXIT_FAILURE);
}
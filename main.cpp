#include <iostream>
#include <string>
#include "bf_compiler.hpp"

void print_usage() {
    std::cerr << "Usage:\n";
    std::cerr << "  ./bf_comp text (--simple)\n";
    std::cerr << "  ./bf_comp llvm (--simple)\n";
}

enum Mode {
    Text,
    LLVM,
};

int main(int argc, char* argv[]) {
    if (argc > 3) {
        std::cerr << "Error: Incorrect number of arguments.\n";
        print_usage();
        return 1;
    }
    
    std::string argument = argv[1];

    Mode mode;
    if (argument == "text") {
        mode = Text;
    } else if (argument == "llvm") {
        mode = LLVM;
    } else {
        std::cerr << "Error: No such mode.\n";
        print_usage();
        return EXIT_FAILURE;
    }

    bool simple = false;
    if (argc == 3) {
        std::string opt = argv[2];
        if (opt == "--simple") {
            simple = true;
        } else {
            std::cerr << "Error: No such option.\n";
            print_usage();
            return EXIT_FAILURE;
        }
    }

    std::vector<Inst> raw;
    rawbf_in(std::cin, raw);

    std::vector<Inst> res;
    std::vector<Inst> temp = raw;

    {
        concat_pass(temp, res);
        
        if (!simple) {
            res.swap(temp);
            res.clear();
            zeroadd_pass(temp, res);
        }
    }

    switch (mode) {
    case Text: {
        textinst_out(std::cout, res);
    } break;
    case LLVM: {
        textllvm_out(std::cout, res);
    } break;
    }

    return EXIT_SUCCESS;
}

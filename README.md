# Advanced brainfuck compiler

Usage:

./bf_comp text (--simple)

./bf_comp llvm (--simple)

# Optimize and run LLVM 

opt -S -O3 < file.ll > opt.ll

lli opt.ll

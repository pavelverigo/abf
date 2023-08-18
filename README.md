# Advanced brainfuck compiler

Compile:

```
./build-debug.sh
```

Run:

```
./out/bf llvm < ./programs/helloworlf.b > ./out/llvm.ll
lli ./out/llvm.ll
```

Optimize by LLVM:

```
opt -S -O3 < ./out/llvm.ll > ./out/opt.ll
```

Test:

```
./test/test.py
```
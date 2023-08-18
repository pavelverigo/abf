#!/usr/bin/env python3

import unittest
import subprocess

import bf_interpreter

def run_bf_compiler(input_file):
    with open(input_file, "r") as infile, open("out/out.ll", "w") as outfile:
        result = subprocess.run(
            ["./out/bf", "llvm"],
            stdin=infile,
            stdout=outfile,
            stderr=subprocess.PIPE
        )

    if result.returncode != 0:
        raise Exception(f"bf_compiler exited with error code {result.returncode}\n{result.stderr}")

def run_lli(input_string=""):
    result = subprocess.run(
        ["lli", "./out/out.ll"],
        input=input_string,
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True
    )
    
    if result.returncode != 0:
        raise Exception(f"lli exited with error code {result.returncode}\n{result.stderr}")
    
    return result.stdout

class LLVM(unittest.TestCase):
    def test_squares(self):
        input_file = "./programs/squares.b"
        input_str = ""
        
        run_bf_compiler(input_file)
        output1 = run_lli(input_str)

        with open(input_file, 'r') as file:
            code = file.read()
            output2 = bf_interpreter.interpret_static(code, input_str)

        self.assertEqual(output1, output2)

    def test_io(self):
        input_file = "./programs/io.b"
        input_str = "a"
        
        run_bf_compiler(input_file)
        output1 = run_lli(input_str)

        with open(input_file, 'r') as file:
            code = file.read()
            output2 = bf_interpreter.interpret_static(code, input_str)

        self.assertEqual(output1, output2)

    def test_helloworld(self):
        input_file = "./programs/helloworld.b"
        input_str = ""
        
        run_bf_compiler(input_file)
        output1 = run_lli(input_str)

        with open(input_file, 'r') as file:
            code = file.read()
            output2 = bf_interpreter.interpret_static(code, input_str)

        self.assertEqual(output1, output2)

if __name__ == '__main__':
    unittest.main(verbosity=2)
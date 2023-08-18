#!/usr/bin/env python3

import sys

def preprocess(code):
    jump_table = {}
    stack = []
    
    for i, char in enumerate(code):
        if char == '[':
            stack.append(i)
        elif char == ']':
            if stack:
                start = stack.pop()
                jump_table[start] = i
                jump_table[i] = start
            else:
                raise SyntaxError("Mismatched brackets")
                
    if stack:
        raise SyntaxError("Mismatched brackets")
    
    return jump_table

def interpret_static(code, input_str=''):
    code = [c for c in code if c in ('<', '>', '+', '-', '.', ',', '[', ']')]
    memory = [0] * 30000
    pointer = 0
    i = 0
    output_str = ''
    jump_table = preprocess(code)
    
    while i < len(code):
        cmd = code[i]
        
        if cmd == '>':
            pointer += 1
        elif cmd == '<':
            pointer -= 1
        elif cmd == '+':
            memory[pointer] = (memory[pointer] + 1) % 256
        elif cmd == '-':
            memory[pointer] = (memory[pointer] - 1) % 256
        elif cmd == '.':
            output_str += chr(memory[pointer])
        elif cmd == ',':
            if len(input_str) > 0:
                input_char = input_str[0]
                input_str = input_str[1:] # TODO: optimize
                memory[pointer] = ord(input_char)
            else: # EOF
                memory[pointer] = 0
        elif cmd == '[':
            if memory[pointer] == 0:
                i = jump_table[i]
        elif cmd == ']':
            if memory[pointer] != 0:
                i = jump_table[i]
                
        i += 1
        
    return output_str

def interpret_interactive(code, flush=True):
    code = [c for c in code if c in ('<', '>', '+', '-', '.', ',', '[', ']')]
    memory = [0] * 30000
    pointer = 0
    i = 0
    jump_table = preprocess(code)
    
    while i < len(code):
        cmd = code[i]
        
        if cmd == '>':
            pointer += 1
        elif cmd == '<':
            pointer -= 1
        elif cmd == '+':
            memory[pointer] = (memory[pointer] + 1) % 256
        elif cmd == '-':
            memory[pointer] = (memory[pointer] - 1) % 256
        elif cmd == '.':
            sys.stdout.write(chr(memory[pointer]))
            if flush:
                sys.stdout.flush()
        elif cmd == ',':
            input_char = sys.stdin.read(1)
            if input_char:
                memory[pointer] = ord(input_char)
            else: # EOF
                memory[pointer] = 0
        elif cmd == '[':
            if memory[pointer] == 0:
                i = jump_table[i]
        elif cmd == ']':
            if memory[pointer] != 0:
                i = jump_table[i]
                
        i += 1

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 bf_interpreter.py [brainfuck_file.bf]")
        sys.exit(1)

    with open(sys.argv[1], 'r') as file:
        code = file.read()

    interpret_interactive(code)
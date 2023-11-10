#!/usr/bin/env python3
from typing import *
import odes

functions: Dict[str, int] = {
    'sin':1,
    'cos':1,
    'tan':1,
    'acos':1,
    'asin':1,
    'atan':1,
    'ddt':1,
    'int':2,
}
operators: Dict[str, Tuple(int, str)] = {
    '=':(0, 'l'),
    '+':(2, 'l'),
    '-':(2, 'l'),
    '*':(3, 'l'),
    '/':(3, 'l'),
    '%':(3, 'l'),
    '^':(4, 'r'),
}

def parse(tokens: List[odes.Token]):
    ouqueue: List[odes.Token] = []
    opstack: List[odes.Token] = []
    for token in tokens:
        if token.tpe == 'string' and token.value in functions:
            opstack.append(token)
        elif token.tpe in ['int', 'float']:
            ouqueue.append(token)
        elif token.tpe == 'string':
            ouqueue.append(token)
        elif token.tpe == 'brace.open':
            opstack.append(token)

    print(ouqueue)
    print(opstack)
    return None

def main():
    tokens = odes.tokenize("t+5 = 3*sin(2*pi*t)+int(t, 15)")
    print(tokens)
    ast = parse(tokens)
    print(ast)

if __name__=='__main__':
    main()
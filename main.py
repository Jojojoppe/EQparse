#!/usr/bin/env python3
from typing import Dict, List, Tuple
import odes

def check(a, b):
    s = ''
    for tok in a:
        s += str(tok) + ' '
    if not s[:-1]==b:
        print('Error: ' + s + ' != ' + b)
    else:
        print('OK')

def main():
    check(odes.parse(odes.tokenize('a + b')), 'a b +')
    check(odes.parse(odes.tokenize('-a + b')), '0 a - b +')
    check(odes.parse(odes.tokenize('a + -b')), 'a 0 b - +')

    check(odes.parse(odes.tokenize('a - b')), 'a b -')
    check(odes.parse(odes.tokenize('-a - b')), '0 a - b -')
    check(odes.parse(odes.tokenize('a - -b')), 'a 0 b - -')
    check(odes.parse(odes.tokenize('-a - -b')), '0 a - 0 b - -')

    check(odes.parse(odes.tokenize('-a * b')), '0 a - b *')
    check(odes.parse(odes.tokenize('a * -b')), 'a 0 b - *')
    check(odes.parse(odes.tokenize('-a * -b')), '0 a - 0 b - *')

    check(odes.parse(odes.tokenize('-a / b')), '0 a - b /')
    check(odes.parse(odes.tokenize('a / -b')), 'a 0 b - /')
    check(odes.parse(odes.tokenize('-a / -b')), '0 a - 0 b - /')

    check(odes.parse(odes.tokenize('-a ^ b')), '0 a - b ^')
    check(odes.parse(odes.tokenize('a ^ -b')), 'a 0 b - ^')
    check(odes.parse(odes.tokenize('-a ^ -b')), '0 a - 0 b - ^')

    check(odes.parse(odes.tokenize('-a % b')), '0 a - b %')
    check(odes.parse(odes.tokenize('a % -b')), 'a 0 b - %')
    check(odes.parse(odes.tokenize('-a % -b')), '0 a - 0 b - %')

    check(odes.parse(odes.tokenize('- ( a + b )')), '0 a b + -')
    check(odes.parse(odes.tokenize('c - ( a + b )')), 'c a b + -')
    check(odes.parse(odes.tokenize('c + - ( a + b )')), 'c 0 a b + - +')
    check(odes.parse(odes.tokenize('c * - ( a + b )')), 'c 0 a b + - *')
    check(odes.parse(odes.tokenize('c / - ( a + b )')), 'c 0 a b + - /')
    check(odes.parse(odes.tokenize('c % - ( a + b )')), 'c 0 a b + - %')
    check(odes.parse(odes.tokenize('c ^ - ( a + b )')), 'c 0 a b + - ^')

    check(odes.parse(odes.tokenize('- sin(a + b)')), '0 a b + sin -')

if __name__=='__main__':
    main()

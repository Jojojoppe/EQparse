''' Simple shunting yard math parser
Can handle unary minus
'''
from typing import Dict, List, Tuple
from .token import Token

# TODO create exception and change asserts to raise calls

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
operators: Dict[str, Tuple[int, str]] = {
    '=':(0, 'l'),
    '+':(7, 'l'),
    '-':(8, 'l'),
    '*':(6, 'l'),
    '/':(6, 'l'),
    '%':(5, 'l'),
    '^':(4, 'r'),
}

class Node():
    def __init__(self, token: Token, tpe: str, value: str):
        self.token = token
        self.tpe = tpe;
        self.value = value
        self.children: List[Node] = []

    def __str__(self):
        return str(self.value)

    def __repr__(self):
        s = ''
        for ch in self.children:
            s += str(ch)
        return self.tpe+'('+str(self)+')[' + s + ']'

def tokenToNodeType(token: Token) -> str:
    if token.tpe in ['int', 'float']:
        return 'number'
    elif token.tpe == 'string' and token.value in functions:
        return 'function'
    elif token.tpe == 'string':
        return 'variable'
    else:
        return 'operator'

# Shunting yard parsing list of tokens to RPN notation
def parseRPN(tokens: List[Token]) -> List[Node]:
    ouqueue: List[Node] = []
    opstack: List[Token] = []
    lastToken: Token = None
    inversed: bool = False
    for token in tokens:
        # Search for unary minus
        if token.tpe == 'operator' and token.value == '-':
            if lastToken is None or lastToken.tpe == 'operator' or (lastToken.tpe == 'string' and lastToken.value in functions):
                inversed = True

        lastToken = token

        # function
        if token.tpe == 'string' and token.value in functions:
            opstack.append(token)
        # number
        elif token.tpe in ['int', 'float']:
            if inversed:
                ouqueue.append(Node(None, 'number', 0))
                inversed = False
            ouqueue.append(Node(token, 'number', token.value))
        # variable/constant (-> number)
        elif token.tpe == 'string':
            if inversed:
                ouqueue.append(Node(None, 'number', 0))
                inversed = False
            ouqueue.append(Node(token, 'variable', token.value))
        # (
        elif token.tpe == 'brace.open':
            opstack.append(token)
        # )
        elif token.tpe == 'brace.close':
            while len(opstack)>0 and opstack[-1].tpe != 'brace.open':
                assert len(opstack)>0, 'Unmatched brace found'
                ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
                opstack = opstack[:-1]
            assert len(opstack)==0 or opstack[-1].tpe == 'brace.open', 'Something went wrong...'
            opstack = opstack[:-1]
            if len(opstack)>0 and opstack[-1].tpe == 'function':
                ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
                opstack = opstack[:-1]
        # opeartor
        elif token.tpe == 'operator':
            assert token.value in operators, 'Unknown operator found'
            # Check for double unary minus
            if token.value=='-' and len(opstack)>0 and opstack[-1].tpe == 'operator' and opstack[-1].value == '-' and inversed:
                ouqueue.append(Node(None, 'number', 0))
                opstack.append(token)
                inversed = False
                continue
            o1 = operators[token.value]
            while len(opstack)>0 and opstack[-1].tpe != 'brace.open':
                if opstack[-1].tpe == 'string' and opstack[-1].value in functions:
                    ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
                    opstack = opstack[:-1]
                else:
                    o2 = operators[opstack[-1].value]
                    if o2[0] > o1[0] or ( o2[0] == o1[0] and o1[1] == 'l'):
                        ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
                        opstack = opstack[:-1]
                    else:
                        break
            opstack.append(token)
        # ,
        elif token.tpe == 'punctuation':
            while len(opstack)>0 and opstack[-1].tpe != 'brace.open':
                ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
                opstack = opstack[:-1]

    while len(opstack)>0:
        assert opstack[-1].tpe != 'brace.open', 'Unmatched brace found'
        ouqueue.append(Node(opstack[-1], tokenToNodeType(opstack[-1]), opstack[-1].value))
        opstack = opstack[:-1]

    return ouqueue

def parse(tokens: List[Token]) -> List[Node]:
    # TODO reverse and create AST
    return parseRPN(tokens)

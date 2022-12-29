from enum import Enum
from .parser import parse
from .ast import AstNode
from .tokenizer import TokenType
from .data import *

class Equation:
    def __init__(self, equation:str):
        self.equation = equation
        self.ast = parse(equation)
        # Check if equation or statement
        if self.ast.tokenType != TokenType.EQUAL:
            raise Exception("Root node of parsed equation not an '=', given string is a statement, not an equation")

    def __repr__(self):
        return str(self.ast)

    def genPNG(self, output:str):
        print("generating PNG from", self)
        self.ast.genPNG(output)

    def simplify(self):
        self.ast.simplify()

from enum import Enum
from .tokenizer import TokenType
import pydot

class AstNode:
    tokenType:TokenType = TokenType.NONE
    position:int = 0
    value = None
    children = []
    
    def __init__(self, tokenType:TokenType, position:int, value):
        self.tokenType = tokenType
        self.position = position
        self.value = value

    def __iter__(self):
        return self.children.__iter__()

    def __getitem__(self, item):
        return self.children[item]

    def __setitem__(self, key, value):
        self.children[key] = value

    def __repr__(self):
        if self.tokenType in [TokenType.VALUE, TokenType.VARIABLE]:
            return str(self.value)
        elif self.tokenType == TokenType.CONSTANT:
            return str(self.value[0])
        elif self.tokenType in [TokenType.OPERATOR, TokenType.FUNCTION, TokenType.EQUAL]:
            ch = [str(n) for n in self]
            return f"{self.value}({','.join(ch)})"
        else:
            return str(self.tokenType)

    def isNumber(self):
        return self.tokenType in [TokenType.VALUE, TokenType.CONSTANT]

    def number(self):
        if self.isNumber():
            if self.tokenType==TokenType.CONSTANT:
                return self.value[1]
            else:
                return self.value
        else:
            return None

    def depth(self):
        if len(self.children)==0:
            return 1
        chdepths = [n.depth() for n in self.children]
        return max(chdepths)+1

    def isOperator(self):
        return self.tokenType==TokenType.OPERATOR

    def isFunction(self):
        return self.tokenType==TokenType.FUNCTION

    def genPNG(self, output:str):
        graph = pydot.Dot('ast', graph_type='digraph')
        graph.add_node(pydot.Node("root"))
        
        def visit(node:AstNode, path:str="rt", i:int=0):
            if node.tokenType in [TokenType.VALUE, TokenType.VARIABLE]:
                label = str(node.value)
            elif node.tokenType == TokenType.CONSTANT:
                label = str(node.value[0])
            elif node.tokenType in [TokenType.OPERATOR, TokenType.FUNCTION, TokenType.EQUAL]:
                label = str(node.value)
            else:
                label = str(node.tokenType)
            graph.add_node(pydot.Node(f'{path}{i}', label=label))
            graph.add_edge(pydot.Edge(path, f'{path}{i}'))
            for j, ch in enumerate(node):
                visit(ch, f'{path}{i}', j)

        visit(self, "root")

        graph.write_png(output)

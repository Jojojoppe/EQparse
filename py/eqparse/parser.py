from enum import Enum
from .tokenizer import Tokenizer, Token, TokenType
from .ast import AstNode
from .data import *

def parse(string:str):
    oprstack = []
    expstack = []
    hasequal = False

    def getAstNode(tok:Token):
        value = tok.string
        if tok.tokenType==TokenType.VALUE:
            value = tok.value
        elif tok.tokenType==TokenType.CONSTANT:
            value = (tok.string, tok.value)
        return AstNode(tok.tokenType, tok.position, value)

    def pushOperationToExpression(op:Token):
        if len(expstack)<2:
            raise Exception(f"Unfinished expression\n{string}\n{(op.position-1)*' '}^")
        e2 = expstack.pop()
        e1 = expstack.pop()
        n = op
        n.children = [e1, e2]
        expstack.append(n)

    def pushFunctionToExpression(fun:Token):
        if len(expstack)<functions[fun.value].arguments:
            raise Exception(f"Unfinished expression for function\n{string}\n{(op.position-1)*' '}^")
        args = []
        for i in range(functions[fun.value].arguments):
            args.append(expstack.pop())
        args.reverse()
        n = op
        n.children = args
        expstack.append(n)

    tokens = Tokenizer(string)
    for tok in tokens:

        if tok.tokenType==TokenType.NONE:
            raise Exception("Unknown token type at position {tok.position}\n{string}\n{(tok.position-1)*' '}^")

        elif tok.tokenType==TokenType.VALUE:
            expstack.append(getAstNode(tok))

        elif tok.tokenType==TokenType.BROPEN:
            oprstack.append(getAstNode(tok))

        elif tok.tokenType==TokenType.BRCLOSE:
            while len(oprstack)>0 and oprstack[-1].tokenType!=TokenType.BROPEN:
                op = oprstack.pop()
                if op.tokenType in [TokenType.OPERATOR, TokenType.EQUAL]:
                    pushOperationToExpression(op)
                elif op.tokenType==TokenType.FUNCTION:
                    pushFunctionToExpression(op)
            if len(oprstack)==0:
                raise Exception(f"Unmatched )\n{string}\n{(tok.position-1)*' '}^")
            oprstack.pop()

        elif tok.tokenType==TokenType.OPERATOR or tok.tokenType==TokenType.EQUAL:
            while len(oprstack)>0 and (((oprstack[-1].tokenType==TokenType.EQUAL or oprstack[-1].tokenType==TokenType.OPERATOR) and operators[tok.string].precedence <= operators[oprstack[-1].value].precedence) or oprstack[-1].tokenType==TokenType.FUNCTION):
                op = oprstack.pop()
                if op.tokenType in [TokenType.OPERATOR, TokenType.EQUAL]:
                    pushOperationToExpression(op)
                elif op.tokenType==TokenType.FUNCTION:
                    pushFunctionToExpression(op)
            if tok.tokenType==TokenType.EQUAL:
                if hasequal:
                    raise Exception(f"Second equal sign found\n{string}\n{(tok.position-1)*' '}^")
                hasequal = True
            oprstack.append(getAstNode(tok))

        elif tok.tokenType==TokenType._STRING:
            if tok.string in constants:
                tok.tokenType = TokenType.CONSTANT
                tok.value = constants[tok.string].value
                expstack.append(getAstNode(tok))
            elif tok.string in functions:
                tok.tokenType = TokenType.FUNCTION
                oprstack.append(getAstNode(tok))
            else:
                tok.tokenType = TokenType.VARIABLE
                expstack.append(getAstNode(tok))

    while len(oprstack):
        op = oprstack.pop()
        if op.tokenType in [TokenType.BROPEN, TokenType.BRCLOSE]:
            raise Exception("Unmatched bracked")
        elif op.tokenType in [TokenType.OPERATOR, TokenType.EQUAL]:
            pushOperationToExpression(op)
        elif op.tokenType==TokenType.FUNCTION:
            pushFunctionToExpression(op)

    if len(expstack)!=1 and len(oprstack)==0:
        print(expstack)
        print(oprstack)
        raise Exception("Parse error, not ended with one expression")

    return expstack.pop()

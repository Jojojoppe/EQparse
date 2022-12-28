from enum import Enum

class TokenType(Enum):
    NONE = 0
    VALUE = 1
    VARIABLE = 2
    FUNCTION = 3
    OPERATOR = 4
    CONSTANT = 5
    EQUAL = 6
    BROPEN = 7
    BRCLOSE = 8
    COMMA = 9
    EOF = 10
    _STRING = 11

class Token:
    tokenType:TokenType = TokenType.NONE
    string:str = ""
    value:float = float('nan')
    position:int = 0

    def __init__(self, tokenType:TokenType, position:int, string:str="", value:float=float('nan')):
        self.tokenType = tokenType
        self.position = position
        self.string = string
        self.value = value

    def __repr__(self):
        return f"{str(self.tokenType)}[{self.value}]({self.string})"

class Tokenizer:
    string:str = ""
    position:int = 0
    prevtoken:Token = Token(TokenType.NONE, 0)

    def __init__(self, string:str):
        self.string = string
        self.position = 0

    def __iter__(self):
        return self

    def __next__(self):
        tok = self._nextToken()
        self.prevtoken = tok
        if tok.tokenType == TokenType.EOF:
            raise StopIteration
        return tok

    def _nextToken(self)->Token:
        if self.position==len(self.string):
            return Token(TokenType.EOF, self.position)

        buf = []
        number_decimal = False
        number_scientific = False

        class State:
            IDLE = 0
            NUMBER = 1
            STRING = 2
        state = State.IDLE

        while True:
            if self.position==len(self.string):
                if len(buf)==0:
                    return Token(TokenType.EOF, self.position)
                else:
                    if state==State.NUMBER:
                        return Token(TokenType.VALUE, self.position, ''.join(buf), float(''.join(buf)))
                    elif state==State.STRING:
                        return Token(TokenType._STRING, self.position, ''.join(buf))
                    else:
                        raise Exception(f"Unknown tokenizer state {stata}")

            c = self.string[self.position]
            self.position += 1

            if state==State.IDLE:
                # Check for start number
                if '0'<=c<='9' or c=='.':
                    if c=='.':
                        number_decimal = True
                    buf.append(c)
                    state = State.NUMBER
                    continue
                # Check for start of string
                elif 'a'<=c<='z' or 'A'<=c<='Z' or c=='_':
                    buf.append(c)
                    state = State.STRING
                    continue
                # Check for operators (ignore minus for a while)
                elif c in ['+', '*', '/', '^', '%']:
                    return Token(TokenType.OPERATOR, self.position, c)
                # Check for brackets
                elif c=='(':
                    return Token(TokenType.BROPEN, self.position)
                elif c==')':
                    return Token(TokenType.BRCLOSE, self.position)
                # Check for other characters
                elif c=='=':
                    return Token(TokenType.EQUAL, self.position, c)
                elif c==',':
                    return Token(TokenType.COMMA, self.position)
                # Ignore whitespaces
                elif c in [' ', '\t', '\n']:
                    self.prevtok = Token(TokenType.NONE, self.position)
                    continue

                # Now try to fit minus
                elif c=='-':
                    if self.prevtoken.tokenType in [TokenType.BRCLOSE, TokenType.FUNCTION, TokenType.VALUE, TokenType._STRING, TokenType.NONE, TokenType.VARIABLE, TokenType.CONSTANT]:
                        return Token(TokenType.OPERATOR, self.position, c)
                    else:
                        buf.append(c)
                        state = State.NUMBER
                        continue

                raise Exception(f"Unknown character {c} at position {self.position}\n{self.string}\n{(self.position-1)*' '}^")

            elif state==State.NUMBER:
                if '0'<=c<='9':
                    buf.append(c)
                elif c=='.' and not number_decimal:
                    buf.append(c)
                    number_decimal = True
                elif (c=='e' or c=='E') and not number_scientific:
                    buf.append('e')
                    number_scientific = True
                elif buf[-1]=='e' and c=='-':
                    buf.append(c)
                else:
                    # No number anymore
                    self.position -= 1
                    return Token(TokenType.VALUE, self.position, ''.join(buf), float(''.join(buf)))

            elif state==State.STRING:
                if 'a'<=c<='z' or 'A'<=c<='Z' or c=='_':
                    buf.append(c)
                else:
                    # No string anymore
                    self.position -= 1
                    return Token(TokenType._STRING, self.position, ''.join(buf))

            else:
                raise Exception(f"Unknown tokenizer state {stata}")
                



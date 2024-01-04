from typing import *

class TokenizerException(Exception):
    def __init__(self, message, errors):
        super().__init__(message)
        self.errors = errors

class Token():
    def __init__(self, value: str, tpe: str):
        self.value: str = value
        self.tpe: str = tpe

    def __str__(self):
        return str(self.value)

    def __repr__(self):
        return self.tpe+'('+str(self)+')'

def tokenize(input: str) -> List[Token]:
    tokens: List[Token] = []

    index: int = 0
    tokenType: str = ''
    tokenValue: str = ''
    prevChType: str = ''
    decorators: List[str] = []
    while index < len(input):
        ch: str = input[index]
        chType: str = ''
        # Check if number
        if ord(ch) >= ord('0') and ord(ch) <= ord('9'):
            chType = 'number'
        elif ch in ['+', '/', '*', '%', '^', '=']:
            chType = 'operator'
        elif ch == '-':
            chType = 'operator.minus'
        elif ch in [',', '.']:
            chType = 'punctuation'
        elif ch in ['(', ')']:
            chType = 'braces'
        elif ch == 'f':
            chType = 'letter.float'
        elif ch == 'e':
            chType = 'letter.scientific'
        elif ch in [' ', '\t', '\r', '\n']:
            chType = 'whitespace'
        elif ch.isalnum() or ch in ['_']:
            chType = 'letter'
        else:
            chType = ''

        # print(ch, chType, tokenType, tokenValue, decorators)

        # start of an int
        if tokenType == '' and chType == 'number':
            tokenType = 'int'
            tokenValue = ch
        # continuation of an int
        elif tokenType == 'int' and chType == 'number':
            tokenValue += ch
        # punctuation in an int (-> float)
        elif tokenType == 'int' and chType == 'punctuation':
            tokenValue += '.'  # implicit conversion to .
            tokenType = 'float'
            decorators.append('punctuation')
        # punctuation in float
        elif tokenType == 'float' and chType == 'punctuation':
            if 'punctuation' in decorators:
                raise TokenizerException(
                    'double punctuation not allowed', (index, ch))
            else:
                tokenValue += '.'  # implicit conversion to .
                decorators.append('punctuation')
        # continuation of a float
        elif tokenType == 'float' and chType == 'number':
            tokenValue += ch
        # scientific identifier
        elif tokenType in ['int', 'float'] and chType == 'letter.scientific':
            if 'scientific' in decorators:
                raise TokenizerException(
                    f'double scientific identifier not allowed at [{index}, {ch}]', (index, ch))
            else:
                tokenValue += ch
                tokenType = 'float'
                decorators.append('scientific')
        # minus in scientific exponent
        elif tokenType == 'float' and prevChType == 'letter.scientific' and chType == 'operator.minus':
            tokenValue += ch
        # float identifier [EOT]
        elif tokenType in ['int', 'float'] and chType == 'letter.float':
            if 'float' in decorators:
                raise TokenizerException(
                    f'double scientific identifier not allowed at [{index}, {ch}]', (index, ch))
            else:
                tokenType = 'float'
                decorators.append('float')
                # EOT
                tokens.append(Token(tokenValue, tokenType))
                tokenValue = ''
                tokenType = ''
                decorators.clear()
        # whitespace after int or float [EOT]
        elif tokenType in ['int', 'float'] and chType == 'whitespace':
            # EOT
            tokens.append(Token(tokenValue, tokenType))
            tokenValue = ''
            tokenType = ''
            decorators.clear()
        # operator
        elif chType.startswith('operator'):
            # EOT
            if (tokenType != ''):
                tokens.append(Token(tokenValue, tokenType))
            # Emit operator token
            tokens.append(Token(ch, 'operator'))
            tokenValue = ''
            tokenType = ''
            decorators.clear()
        # whitespace after nothing
        elif tokenType == '' and chType == 'whitespace':
            pass
        # whitespace after int or float
        elif tokenType in ['int', 'float'] and chType == 'whitespace':
            # EOT
            tokens.append(Token(tokenValue, tokenType))
            tokenValue = ''
            tokenType = ''
            decorators.clear()
        # Braces
        elif chType == 'braces':
            if ch == '(':
                # EOT
                if (tokenType != ''):
                    tokens.append(Token(tokenValue, tokenType))
                # Emit operator token
                tokens.append(Token(ch, 'brace.open'))
            else:
                # EOT
                if (tokenType != ''):
                    tokens.append(Token(tokenValue, tokenType))
                # Emit operator token
                tokens.append(Token(ch, 'brace.close'))
            tokenValue = ''
            tokenType = ''
            decorators.clear()
        # Letters or number after string
        elif chType.startswith('letter') or (chType == 'number' and tokenType == 'string'):
            if tokenType != 'string' and tokenType != '':
                tokens.append(Token(tokenValue, tokenType))
                tokenValue = ''
                tokenType = ''
                decorators.clear()
            tokenType = 'string'
            tokenValue += ch
        # Punctuation outside numbers as commas
        elif chType == 'punctuation':
            if (tokenType != ''):
                tokens.append(Token(tokenValue, tokenType))
            # Emit operator token
            tokens.append(Token(',', 'punctuation'))
            tokenValue = ''
            tokenType = ''
            decorators.clear()

        # ignore whitespace and emit token if needed
        elif chType == 'whitespace':
            if tokenType != 'string' and tokenType != '':
                tokens.append(Token(tokenValue, tokenType))
                tokenValue = ''
                tokenType = ''
                decorators.clear()
        # Anything else is not allowed
        else:
            raise TokenizerException(
                f'Unknown or unexpected character at [{index}, {ch}]', (index, ch))

        index += 1
        prevChType = chType
    # Last token if there is
    if tokenType != '':
        tokens.append(Token(tokenValue, tokenType))

    return tokens


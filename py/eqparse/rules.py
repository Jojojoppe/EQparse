from .tokenizer import TokenType
from enum import Enum

class RuleAction(Enum):
    NONE = 0
    DEPTH = 1
    NOVALUE = 2
    NOONE = 3

class Rule:
    token = TokenType.NONE
    value = None
    important = None
    error = None
    action = RuleAction.NONE
    def __init__(self, children:tuple, important=None):
        self.children = children
        self.important = important

class ERROR(Rule):
    def __init__(self, msg):
        Rule.__init__(self, tuple())
        self.error = msg

class DNC(Rule):
    def __init__(self, important=None):
        Rule.__init__(self, tuple(), important)

class DNC_NV(Rule):
    action = RuleAction.NOVALUE
    def __init__(self, important=None):
        Rule.__init__(self, tuple(), important)

class DNC_D(Rule):
    value = 1
    action = RuleAction.DEPTH
    def __init__(self, depth, important=None):
        Rule.__init__(self, tuple(), important)
        self.value = depth

class OP(Rule):
    token = TokenType.OPERATOR
    value = None
    def __init__(self, important=None):
        Rule.__init__(self, tuple(), important)

class ADD(Rule):
    token = TokenType.OPERATOR
    value = '+'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)
class SUB(Rule):
    token = TokenType.OPERATOR
    value = '-'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)
class MUL(Rule):
    token = TokenType.OPERATOR
    value = '*'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)
class DIV(Rule):
    token = TokenType.OPERATOR
    value = '/'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)
class POW(Rule):
    token = TokenType.OPERATOR
    value = '^'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)
class MOD(Rule):
    token = TokenType.OPERATOR
    value = '%'
    def __init__(self, children, important=None):
        Rule.__init__(self, children, important)

class VALUE(Rule):
    token = TokenType.VALUE
    value = None
    def __init__(self, important=None):
        Rule.__init__(self, tuple(), important)
class VALUE_NO(Rule):
    token = TokenType.VALUE
    value = None
    action = RuleAction.NOONE
    def __init__(self, important=None):
        Rule.__init__(self, tuple(), important)
class VALUE_V(Rule):
    token = TokenType.VALUE
    value = None
    def __init__(self, value, important=None):
        self.value = float(value)
        Rule.__init__(self, tuple(), important)

class FUN(Rule):
    token = TokenType.FUNCTION
    value = None
    def __init__(self, name, children=tuple(), important=None):
        Rule.__init__(self, children, important)
        self.value = str(name)

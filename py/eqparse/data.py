import math

class FunctionData:
    def __init__(self, arguments:int):
        self.arguments = arguments
functions = {
    "sin":FunctionData(1),
    "cos":FunctionData(1),
    "tan":FunctionData(1),
    "asin":FunctionData(1),
    "acos":FunctionData(1),
    "atan":FunctionData(1),
    "ddt":FunctionData(1),
    "int":FunctionData(2),
    "sqrt":FunctionData(1),
    "log":FunctionData(1),
    "ln":FunctionData(1),
}

class OperatorData:
    def __init__(self, precedence:int, commutative:bool):
        self.precedence = precedence
        self.commutative = commutative
operators = {
    "+":OperatorData(2, True),
    "-":OperatorData(2, True),
    "*":OperatorData(3, True),
    "/":OperatorData(3, False),
    "^":OperatorData(4, False),
    "%":OperatorData(1, False),
    "=":OperatorData(0, True),
}

class ConstantData:
    def __init__(self, value:float):
        self.value = value
constants = {
    "pi":ConstantData(math.pi),
    "e":ConstantData(math.e),
}

from .rules import *
rules = [

    # Zero rules
    # ----------
    # a+0 -> a
    (ADD((DNC(1), VALUE_V(0))),
     DNC(1)),
    # a*0 -> 0
    (MUL((DNC(), VALUE_V(0))),
     VALUE_V(0)),
    # a^0 -> 1
    (POW((DNC(), VALUE_V(0))),
     VALUE_V(1)),
    # 0^a -> 0
    (POW((VALUE_V(0), DNC())),
     VALUE_V(0)),
    # 0/a -> 0
    (DIV((VALUE_V(0), DNC())),
     VALUE_V(0)),
    # a/0 -> ERROR
    (DIV((DNC(), VALUE_V(0))),
     ERROR("Division by zero")),

    # Unity rules
    # -----------
    # a*1 -> a
    (MUL((DNC(1), VALUE_V(1))),
    DNC(1)),
    # a+a -> 2*a
    (ADD((DNC(1), DNC(1))),
     MUL((VALUE_V(2), DNC(1)))),
    # a*a -> a^2
    (MUL((DNC(1), DNC(1))),
    POW((DNC(1), VALUE_V(2)))),
    # a/a -> 1
    (DIV((DNC(1), DNC(1))),
     VALUE_V(1)),
    # a^1 -> a
    (POW((DNC(1), VALUE_V(1))),
     DNC(1)),

    # Basic inverse to normal rules
    # -----------------------------
    # a-b -> a+(b*-1)
    (SUB((DNC(1), DNC(2))),
     ADD((DNC(1), MUL((DNC(2), VALUE_V(-1)))))),
    # a/b -> a*(1/b)
    # (DIV((DNC_NV(1), DNC(2))),
     # MUL((DNC(1), DIV((VALUE_V(1), DNC(2)))))),

    # Tree rules
    # ----------
    # (a+Nb)+Nc -> a+(Nb+Nc)
    (ADD((ADD((DNC(1), VALUE(2))), VALUE(3))),
     ADD((DNC(1), ADD((VALUE(2), VALUE(3)))))),
    # (a*Nb)*Nc -> a*(Nb*Nc)
    (MUL((MUL((DNC(1), VALUE(2))), VALUE(3))),
     MUL((DNC(1), MUL((VALUE(2), VALUE(3)))))),
    # (a+b)+b -> a+(2*b)
    (ADD((ADD((DNC(1), DNC(2))), DNC(2))),
     ADD((DNC(1), MUL((VALUE_V(2), DNC(2)))))),
    # (a*b)+(a*c) -> (b+c)*a
    (ADD((MUL((DNC(1), DNC(2))), MUL((DNC(1), DNC(3))))),
    MUL((ADD((DNC(2), DNC(3))), DNC(1)))),
    # (a+b)+(b*c) -> a+(b*(c+1))
    (ADD((ADD((DNC(1), DNC(2))), MUL((DNC(2), DNC(3))))),
     ADD((DNC(1), MUL((DNC(2), ADD((DNC(3), VALUE_V(1)))))))),
    # (a+b)*Nc -> (a*Nc)+(b*Nc) -> sum of products
    (MUL((ADD((DNC(1), DNC(2))), VALUE(3))),
     ADD((MUL((DNC(1), VALUE(3))), MUL((DNC(2), VALUE(3)))))),
    # (a+(b*Nc))+(b*Nd) -> a+(b*(Nc+Nd))
    (ADD((ADD((DNC(1), MUL((DNC(2), VALUE(3))))), MUL((DNC(2), VALUE(4))))),
     ADD((DNC(1), MUL((DNC(2), ADD((VALUE(3), VALUE(4)))))))),
    # ((b*Nc)+a)+(b*Nd) -> a+(b*(Nc+Nd))
    (ADD((ADD((MUL((DNC(2), VALUE(3))), DNC(1))), MUL((DNC(2), VALUE(4))))),
     ADD((DNC(1), MUL((DNC(2), ADD((VALUE(3), VALUE(4)))))))),
    # (a*Nb)+a -> a*(Nb+1)
    (ADD((MUL((DNC(1), VALUE(2))), DNC(1))),
     MUL((DNC(1), ADD((VALUE(2), VALUE_V(1)))))),

    # Division rules
    # --------------
    # (a*Nb)/Nc -> a*(Nb/Nc)
    (DIV((MUL((DNC(1), VALUE(2))), VALUE(3))),
     MUL((DNC(1), DIV((VALUE(2), VALUE(3)))))),
    # (a*Nb)/(c*Nd) -> (a*(Nb/Nd))/c
    (DIV((MUL((DNC(1), VALUE(2))), MUL((DNC(3), VALUE(4))))),
     DIV((MUL((DNC(1), DIV((VALUE(2), VALUE(4))))), DNC(3)))),
    # a/(b*Nc) -> (a/b)*(1/Nc)
    (DIV((DNC(1), MUL((DNC(2), VALUE(3))))),
     MUL((DIV((DNC(1), DNC(2))), DIV((VALUE_V(1), VALUE(3)))))),
    # (a*Nb)/c -> (a/c)*Nb
    (DIV((MUL((DNC(1), VALUE(2))), DNC(3))),
     MUL((DIV((DNC(1), DNC(3))), VALUE(2)))),
    # (a+Nb)/C -> (a/C)+(Nb/C)
    (DIV((ADD((DNC(1), VALUE(2))), DNC_NV(3))),
     ADD((DIV((DNC(1), DNC_NV(3))), DIV((VALUE(2), DNC_NV(3)))))),

    # Function rules
    # --------------
    # ddt(int(a, b)) -> a
    (FUN('ddt', (FUN('int', (DNC(1), DNC())),)),
     DNC(1)),
    # int(ddt(a), b) -> a+b
    (FUN('int', (DNC(1), FUN('ddt', (DNC(2),)))),
    ADD((DNC(1), DNC(2)))),
]

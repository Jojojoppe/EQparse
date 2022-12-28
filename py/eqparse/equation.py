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
        self.ast.genPNG(output)

    def simplify(self):
        steps = 100
        while steps:
            print('.', end='')
            changes = 0
            changes += self._normalize()
            changes += self._apply_rules()
            self._eval_numeric()
            steps -= 1
            if changes==0 and steps>1:
                steps = 1
        print()

    def _eval_numeric(self):
        def visit(node)->int:
            changes = 0
            if node.isOperator() and node[0].isNumber() and node[1].isNumber():
                v1 = node[0].number()
                v2 = node[1].number()
                node.tokenType = TokenType.VALUE
                node.children = []
                if node.value=='+':
                    node.value = v1+v2
                elif node.value=='-':
                    node.value = v1-v2
                elif node.value=='*':
                    node.value = v1*v2
                elif node.value=='/':
                    node.value = v1/v2
                elif node.value=='^':
                    node.value = v1^v2
                elif node.value=='%':
                    node.value = v1%v2
                else:
                    raise Exception(f"Unknown operator type {node.value}")
                node.value = float(node.value)
                changes += 1
            for ch in node:
                changes += visit(ch)
            return changes
        while visit(self.ast):
            pass

    def _normalize(self):
        def visit(node)->int:
            changes = 0
            for ch in node:
                changes += visit(ch)
            if node.isOperator() and operators[node.value].commutative:
                d1 = node[0].depth()
                d2 = node[1].depth()
                # Move tails to the left (only for commutative operators)
                if d1<d2:
                    node[0], node[1] = node[1], node[0]
                    changes += 1
                # Move the unknown to the right if the same and one is a number
                elif d1==1 and d2==1 and node[1].isNumber():
                    node[0], node[1] = node[1], node[0]
                    changes += 1
            return changes
        return visit(self.ast)

    def _apply_rules(self):
        def equivalent(node, rule)->bool:
            # DNC
            if rule.token==TokenType.NONE:
                return True
            # If not the same type no match
            if rule.token!=node.tokenType:
                return False
            # If value check for equivalence
            if rule.token==TokenType.VALUE and rule.value is not None and rule.value!=node.value:
                return False
            # If OPERATOR check for equivalence
            if rule.token==TokenType.OPERATOR and rule.value!=node.value:
                return False

            # Check in children
            # length of children must be the same
            if len(node.children)!=len(rule.children):
                return False
            for ch, r in zip(node, rule.children):
                if not equivalent(ch, r):
                    return False
            return True

        def applyOnNode(node)->tuple:
            # Depth first
            changes = 0
            newch = []
            for ch in node:
                change, n = applyOnNode(ch)
                newch.append(n)
                changes += change
            node.children = newch

            # Check if rule matches on current node
            for rulenr, rule in enumerate(rules):
                fr = rule[0]
                to = rule[1]
                if equivalent(node, fr):

                    def tryout(node, rf, to):
                        # Find interested terms
                        interested = {}
                        def getInterested(node, rule)->bool:
                            if rule.important:
                                if rule.important in interested:
                                    # Already in interested list, check for equivalence, if not error...
                                    if str(node)!=str(interested[rule.important]):
                                        return False
                                else:
                                    interested[rule.important] = node
                                return True
                            for ch, r in zip(node, rule.children):
                                if not getInterested(ch, r):
                                    return False
                            return True
                        if not getInterested(node, fr):
                            # Did not match after all
                            return changes, node
                        
                        # Apply rule
                        def apply(node, rule):
                            # Check for error
                            if rule.error is not None:
                                raise Exception(rule.error)
                            if rule.important:
                                if rule.important not in interested:
                                    raise Exception("Rule error: important term not in list")
                                node = interested[rule.important]
                            else:
                                node = AstNode(rule.token, 0, rule.value)
                                node.children = []
                                for r in rule.children:
                                    n = AstNode(TokenType.NONE, 0, None)
                                    n = apply(n, r)
                                    node.children.append(n)
                            return node
                        node = apply(AstNode(TokenType.NONE, node.position, None), to)

                        return (changes + 1, node)

                    ch, n = tryout(node, fr, to)
                    if ch:
                        changes += ch
                        return changes, n

            return changes, node

        ch, self.ast = applyOnNode(self.ast)
        return ch

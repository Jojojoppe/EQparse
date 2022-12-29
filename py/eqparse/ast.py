from enum import Enum
from .tokenizer import TokenType
from .data import *
import pydot

# Bubble sort over AstNode array
def bubbleSort(arr):
    n = len(arr)
 
    # Traverse through all array elements
    for i in range(n):
 
        # Last i elements are already in place
        for j in range(0, n-i-1):
 
            # traverse the array from 0 to n-i-1
            # Swap if the element found is greater
            # than the next element
            if str(arr[j]) > str(arr[j+1]):
            # if str(arr[j]) < str(arr[j+1]):
                arr[j], arr[j+1] = arr[j+1], arr[j]

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

    def genPNG(self, output:str, rootlabel="root"):
        graph = pydot.Dot('ast', graph_type='digraph')
        graph.add_node(pydot.Node("root", label=rootlabel))
        
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

    def simplify(self):
        steps = 500
        while steps:
            changes = 0
            while self._normalize():
                changes += 1
            self._eval_numeric()
            while self._apply_rules():
                changes += 1
            self._eval_numeric()
            steps -= 1
            if changes==0 and steps>1:
                steps = 1
        while self._normalize():
            pass
        print()

    def _eval_numeric(self):
        print('E', end='')
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
                    node.value = math.pow(v1, v2)
                elif node.value=='%':
                    node.value = v1%v2
                else:
                    raise Exception(f"Unknown operator type {node.value}")
                node.value = float(node.value)
                changes += 1
            for ch in node:
                changes += visit(ch)
            return changes
        while visit(self):
            pass

    def _normalize(self):
        print('N', end='')
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
                    print('a')
                # Move numbers to the right
                elif d1==1 and d2==1 and node[0].isNumber() and not node[1].isNumber():
                    node[0], node[1] = node[1], node[0]
                    changes += 1
                    print('b')
                # Sort bottom leaves
                elif d1==1 and d2==1 and not node[0].isNumber() and not node[1].isNumber() and not node[0].value==node[1].value:
                    tosort = [node[0], node[1]]
                    bubbleSort(tosort)
                    tosort.reverse()
                    if node[0]!=tosort[0]:
                        node[0], node[1] = tosort[0], tosort[1]
                        changes += 1
                        print('c')
                # Check to see if left child is same operator -> commutative tree
                # elif node[0].isOperator() and node[0].value == node.value and not node[1].isNumber() and not node[0][1].isNumber() and node[1].depth()==1 and node[0][1].depth()==1:
                elif node[0].isOperator() and node[0].value == node.value and not node[1].isNumber() and not node[0][1].isNumber():
                    tosort = [node[1], node[0][1]]
                    bubbleSort(tosort)
                    if node[1]!=tosort[0]:
                        node[1], node[0][1] = tosort[0], tosort[1]
                        changes += 1
                        print('d')
                # Move numbers up
                elif node[0].isOperator() and node[0].value == node.value and not node[1].isNumber() and node[0][1].isNumber():
                    node[1], node[0][1] = node[0][1], node[1]
                    changes += 1
                    print('e')
                # Move trees down
                elif node[0].isOperator() and node[0].value == node.value and node[1].depth()>1 and node[0][1].depth()==1:
                    node[1], node[0][1] = node[0][1], node[1]
                    changes += 1
                    print('f')

            return changes
        return visit(self)

    def _apply_rules(self):
        print('R', end='')
        def equivalent(node, rule)->bool:
            # DNC
            if rule.token==TokenType.NONE:
                # Check if DEPTH action
                if rule.action==RuleAction.DEPTH:
                    if node.depth()==rule.value:
                        return True
                    else:
                        return False
                elif rule.action==RuleAction.NOVALUE:
                    if node.tokenType==TokenType.VALUE:
                        return False
                elif rule.action==RuleAction.NOONE:
                    if node.value==1.0:
                        return False
                return True
            # If not the same type no match
            if rule.token!=node.tokenType:
                return False
            # If value check for equivalence
            if rule.token==TokenType.VALUE and rule.value is not None and rule.value!=node.value:
                return False
            # If OPERATOR check for equivalence
            if rule.token==TokenType.OPERATOR:
                if rule.value is None:
                    return True
                if rule.value!=node.value:
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
                        mustsort = []
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

        ch, self = applyOnNode(self)
        return ch

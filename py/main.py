#!/usr/bin/env python
import eqparse

eq = eqparse.Equation("y=5*x+x+12+0.3*x")
eq.simplify()
print(eq)
eq.genPNG('out.png')

#!/usr/bin/env python
import eqparse

eq = eqparse.Equation("y=(2*x*y*z+3)/(y*z)+10*y+y")
eq.simplify()
eq.genPNG('out.png')

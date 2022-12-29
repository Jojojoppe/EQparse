#!/usr/bin/env python
import eqparse

eq = eqparse.Equation("y=2*(y+x+z) + 3*(x+y) + 5*(x+z) + 2*y + z")
eq.simplify()
eq.genPNG('out.png')

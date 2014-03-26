# Demo curve to be called from within an AMPL model.
from math import sin, cos

debug = False  # True

# Spiral.
def curvex(t):
    if debug: print 'In curvex with t=', t
    sint = sin(t) ; cost = cos(t) ; tsint = t*sint ; tcost = t*cost
    return (tsint, sint+tcost, 2*cost-tsint)

def curvey(t):
    if debug: print 'In curvey with t=', t
    sint = sin(t) ; cost = cos(t) ; tsint = t*sint ; tcost = t*cost
    return (tcost, cost-tsint, -2*sint-tcost)


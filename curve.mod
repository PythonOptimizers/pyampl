# This version maximizes the interdistance.

model;

function curvex;            # Defined in 'amplfunc' Python module.
function curvey;            # Defined in 'amplfunc' Python module.

param pi;
param N = 25;               # Number of points on curve.
param q;                    # Some multiplicative parameter.
var t {0..N+1} >= 0, <= 1;  # Parameters.

# Curve.
var x {i in 0..N+1} = curvex(q*t[i]);  # Call Python module!
var y {i in 0..N+1} = curvey(q*t[i]);

var d {i in 0..N} = (x[i+1] - x[i])^2 + (y[i+1] - y[i])^2;

maximize interdistance: d[0];

subject to monotonicity {i in 0..N}: t[i] <= t[i+1];
subject to inital_point: t[0]   = 0;
subject to final_point:  t[N+1] = 1;

subject to equidistance {i in 1..N}:
    d[i] = d[0];

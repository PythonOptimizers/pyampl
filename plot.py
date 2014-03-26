import numpy as np
from amplfunc import curvex, curvey
import matplotlib.pyplot as plt

# Read solution output by solver.
x = np.loadtxt("x.dat")
y = np.loadtxt("y.dat")

# Finely discretized curve.
T = np.linspace(0, 4*np.pi, 200)
X = T * np.sin(T)
Y = T * np.cos(T)

plt.plot(X, Y, 'r-', x, y, 'bo')
plt.show()
#plt.savefig("spiral.jpg")

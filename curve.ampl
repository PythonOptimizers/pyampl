function py_initialize;  # Function to initialize the Python interpreter.
function py_finalize;    # Function to finalize the Python interpreter.

# Initialize the Python interpreter. The return value is unimportant.
param err_initialize := py_initialize();

# Define model, data and solve.
model curve.mod;
data curve.dat;
option solver snopt;  # Minos doesn't quite find a solution.
solve;

# Write final solution to file.
printf {i in 0..N+1} '%15.7e\n', x[i] > x.dat;
printf {i in 0..N+1} '%15.7e\n', y[i] > y.dat;

# Finalize the Python interpreter. The return value is unimportant.
param err_finalize := py_finalize();

# PyAMPL

A simple bridge between AMPL and Python allowing users to define external
functions in Python and use those functions inside AMPL models.

This example bridge is used to define a parametric curve in Python, to be used
in the AMPL model. It should generalize easily to other cases.


## Compilation notes on Mac Intel

    brew install python --universal
    brew tap homebrew/science
    brew install asl
    make amplfunc.dll
    pip install numpy, scipy  # For the example.


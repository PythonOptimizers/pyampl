/* Experimental bridge between AMPL and Python allowing users to define
 * extension functions in Python and call them seamlessly from AMPL.
 *
 * dominique.orban@gerad.ca, August 2011.
 */

#include <Python.h>
#include "asl/funcadd.h"

// Defines.
// Each function that will use LOG() or DBG() will need to declare
// AmplExports *ae = al->AE;
#ifdef LOGGING
#define LOG(...) {fprintf(Stderr,__VA_ARGS__); fflush(Stderr);}
#else
#define LOG(...) // as nothing.
#endif

#ifdef DEBUG
#define DBG(...) {fprintf(Stderr,__VA_ARGS__); fflush(Stderr);}
#else
#define DBG(...) // as nothing.
#endif

// Prototypes.
static int  initialize_interpreter(arglist *al);
static int  finalize_interpreter(arglist *al);
static real curvex_func(arglist *al);  // First  user-defined function.
static real curvey_func(arglist *al);  // Second user-defined function.
static real curve_func(arglist *al, int which);
void funcadd(AmplExports *ae);

// Global variables.
static int py_initialized = 0;
static PyObject *pModule=NULL, *pName=NULL;
static PyObject *p_curvex_Func=NULL, *p_curvey_Func=NULL;

// Initialize the Python interpreter.
static int initialize_interpreter(arglist *al) {

    AmplExports *ae = al->AE;         // For [fs]printf(), TempMem().
    int err = 1;

    LOG("In initialize_interpreter()\n");
    if (! py_initialized) {
        Py_Initialize();
        if (Py_IsInitialized()) {
            py_initialized = 1;
            err = 0;
        } else {
            al->Errmsg = (char*)TempMem(al->TMI, 64);
            sprintf(al->Errmsg, "Unable to initialize interpreter");
            return err;
        }
    }
    LOG("py_initialized=%d\n", py_initialized);

    pName = PyString_FromString("amplfunc");
    LOG("Importing module amplfunc\n");
    pModule = PyImport_Import(pName); // Import module.
    Py_DECREF(pName);

    if (pModule == NULL) {

        DBG("Oops! Got pModule=%p\n", pModule);
        PyRun_SimpleString("import traceback; traceback.print_stack()");
        al->Errmsg = (char*)TempMem(al->TMI, 64);
        sprintf(al->Errmsg, "Unable to import Python module (arg=%8.1e)", al->ra[0]);
        finalize_interpreter(al);
        return err;
    }

    // Grab reference to relevant functions inside module.
    LOG("Grabbing reference to curvex\n");
    p_curvex_Func = PyObject_GetAttrString(pModule, "curvex");
    if (!(p_curvex_Func && PyCallable_Check(p_curvex_Func))) {
        al->Errmsg = (char*)TempMem(al->TMI, 64);
        sprintf(al->Errmsg,
                "Unable to import function curvex from module (arg=%8.1e)", al->ra[0]);
        return err;
    }

    LOG("Grabbing reference to curvey\n");
    p_curvey_Func = PyObject_GetAttrString(pModule, "curvey");
    if (!(p_curvey_Func && PyCallable_Check(p_curvey_Func))) {
        al->Errmsg = (char*)TempMem(al->TMI, 64);
        sprintf(al->Errmsg,
                "Unable to import function curvey from module (arg=%8.1e)", al->ra[0]);
        return err;
    }

    return 0;
}

// Finalize the Python interpreter.
static int finalize_interpreter(arglist *al) {
    if (py_initialized) {
        Py_Finalize();
        py_initialized = 0;
    }
    Py_DECREF(pModule);
    Py_DECREF(p_curvex_Func);
    Py_DECREF(p_curvey_Func);
    pModule=NULL;
    pName=NULL;
    p_curvex_Func=NULL;
    p_curvey_Func=NULL;
    return 0;
}

// Call curvex().
static real curvex_func(arglist *al) {
    return curve_func(al,0);
}

// Call curvey().
static real curvey_func(arglist *al) {
    return curve_func(al,1);
}

// Call curvex() (which=0) or curvey() (which=1).
static real curve_func(arglist *al, int which) {

    PyObject *pArgs=NULL, *pValue=NULL;
    AmplExports *ae = al->AE;         // For [fs]printf(), TempMem().
    real rv=0, d1=0, d2=0;
    int err;

    LOG("Entering curve_func() with py_initialized=%d\n", py_initialized);
    if (!py_initialized) {
        err = initialize_interpreter(al);
        if (err) return err;
    }

    // Convert input argument.
    pArgs = PyTuple_New(1);
    pValue = PyFloat_FromDouble(al->ra[0]);
    if (!pValue) {
        Py_DECREF(pArgs);
        al->Errmsg = (char*)TempMem(al->TMI, 64);
        sprintf(al->Errmsg, "Unable to convert argument (arg=%8.1e)", al->ra[0]);
        finalize_interpreter(al);
    }
    PyTuple_SetItem(pArgs, 0, pValue);

    LOG("Calling Python function...\n");
    if (which == 0)
        pValue = PyObject_CallObject(p_curvex_Func, pArgs);
    else
        pValue = PyObject_CallObject(p_curvey_Func, pArgs);
    LOG("Returning from Python function\n");
    Py_DECREF(pArgs);

    if (pValue != NULL) {

        LOG("Parsing return arguments\n");
        if (!PyArg_ParseTuple(pValue, "ddd", &rv, &d1, &d2)) {

            al->Errmsg = (char*)TempMem(al->TMI, 64);
            sprintf(al->Errmsg, "Unable to parse return values (arg=%8.1e)", al->ra[0]);
            finalize_interpreter(al);

        } else {

            if (al->derivs)
                al->derivs[0] = d1;
            if (al->hes)
                al->hes[0] = d2;
        }
        Py_DECREF(pValue);

    } else {

        DBG("Oops! Return value is NULL...\n");
        PyRun_SimpleString("import traceback; traceback.print_stack()");
        al->Errmsg = (char*)TempMem(al->TMI, 64);
        sprintf(al->Errmsg, "Call to Python function failed (arg=%8.1e)", al->ra[0]);
        finalize_interpreter(al);
    }

    return rv;
}


void funcadd(AmplExports *ae) {

    /* Arg 3, called type, must satisfy 0 <= type <= 6:
    | type&1 == 0: 0,2,4,6 ==> force all arguments to be numeric.
    | type&1 == 1: 1,3,5   ==> pass both symbolic and numeric arguments.
    | type&6 == 0: 0,1 ==> the function is real valued.
    | type&6 == 2: 2,3 ==> the function is char * valued; static storage
    |               suffices: AMPL copies the return value.
    | type&6 == 4: 4,5 ==> the function is random (real valued).
    | type&6 == 6: 6   ==> random, real valued, pass nargs real args,
    |              0 <= nargs <= 2.
    |
    | Arg 4, called nargs, is interpretted as follows:
    |  >=  0 ==> the function has exactly nargs arguments
    |  <= -1 ==> the function has >= -(nargs+1) arguments.
    |
    | Arg 5, called funcinfo, is copied without change to the arglist
    |  structure passed to the function; funcinfo is for the
    |  function to use or ignore as it sees fit.
    */

    // Declare real-valued functions with a single numeric arg.
    addfunc("curvex", (rfunc)curvex_func, 0, 1, 0);
    addfunc("curvey", (rfunc)curvey_func, 0, 1, 0);

    // Initialize/Finalize functions take no arguments for now.
    addfunc("py_initialize", (rfunc)initialize_interpreter, 0, 0, 0);
    addfunc("py_finalize",   (rfunc)finalize_interpreter,   0, 0, 0);
}

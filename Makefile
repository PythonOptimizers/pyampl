# For Macintosh under OS X
# See http://docs.python.org/extending/embedding.html#linking-requirements

CC ?= clang
ARCH = -arch i386    # The AMPL engine doesn't recognize x86_64.
DEFS =
ifdef DEBUG
DEFS = -DDEBUG -DLOGGING
endif

# It shouldn't be necessary to modify anything below.

.SUFFIXES: .c .o

PY_INC = $(shell python -c "import distutils; print distutils.sysconfig.get_python_inc()")
PY_LIB = $(shell python -c "import distutils; print distutils.sysconfig.PREFIX")/lib
PY_LDFLAGS = $(shell python -c "import distutils; print distutils.sysconfig.get_config_var('LINKFORSHARED')")
ASL_INC = $(shell brew --prefix asl)/include
ASL_LIB = $(shell brew --prefix asl)/lib

CFLAGS = -I$(PY_INC) -I$(ASL_INC) -O2 $(ARCH) $(DEFS)

all: viewconfig amplfunc.dll

viewconfig:
	@echo PY_INC: $(PY_INC)
	@echo PY_LIB: $(PY_LIB)
	@echo PY_LDFLAGS: $(PY_LDFLAGS)
	@echo ASL_INC: $(ASL_INC)
	@echo ASL_LIB: $(ASL_LIB)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

# The extension library must be named "amplfunc.dll" on all systems.
amplfunc.dll: funcadd.o
	$(CC) -bundle $(ARCH) -o amplfunc.dll -L$(ASL_LIB) -lasl -L$(PY_LIB) -lpython2.7 $<

clean:
	rm -f funcadd.o

mrclean: clean
	rm -f amplfunc.dll


# 3Delgiht or other render's base directory
RENDERMANDIR = ${DELIGHT}

# Include and library directories
INC_DIRS = -I${RENDERMANDIR}/include
LIB_DIRS = -L${RENDERMANDIR}/lib/

# additional libraries
LIBS = -l3delight -lm -ldl -lc

camplace: main.c Makefile
	clang -Wall -g -o $@ $< ${INC_DIRS} ${LIB_DIRS} ${LIBS}

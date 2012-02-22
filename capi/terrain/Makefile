
# 3Delgiht or other render's base directory
RENDERMANDIR = ${DELIGHT}

# Include and library directories
INC_DIRS = -I${RENDERMANDIR}/include
LIB_DIRS = -L${RENDERMANDIR}/lib/

# additional libraries
LIBS = -l3delight -lm -ldl -lc

SRC_FILES = main.c trimesh.c

terrain: $(SRC_FILES) Makefile
	clang -Wall -g $(SRC_FILES) -o terrain  ${INC_DIRS} ${LIB_DIRS} ${LIBS}

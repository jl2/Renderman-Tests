
# 3Delgiht or other render's base directory
RENDERMANDIR = ${DELIGHT}

# Include and library directories
INC_DIRS = -I${RENDERMANDIR}/include
LIB_DIRS = -L${RENDERMANDIR}/lib/

# additional libraries
LIBS = -l3delight -lm -ldl -lc -lavformat -lavcodec -lavutil -lfftw3


sndanim: sndanim.c Makefile
	clang -g -o sndanim sndanim.c ${INC_DIRS} ${LIB_DIRS} ${LIBS}

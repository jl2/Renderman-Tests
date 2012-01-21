
# CFLAGS = -I /usr/local/include/aqsis/
# CXXFLAGS = ${CFLAGS}
# LDFLAGS = -laqsis -L /usr/local/lib/

RENDERMANDIR = ${DELIGHT}

main: csg.cpp Makefile
	g++ -g -o main csg.cpp -I${RENDERMANDIR}/include -L${RENDERMANDIR}/lib/ -l3delight -lm -ldl -lc

animation: main
	./main testit

# main: main.o -lm -ldl -lc

# readpgm: readpgm.o -lm -ldl -lc

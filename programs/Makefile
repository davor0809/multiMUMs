# ATTENTION: please enter under SDSL the path to the directory in which the sdsl-library is stored
SDSL = ~/sdsl
CC = g++
CFLAGS = -std=c++11 -O3 -DNDEBUG -I $(SDSL)/include -L $(SDSL)/lib
LIBS = -lsdsl -ldivsufsort -ldivsufsort64

comparer: comparer.cpp
	$(CC) $(CFLAGS) $< simpleMultiMUM.cpp streamMultiMUM.cpp suffixArrayMultiMUM.cpp simpleMultiMUMParallel.cpp streamMultiMUMParallel.cpp suffixArrayMultiMUMParallel.cpp -o $@ $(LIBS)

%: %.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
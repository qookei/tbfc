OBJS = tbfc.o
PROGRAM = tbfc

CXX ?= g++

CXXFLAGS = -std=c++17 -O3

$(PROGRAM): $(OBJS)
	g++ -o $@ $^ $(CXXFLAGS)

%.o: %.cc
	g++ -c -o $@ $< $(CXXFLAGS)

.PHONY: clean

clean:
	rm $(OBJS)
	rm $(PROGRAM)

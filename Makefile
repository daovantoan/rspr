CC=gcc           ### icc
CXX=g++          ### icpc
LDFLAGS=-lm -O2  ### -mmic  for Intel Xeon Phi

all: rspr calnum simuo simup simur simseq genMT

rspr: rspr.c genMRIP.c data.c TGenMT.c
	$(CC) $^ -o $@ $(LDFLAGS)

calnum: calnum.c genMRIP.c data.c runSim.c TGenMT.c
	$(CC) $^ -o $@ $(LDFLAGS)

genMT: mainMt.c TGenMT.c
	$(CC) $^ -o $@

simuo: simuo.cpp TGenMT.cpp simuPo.cpp
	$(CXX) $^ -o $@ $(LDFLAGS) -fopenmp

simseq: simuseq.cpp TGenMT.cpp simuPo.cpp
	$(CXX) $^ -o $@ $(LDFLAGS)

simup: simup.cpp TGenMT.cpp simuPo.cpp
	$(CXX) $^ -o $@ $(LDFLAGS) -lpthread

simur: simur.cpp TGenMT.cpp simuPo.cpp
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o simuo simup simur simseq rspr calnum genMT

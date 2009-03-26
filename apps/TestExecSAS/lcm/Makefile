SRC = ThreadSpawn

CXX = g++

% :
	$(CXX) $(INCLUDES) -fPIC $@.cc -c
	ar cru liblcmUtils.a $@.o

all: $(SRC)
	$(CXX) -fPIC -shared -o liblcmUtils.so *.o
	ranlib liblcmUtils.a
	(cd structs; make clean; make all)

default: all


clean:
	(cd structs; make clean)
	rm *.so *.a *.o
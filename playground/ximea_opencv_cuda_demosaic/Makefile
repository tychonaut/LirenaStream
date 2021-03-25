CXX=g++
SOURCES=ocvDemosaic.cpp
OBJECTS=$(SOURCES:.cpp=.o)
PROGRAM=ocvDemosaic


all: $(PROGRAM)

$(PROGRAM): ocvDemosaic.o 
	g++ $(OBJECTS) -O3 -o ocvDemosaic -lm3api -lopencv_core -lopencv_highgui -lopencv_cudaimgproc -lopencv_cudaarithm -lcudart -L/usr/local/lib -L/usr/local/cuda/lib64 

ocvDemosaic.o: $(SOURCES)
	g++ -c $(SOURCES) -O3 -I . -I /usr/local/include/ -I /usr/local/cuda/include -std=c++11 -g3 -Wall -c -fmessage-length=0 

clean:
	rm -f *.o *~ $(PROGRAM) $(OBJECTS)







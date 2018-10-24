CXXFLAGS=-std=c++11 -O3 -Wall -fmessage-length=0 -g 

OBJS=main.o Simulation.o ExtendedDartAdaptationManager.o SASSAdaptationManager.o ../../../build/src/libadaptmgr.a
LDLIBS=-lyaml-cpp -lboost_system -lboost_filesystem -lboost_serialization -ldartam -ladaptmgr 
TARGET=dartfitness
CPPFLAGS=-I../dartam/include -I../../../include -I../
LDFLAGS=-L../dartam/build/src/.libs -L../../../build/src -L/usr/lib/x86_64-linux-gnu -g -Wl,-rpath-link=../../../build/src

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	 rm -f $(OBJS) $(TARGET)

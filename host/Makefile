CXXFLAGS=-Wall
OBJECTS=flaschen-taschen.o

all : send-image opc-server

send-image: send-image.o flaschen-taschen.o
	$(CXX) -o $@ $^

opc-server: opc-server.cc flaschen-taschen.o
	$(CXX) -o $@ $^

clean :
	rm -f send-image opc-server

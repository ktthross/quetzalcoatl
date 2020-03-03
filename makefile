CPP = g++
CFLAGS = -std=c++11 -Wall -g -O0
#HFLAGS = -I ~/working/Eigen -I  ~/working/Eigen/unsupported -I /usr/local/lib64
#LFLAGS = -lgsl -lgslcblas -lm
#LFLAGS = -lgsl -lgslcblas -lm
LDFLAGS = -L /home/xiuhtecuhtli/working/jsoncpp/build/debug/src/lib_json -ljsoncpp
OBJ = main.o qtzio.o

#quetzalcoatl : $(OBJ) 
#	$(CPP) $(CFLAGS) $(HFLAGS) -o quetzalcoatl $(OBJ) $(LFLAGS)
#

quetzalcoatl : $(OBJ) 
	$(CPP) -o quetzalcoatl $(OBJ) $(LDFLAGS)

all: $(OBJ)

main.o : %.o: %.cpp
	$(CPP) $(CFLAGS) -L/home/xiuhtecuhtli/working/jsoncpp/build/debug/lib -ljsoncpp -I/home/xiuhtecuhtli/working/jsoncpp/include/json -c $< -o $@ $(LFLAGS)

qtzio.o : qtzio.cpp
	$(CPP) $(CFLAGS) $(LDFLAGS) -I/home/xiuhtecuhtli/working/jsoncpp/include/json -I$(BOOST_ROOT) -c $< -o $@

#$(OBJ) : %.o: %.cpp
#	$(CPP) $(CFLAGS) $(HFLAGS) -c $< -o $@ $(LFLAGS)


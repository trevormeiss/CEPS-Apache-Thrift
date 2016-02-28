GEN_SRC = Administrator.cpp Player.cpp ceps_constants.cpp ceps_types.cpp ParseExpression.cpp
GEN_OBJ = $(patsubst %.cpp,%.o, $(GEN_SRC))

THRIFT_DIR = /usr/local/include/thrift
BOOST_DIR = /usr/local/include/boost

INC = -I$(THRIFT_DIR) -I$(BOOST_DIR)

.PHONY: all clean

all: CEPS_Admin CEPS_Player

%.o: %.cpp
	$(CXX) -std=c++11 -Wall $(INC) -c $< -o $@

CEPS_Admin: CEPS_Admin.o $(GEN_OBJ)
	$(CXX) -std=c++11 $^ -o $@ -L/usr/local/lib -lthrift

CEPS_Player: CEPS_Player.o $(GEN_OBJ)
	$(CXX) -std=c++11 $^ -o $@ -L/usr/local/lib -lthrift

clean:
	$(RM) *.o CEPS_Admin CEPS_Player
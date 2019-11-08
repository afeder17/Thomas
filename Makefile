#Makefile for Thomas

CXX      = clang++ -O3 -std=c++11
CXXFLAGS = -g -Wall -Wextra
LDFLAGS  = -g -pthread

HDRS = AI.h AI_r.h board.h
SRCS = checkers.cpp AI.cpp AI_r.cpp board.cpp
OBJS = checkers.o AI.o board.o AI_r.o

Thomas:  ${OBJS} ${HDRS}
	${CXX} ${LDFLAGS} -o Thomas ${OBJS}

clean:
	rm -rf Thomas ${OBJS} *~ *.dSYM

checkers.o: checkers.cpp AI.h AI_r.h board.h
AI.o: AI.cpp board.h
AI_r.o: AI_r.cpp board.h
board.o: board.cpp

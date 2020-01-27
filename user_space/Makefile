PROG = pi_scale
CC = g++
CPPFLAGS = -g -Wall
LDFLAGS = -lpthread
OBJS = scale_main.o Scale.o

$(PROG) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)
scale_main.o :
	$(CC) $(CPPFLAGS) -c scale_main.cpp
Scale.o : include/Scale.h
	$(CC) $(CPPFLAGS) -c include/Scale.cpp
clean:
	rm -f core $(PROG) $(OBJS)
CC = g++
EXEC = ctrl

all: Ardcom.o string_manip.o stdutil.o tls_ctrl.cpp
	$(CC) -o $(EXEC) tls_ctrl.cpp Ardcom.o string_manip.o stdutil.o

Ardcom.o: Ardcom.cpp
	$(CC) -c Ardcom.cpp

string_manip.o: string_manip.cpp
	$(CC) -c string_manip.cpp

stdutil.o: stdutil.cpp
	$(CC) -c stdutil.cpp

clean:
	rm *.o
	rm $(EXEC)

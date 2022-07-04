CFLAGS :=  -Wall -Wextra -Wpedantic  
EXE := server
LIBS := -lpthread
all: $(EXE)

server: utils.o tcp.o $(LIBS)


utils.o: utils.h
tcp.o: utils.h tcp.h


clean:
	$(RM) $(EXE) *~ *.o

.PHONY: all clean
CC = gcc
CFLAGS = -Wall -Wextra #-std=c99
LDFLAGS =

EXE = nep_pac
OBJECTS = nep_pac.o

all : $(OBJECTS)
	$(CC) $(CFLAGS)  $(OBJECTS) -o $(EXE) $(LDFLAGS) 

nep_pac.o : nep_pac.c
	$(CC) $(CFLAGS) -c nep_pac.c $(LDFLAGS) 
  
clean: 
	rm -f $(EXE) $(OBJECTS)

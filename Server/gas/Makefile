CC=gcc
INCPATH=-I ../Include
LIBPATH=-L../Lib -L/usr/local/lib
CFLAGS=-Wall -g $(INCPATH) 
LIBS=-lssl -lmysqlclient -lgendo -liconv -levent

SRCS=gas-main.c gas-user.c gas-user-msg.c
OBJS=gas-main.o gas-user.o gas-user-msg.o

PROGNAME=gas

all: $(PROGNAME)

$(PROGNAME): $(OBJS)
	$(CC) -o $(PROGNAME) $(OBJS) $(LIBPATH) $(LIBS)

clean:
	-rm -f *.o $(PROGNAME) 


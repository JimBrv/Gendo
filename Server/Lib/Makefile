CC=gcc

INCPATH=../Include -I/usr/local/include
LIBPATH=-L/usr/local/lib
CFLAGS= -g -Wall --shared -I$(INCPATH) -D_GNU_SOURCE -levent -liconv -lpthread -lmysqlclient
LIBS=-levent -liconv -lpthread

MAKE_DIR=$(PWD)

SRCS :=$(wildcard *.c)
SRC_FILES:=$(notdir $(SRCS))
OBJ_FILES :=$(patsubst %c,%o,$(SRC_FILES))

LIBNAME=libgendo.a

$(LIBNAME): $(OBJ_FILES)
	@echo -e "build $(LIBNAME)"
	$(AR) cr $(LIBNAME) $(OBJ_FILES) 

all: $(LIBNAME)

clean:
	-rm -f *.o $(LIBNAME) *~	

.PHONY:clean

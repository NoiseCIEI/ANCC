#
INST_DIR = ../../bin
BIN  = add_num

cflags = -O -Wall 
# cflags = -O2

LDLIBS =   -lm

CFLAGS = $(DBG) $(cflags) -I../../include

CC = gcc

DBG =

COBJS = add_num.o

$(BIN) : $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(COBJS)

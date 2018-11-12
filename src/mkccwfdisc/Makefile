#
INST_DIR = ../../bin
BIN  =  mkccwfdisc

cflags = -O -Wall 
# cflags = -O2


CFLAGS = $(DBG) $(cflags) -I../../include

CC = gcc

DBG =

COBJS = mkccwfdisc.o etime.o

$(BIN) : $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(BIN) $(LDLIBS) -lm

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(COBJS)

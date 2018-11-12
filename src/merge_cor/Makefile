#
INST_DIR = ../../bin
BIN  =  merge_cor

cflags = -O -Wall 
# cflags = -O2


CFLAGS = $(DBG) $(cflags) -I../../include

CC = gcc

DBG =

COBJS = merge_cor.o

$(BIN) : $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(COBJS)

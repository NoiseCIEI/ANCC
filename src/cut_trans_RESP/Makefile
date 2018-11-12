#
INST_DIR = ../../bin
BIN  =  cut_trans_RESP

cflags = -O -Wall 
# cflags = -O2

# LDLIBS =   -lfftw3

CFLAGS = $(DBG) $(cflags) -I../../include

CC = gcc

DBG =

COBJS = cut_trans_RESP.o

$(BIN) : $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(COBJS)

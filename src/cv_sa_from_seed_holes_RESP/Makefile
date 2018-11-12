#
INST_DIR = ../../bin
BIN  = cv_sa_from_seed_holes_RESP

cflags = -O -Wall 

LDLIBS =   -lm

CFLAGS = $(DBG) $(cflags) -I../../include

CC = gcc

DBG =

COBJS = cv_sa_from_seed_holes_RESP.o etime.o

$(BIN) : $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(COBJS)

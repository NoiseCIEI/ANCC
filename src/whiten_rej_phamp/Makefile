#
INST_DIR = ../../bin
BIN  = whiten_rej_phamp

fflags =  -O -Wall -ffixed-line-length-none
cflags =  -O -Wall

LDLIBS = -lfftw3

FFLAGS = $(DBG) $(fflags)

CFLAGS = $(DBG) $(cflags) -I../../include

FC = g77

#CC = gcc

DBG = -g 

FOBJS = driver_c.o whiten_rej_phamp.o swapn.o

$(BIN) : $(FOBJS)
	$(FC) $(FFLAGS) $(CFLAGS) $(FOBJS) -o $(BIN) $(LDLIBS)

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(FOBJS)

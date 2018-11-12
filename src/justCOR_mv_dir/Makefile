#
INST_DIR = ../../bin

BIN  = justCOR_mv_dir


fflags = -O -Wall -ffixed-line-length-none
cflags = -O -Wall
# if you don't use shared libraries comment the next line
#shlib = -R/opt/SUNWspro/lib

FFLAGS = $(DBG)  $(fflags) $(shlib)

CFLAGS = $(DBG) $(cflags) -I../../include

LDLIBS = -lfftw3

FC = g77

DBG =

FOBJS = dmultifft.o justCOR_mv_dir.o

$(BIN) : $(FOBJS)
	$(FC) $(FFLAGS) $(CFLAGS) $(FOBJS) -o $(BIN) $(LDLIBS) -lm

install :: $(BIN)
	install  -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(FOBJS)

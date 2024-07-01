CC	= nvc
CFLAGS	= -O3
EXE	= $(firstword $(MAKECMDGOALS))
OBJS	= $(EXE).o

ifeq ($(openacc), 1)
        CFLAGS += -acc=gpu -gpu=cc70 -Minfo=all
endif
ifeq ($(openacc_debug), 1)
        CFLAGS += -acc=gpu -gpu=cc70 -Minfo=all -g
endif
ifeq ($(openmp), 1)
        CFLAGS += -mp
endif
ifeq ($(mpi), 1)
        CC = mpicc
endif

.SUFFIXES: .o .c

.c.o:
	$(CC) -c $(CFLAGS) $<

$(EXE):$(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)
	rm -f $(OBJS) *.mod

FC	= nvfortran
FFLAGS	= -O3 -cpp -Minline
EXE	= $(firstword $(MAKECMDGOALS))
OBJS	= $(EXE).o

ifeq ($(openacc), 1)
        FFLAGS += -acc=gpu -gpu=cc70 -Minfo=all
endif
ifeq ($(openacc_debug), 1)
        FFLAGS += -acc=gpu -gpu=cc70 -Minfo=all -g
endif
ifeq ($(openmp), 1)
        FFLAGS += -mp
endif
ifeq ($(mpi), 1)
        FC = mpifort
endif

.SUFFIXES: .o .f90

.f90.o:
	$(FC) -c $(FFLAGS) $<

$(EXE):$(OBJS)
	$(FC) $(FFLAGS) -o $(EXE) $(OBJS)
	rm -f $(OBJS) *.mod

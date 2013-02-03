# Makefile for Pilot tutorial C and Fortran labs
# HPCVL global desktop; change to your local Pilot installation
PILOTHOME = /work/wgardner/pilot

CC = mpicc -intel
CPPFLAGS = -I$(PILOTHOME)/include
LDFLAGS = -L$(PILOTHOME)/lib -lpilot \
    -L/opt/sharcnet/r/2.10.0/lib64/R/lib \
    -L/opt/sharcnet/openmpi/1.4.2/intel/lib \
    -L/opt/mx/lib64 \
    -L/opt/sharcnet/intel/11.0.083/icc/lib/intel64 \
    -L/opt/sharcnet/intel/11.0.083/ifc/lib/intel64 \
    -L/opt/sharcnet/intel/11.0.083/ifc/mkl/lib/em64t \
    -L/opt/sharcnet/fftw/2.1.5/intel/lib \
    -L/opt/sharcnet/openmpi/current/intel/lib
    
FC = mpif90 -intel
FFLAGS = -fpp -I$(PILOTHOME)/include
# LDFLAGS same as above

cannon: cannon.o
	$(CC) $< $(LDFLAGS) -o $@

clean:
	rm *.o cannon

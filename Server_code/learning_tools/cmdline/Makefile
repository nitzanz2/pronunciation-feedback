
# headers
HDRS=cmd_option.h cmd_line.h

# sources
SRCS=cmd_line.cpp tester.cpp

CC = g++
MPICC = mpic++
# CFLAGS  = -Wall -O3 -arch x86_64 -arch i386
CFLAGS  = -Wall -O3 
LFLAGS  = -lcmdline

%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $<

%.om: %.cpp $(HDRS)
	$(MPICC) $(CFLAGS_MPI) -c $< -o $@

# all: tester libcmdline.a libcmdline_mpi.a
all: tester libcmdline.a

tester: tester.o libcmdline.a 

libcmdline.a: cmd_line.o
	ar rv $@ $?
	ranlib $@

libcmdline_mpi.a: cmd_line.om
	ar rv $@ $?
	ranlib $@

clean:
	rm -f *.o *.om tester libcmdline.a lincmdline_mpi.a

doc:    $(HDRS)
	mkdir html; kdoc -f html -d html -n cmd_line $(HDRS)

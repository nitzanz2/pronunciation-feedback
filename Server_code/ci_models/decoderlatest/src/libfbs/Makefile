#
# Makefile
# 
# HISTORY
# 
# 23-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
# 		Created.
#


include $(S3ROOT)../../Makefile.defines
VPATH = .:..


TARGET = s3decode

OBJS =	agc.o \
	cmn.o \
	gauden.o \
	senone.o \
	interp.o \
	tmat.o \
	logs3.o \
	vector.o \
	dict.o \
	mdef.o \
	lm.o \
	fillpen.o \
	mllr.o \
	fwd.o \
	main.o

CFLAGS = $(S3DEBUG) $(S2HMMTOPO) ${$(MACHINE)_CFLAGS}

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -L$(LIBDIR) -o $(TARGET) $(OBJS) -lutil -lm -lfeat -lio

install: $(TARGET)
	- mv $(BINDIR)/$(TARGET) $(BINDIR)/$(TARGET).old
	cp $(TARGET) $(BINDIR)
	rm -f $(BINDIR)/$(TARGET).old

logs3: logs3.c
	$(CC) -g -D_LOGS3_TEST_=1 $(CFLAGS) -L$(LIBDIR) -o logs3 logs3.c -lutil -lm

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

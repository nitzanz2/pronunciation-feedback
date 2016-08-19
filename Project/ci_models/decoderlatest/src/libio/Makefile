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
LOCALINCLUDE = -I$(S3ROOT)/src/libio

TARGET = libio.a

OBJS =	bio.o

CFLAGS = $(S3DEBUG) ${$(MACHINE)_CFLAGS} $(LOCALINCLUDE)

$(TARGET): $(OBJS)
	ar crv $@ $?
	ranlib $@

install: $(TARGET)
	mv $(TARGET) $(LIBDIR)

clean:
	rm -f *.o *.a *.BAK *.CKP .*.BAK .*.CKP *~ .*~ #*#

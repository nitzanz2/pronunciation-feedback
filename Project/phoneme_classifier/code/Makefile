

ifeq (,$(filter _%,$(notdir $(CURDIR))))
  include target.mk
else
#----- End Boilerplate

# This section is for the architecture-specific target directory
VPATH =  $(SRCDIR)

# Additional headers and libraries directories
INFRA_PATH = ../../../learning_tools/infra2
LEARNING_PATH = ../../../learning_tools

CC = g++
CXXFLAGS = -I$(INFRA_PATH) -I$(LEARNING_PATH)
LDLIBS = -L$(INFRA_PATH) -L$(LEARNING_PATH)/cmdline

# Check if the configuration is Release or Debug
ifeq ($(CONFIGURATION),Debug)
	CXXFLAGS += -g
	LDLIBS += -g -linfra_debug -lcmdline
else
	CXXFLAGS += -O3 -DNDEBUG
	LDLIBS += -O3 -linfra -lcmdline
	CONFIGURATION = Release
endif

ifeq ($(ATLAS),yes)
  	LDLIBS += -lcblas -latlas
	CXXFLAGS += -D_USE_ATLAS_
endif

# Targets

PROGRAMS = PhonemeFrameBasedTrain PhonemeFrameBasedDecode RandomShuffleDb

all: $(PROGRAMS)

PhonemeFrameBasedTrain: PhonemeFrameBasedTrain.o Classifier.o HtkFile.o Dataset.o \
KernelExpansion.o ConcateFeatures.o
PhonemeFrameBasedDecode: PhonemeFrameBasedDecode.o Classifier.o Dataset.o HtkFile.o \
KernelExpansion.o ConcateFeatures.o
RandomShuffleDb: RandomShuffleDb.o HtkFile.o Dataset.o ConcateFeatures.o

install: all
	if test "$(BINDIR)" = "" ; then \
		echo "BINDIR not set. Cannot install executables"; \
		exit 1; \
		fi
	for prog in $(PROGRAMS); do \
		install $(PWD)/_$(_ARCH)_$(CONFIGURATION)/$$prog $(BINDIR); \
		done

#----- Begin Boilerplate
endif

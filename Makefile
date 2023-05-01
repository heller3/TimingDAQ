CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)
INC = $(shell pwd)

LDFLAGS := $(shell root-config --glibs)
CPPFLAGS := $(shell root-config --cflags) -I$(INC)/include
#CPPFLAGS += -g -std=c++14
CPPFLAGS += -g
#CPPFLAGS += -g -fsanitize=address -Wall -Wextra -Wno-sign-compare

ifeq ($(shell uname), Darwin)
	CPPFLAGS += -rpath $(shell root-config --prefix)/lib
endif

TARGETS = VMEDat2Root DRSDat2Root DRSclDat2Root NetScopeDat2Root NetScopeStandaloneDat2Root ETL_ASIC_Dat2Root DT5742Dat2Root
SRC = src/Configuration.cc src/Interpolator.cc src/DatAnalyzer.cc 


$(info $$CXX is  [${CXX}])
$(info $$LD is  [${LD}])
$(info $$INC is  [${INC}])

all : $(TARGETS)

$(TARGETS) : %Dat2Root : $(SRC:.cc=.o) src/%Analyzer.o app/%Dat2Root.cc
	@echo Building $@
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.cc
	@echo $@
	$(CXX) $(CPPFLAGS) -o $@ -c $<
clean :
	rm -rf *.o app/*.o src/*.o $(TARGETS) *~ *.dSYM

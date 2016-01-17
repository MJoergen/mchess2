sources  = main.cc
sources += CBoard.cc
sources += CMove.cc
sources += ai.cc
sources += CMoveList.cc
sources += CPerft.cc
sources += CHashEntry.cc
sources += CHashTable.cc
sources += CPerftSuite.cc
sources += CSearchSuite.cc

program = mchess

version = 1-02-00 # UCI-version - Engine-version - Bugfixes

relname = $(program)-$(version)

#TARGET = linux
TARGET = windows

objects = $(sources:.cc=.o)
depends = $(sources:.cc=.d)

OPTIONS  = -Wextra -Wall -Weffc++ -Wpedantic -Wno-long-long
OPTIONS  += -Wswitch-default
OPTIONS  += -O3
OPTIONS  += -DNAME="$(relname)"

#OPTIONS  += -Og
#OPTIONS  += -g  # debug info
OPTIONS  += -DENABLE_TRACE  
#OPTIONS  += -pg # profiling   # Only needed for performance tuning.
#OPTIONS  += -DDEBUG_HASH      # Only needed for debugging.

ifeq ($(TARGET),linux)
  CC = g++
else
  CC = i686-w64-mingw32-g++
  OPTIONS  += -static-libgcc -static-libstdc++
  program := $(program).exe
endif

$(program): $(objects) Makefile
	$(CC) -o $@ $(objects) $(OPTIONS)
	cp $@ $(HOME)/bin

# Automatically generate dependency files.
%.d: %.cc Makefile
	set -e; $(CC) -M $(CPPFLAGS) $(DEFINES) $(INCLUDE_DIRS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

include $(depends)

%.o : %.cc Makefile
	$(CC) $(OPTIONS) $(DEFINES) $(INCLUDE_DIRS) -c $< -o $@

clean:
	-rm -f $(objects)
	-rm -f $(depends)
	-rm -f gmon.out
	-rm -f $(program)

ctags:
	ctags $(sources) *.h

release: $(program)
	test ! -e $(HOME)/bin/$(relname)
	mv $(HOME)/bin/$(program) $(HOME)/bin/$(relname)
	cvs tag -c release-$(relname)


# Include CMPI headers and lib paths
include ../setting.cmpi

# BY DEFAULT BUILD ALL *.c SOURCES FILES INTO A LIKE-NAMED LIBRARY
SOURCES=$(shell ls -1 *.c)
LIBTARGETS=$(patsubst %.c, lib%.so, $(SOURCES))
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

SUBDIRS=$(shell find -type d -mindepth 1 -maxdepth 1)

# Build Options
export CFLAGS=-Wall -g -DDEBUG -fPIC
CPPFLAGS=-I. $(foreach SUBDIR, $(SUBDIRS), -I./$(SUBDIR)) -I$(CIMOMINC) -I$(COMMONINC)
LDFLAGS=-L. $(foreach SUBDIR, $(SUBDIRS), -L./$(SUBDIR)) -L$(CIMOMLIB) -L$(COMMONLIB) -shared -lpthread -lcmpiOSBase_Common 
# ADD ANY EXTRA REQUIRED LIBRARIES HERE
LOADLIBES=

# REMAINDER OF THIS MAKEFILE GENERALLY WONT NEED TO BE MODIFIED
%.d: %.c
	@$(RM) $@
	$(CC) -MM $(CPPFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@
 
lib%.so: %.o
	$(LINK.c) -shared $^ $(LOADLIBES) $(LDLIBS)-o $@

.PHONY: all clean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBTARGETS)

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) $(DEPFILES) $(LIBTARGETS)

install: $(SUBDIRS) $(LIBTARGETS)
	$(foreach LIB, $(LIBTARGETS), install -m 755 $(LIB) $(CIMOMLIB); )

uninstall: $(SUBDIRS) 
	$(foreach LIB, $(LIBTARGETS), rm -f $(CIMOMLIB)/$(LIB); )

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

ifneq ($(MAKECMDGOALS),clean)
include $(DEPFILES)
endif


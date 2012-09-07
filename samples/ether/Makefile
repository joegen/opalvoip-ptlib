PROG = ether
SOURCES := ether.cxx

ifdef PTLIBDIR
  include $(PTLIBDIR)/make/ptlib.mak
else
  include $(shell pkg-config ptlib --variable=makedir)/ptlib.mak
endif

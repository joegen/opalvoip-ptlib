ifndef PWLIBDIR
PWLIBDIR=$(HOME)/pwlib
endif

PROG		= asnparser
SOURCES		= asn_grammar.cxx asn_lex.cxx main.cxx
TAR_SOURCES	= asn_grammar.y   asn_lex.l   main.cxx main.h Makefile

CLEAN_FILES	= asn_grammar.cxx asn_grammar.h asn_grammar.output asn_lex.cxx


include $(PWLIBDIR)/make/ptlib.mak

install: obj_$(PLATFORM_TYPE)_r/$(PROG)
	-mkdir $(PWLIBDIR)/bin
	install obj_$(PLATFORM_TYPE)_r/$(PROG) $(PWLIBDIR)/bin

asn_grammar.cxx: asn_grammar.y
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	bison -t -v -d -o $* $<
	mv $* $*.cxx

asn_lex.cxx: asn_lex.l
	flex -t $< > $@

tarfiles:
	@echo $(patsubst %, $(notdir $(shell cd ..;pwd))/$(notdir $(shell pwd))/%, $(TAR_SOURCES))

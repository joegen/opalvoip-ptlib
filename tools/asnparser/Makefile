PROG		= asnparser
SOURCES		= asn_grammar.cxx asn_lex.cxx main.cxx

DEBUG = 1


include ~/pwlib/lib/ptlib.mak

asn_grammar.cxx: asn_grammar.y
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	bison -t -v -d -o $* $<
	mv $* $*.cxx

asn_lex.cxx: asn_lex.l
	flex -t $< > $@


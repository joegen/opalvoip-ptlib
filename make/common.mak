######################################################################
#
# common rule
#
######################################################################

#
# create list of object files 
#
OBJS		:= $(OBJS) $(SRCS:cxx=o)
OBJS		:= $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(OBJS)))

#
# add in good files to delete
#
CLEAN_FILES	:= $(CLEAN_FILES) $(OBJS)
CLEAN_FILES	:= $(CLEAN_FILES) core

######################################################################
#
# rules for application
#
######################################################################

ifdef	PROG

all:	$(OBJDIR)/$(PROG)

$(OBJDIR)/$(PROG):	$(OBJS) $(PTLIB_FILE)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(OBJDIR)/$(PROG) $(LDLIBS)

CLEAN_FILES	:= $(CLEAN_FILES) $(OBJDIR)/$(PROG)

# ifdef PROG
endif

######################################################################
#
# common rules for creating dependencies
#
######################################################################

depend: $(SRCS)
	@md -- $(CCONLYFLAGS) -- $(SRCS)
	@mv Makefile Makefile.bak
	@sed '/^# Do not delete/,$$s%$(PWLIBDIR)%$$(PWLIBDIR)%g' < Makefile.bak > Makefile

######################################################################
#
# common rules for cleaning up
#
######################################################################

clean:
	rm -f $(CLEAN_FILES)


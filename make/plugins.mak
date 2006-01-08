include ../../make/unix.mak

PLUGIN_FILENAME = $(PLUGIN_NAME)_pwplugin.$(LIB_SUFFIX)

OBJDIR = ../pwlib/$(PLUGIN_FAMILY)

TARGET = $(OBJDIR)/$(PLUGIN_FILENAME)

ifeq ($(USE_GCC),yes)
  LDSOPTS += -shared
else
 ifeq ($(OSTYPE),solaris)
  LDSOPTS += -G
 endif
endif 
$(OBJDIR)/$(PLUGIN_FILENAME): $(PLUGIN_SOURCES)
	mkdir -p $(OBJDIR)
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) \
	$(PLUGIN_LIBS) \
	-I. $(LDSOPTS) $< -o $@


include ../../make/common.mak

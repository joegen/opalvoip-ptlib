include ../../make/unix.mak

PLUGIN_FILENAME = $(PLUGIN_NAME)_pwplugin.$(LIB_SUFFIX)

OBJDIR = ../pwlib/$(PLUGIN_FAMILY)

TARGET = $(OBJDIR)/$(PLUGIN_FILENAME)

$(OBJDIR)/$(PLUGIN_FILENAME): $(PLUGIN_SOURCES)
	mkdir -p $(OBJDIR)
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) \
	$(PLUGIN_LIBS) \
	-I. -shared $< -o $@


include ../../make/common.mak

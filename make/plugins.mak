include ../../make/unix.mak

PLUGIN_FILENAME = $(PLUGIN_NAME)_pwplugin.$(LIB_SUFFIX)

OBJDIR = ../pwlib/$(PLUGIN_FAMILY)

TARGET = $(OBJDIR)/$(PLUGIN_FILENAME)

ifeq ($(OSTYPE),solaris)
  LDSOPTS += -G
else
  LDSOPTS += -shared
endif

ifeq ($(MACHTYPE),x86_64)
  CFLAGS += -fPIC
endif

$(OBJDIR)/$(PLUGIN_FILENAME): $(PLUGIN_SOURCES)
	mkdir -p $(OBJDIR)
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) \
	$(PLUGIN_LIBS) \
	-I. $(LDSOPTS) $< -o $@

OBJS	 := $(patsubst %.c, $(OBJDIR)/%.o, $(patsubst %.cxx, $(OBJDIR)/%.o, $(notdir $(PLUGIN_SOURCES))))

CLEAN_FILES += $(OBJDIR)/$(PLUGIN_FILENAME)

include ../../make/common.mak

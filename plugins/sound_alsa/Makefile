ifndef PWLIBDIR
PWLIBDIR := $(HOME)/pwlib
endif

include ../../make/unix.mak

BASENAME	= pwplugin_sound_alsa
SOURCES		= sound_alsa.cxx
MAJOR_VERSION	= 1
MINOR_VERSION	= 1
BUILD_NUMBER	= 1

OBJDIR	=	.
LIBDIR	=	.


BASE_FILE  	= lib$(BASENAME).$(LIB_SUFFIX)

TARGET	=	$(LIBDIR)/$(BASE_FILE)

include ../../make/common.mak

P_SHAREDLIB	= 1

LIB_BASENAME=$(BASENAME)
LIB_FILENAME=$(BASE_FILE)

include ../../make/lib.mak

fred:
	@echo ""
	@echo "target is " $(TARGET)
	@echo "lib suffix is" $(LIB_SUFFIX)
	@echo "PWLIBDIR is " $(PWLIBDIR)
	@echo ""


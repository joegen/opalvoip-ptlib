ifneq (,$(wildcard /usr/local/lib/libXm*))
MOTIF_DIR	= /usr/local
else
  ifneq (,$(wildcard /usr/X11R6/lib/libXm*))
  MOTIF_DIR	= /usr/X11R6
  else
    ifneq (,$(wildcard /usr/openwin/lib/libXm*))
    MOTIF_DIR	= /usr/openwin
    endif
  endif
endif
GUILIB		= -L$(MOTIF_DIR)/lib -lMrm -lXm -lXt -lXmu -lX11
STDCCFLAGS	:= $(STDCCFLAGS) -I$(MOTIF_DIR)/include 

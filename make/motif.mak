DEFAULT_X11_DIR = /usr/X11R6

ifneq (,$(wildcard /usr/local/lib/libXm*))
MOTIF_DIR	= /usr/local
else
  ifneq (,$(wildcard $(DEFAULT_X11_DIR)/lib/libXm*))
  MOTIF_DIR	= $(DEFAULT_X11_DIR)
  else
    ifneq (,$(wildcard /usr/openwin/lib/libXm*))
    MOTIF_DIR	= /usr/openwin
    endif
  endif
endif

ifneq ($(DEFAULT_X11_DIR), $(MOTIF_DIR))
GUILIB		:= -L$(DEFAULT_X11_DIR)/lib
STDCCFLAGS	:= $(STDCCFLAGS) -I$(DEFAULT_X11_DIR)/include 
endif

GUILIB		:= $(GUILIB) -L$(MOTIF_DIR)/lib -lMrm -lXm -lXt -lXmu -lX11
STDCCFLAGS	:= $(STDCCFLAGS) -I$(MOTIF_DIR)/include 

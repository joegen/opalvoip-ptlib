DEFAULT_X11_DIR = /usr/X11R6

ifneq (,$(wildcard /usr/local/lib/libXm*))
MOTIF_DIR	= /usr/local
else
  # Search for LessTif's Motif 2.0 support. Some systems install LessTif with
  # the Motif 1.2 libraries as the default libraries in /usr/X11R6/lib.
  ifneq (,$(wildcard /usr/X11R6/LessTif/Motif2.0/lib/libXm*))
  MOTIF_DIR	= /usr/X11R6/LessTif/Motif2.0
  else
    ifneq (,$(wildcard $(DEFAULT_X11_DIR)/lib/libXm*))
    MOTIF_DIR	= $(DEFAULT_X11_DIR)
    else
      ifneq (,$(wildcard /usr/openwin/lib/libXm*))
      MOTIF_DIR	= /usr/openwin
      endif
    endif
  endif
endif

# Specify the MOTIF include and library paths.
# These must come before any X11 paths incase they override them.
GUILIB		:= -L$(MOTIF_DIR)/lib -lMrm -lXm -lXt -lXmu -lX11
STDCCFLAGS	:= $(STDCCFLAGS) -I$(MOTIF_DIR)/include 

# Include the X11 paths if needed.
ifneq ($(DEFAULT_X11_DIR), $(MOTIF_DIR))
GUILIB		:= $(GUILIB) -L$(DEFAULT_X11_DIR)/lib
STDCCFLAGS	:= $(STDCCFLAGS) -I$(DEFAULT_X11_DIR)/include 
endif


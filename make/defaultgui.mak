ifeq (,$(GUI))
ifneq (,wildcard($(PWLIBDIR)/include/pwlib))
ifneq (,wildcard(/usr/local/qt))
GUI = qt
else
  ifneq (,wildcard(/usr/X11R6/include/Xm))
  GUI = motif
  else
    ifneq (,wildcard(/usr/openwin/include/Xm))
    GUI = motif
    else
      ifneq (,wildcard(/usr/X11R6))
      GUI = xlib
      else
        ifneq (,wildcard(/usr/openwin))
        GUI = xlib
        endif
      endif
    endif
  endif
endif
endif
endif

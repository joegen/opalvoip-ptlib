ifeq (,$(GUI_TYPE))
  ifneq (,$(wildcard $(PWLIBDIR)/include/pwlib))
    ifneq (,$(GUI))
      GUI_TYPE = $(GUI)
    else
      ifneq (,$(wildcard /usr/local/qt))
        GUI_TYPE = qt
      else
        ifneq (,$(wildcard /usr/X11R6/include/Xm)$(wildcard /usr/openwin/include/Xm))
          GUI_TYPE = motif
        else
          ifneq (,$(wildcard /usr/X11R6)$(wildcard /usr/openwin))
            GUI_TYPE = xlib
          endif
        endif
      endif
    endif
  endif
endif

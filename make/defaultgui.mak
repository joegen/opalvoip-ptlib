#
# common.mak
#
# Common make rules included in ptlib.mak and pwlib.mak
#
# Portable Windows Library
#
# Copyright (c) 1993-2000 Equivalence Pty. Ltd.
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
# the License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is Portable Windows Library.
#
# The Initial Developer of the Original Code is Equivalence Pty. Ltd.
#
# Contributor(s): ______________________________________.
#
# $Log: defaultgui.mak,v $
# Revision 1.6  2000/03/14 02:17:21  craigs
# Added autodetect of Motif/Lestif directory
# Added detection of Lestif directory when using default install
#
# Revision 1.5  2000/03/03 22:58:24  robertj
# Added copyright header comment.
#

ifeq (,$(GUI_TYPE))
  ifneq (,$(wildcard $(PWLIBDIR)/include/pwlib))
    ifneq (,$(GUI))
      GUI_TYPE = $(GUI)
    else
      ifneq (,$(wildcard /usr/local/qt))
        GUI_TYPE = qt
      else
        ifneq (,$(wildcard /usr/local/include/Xm)$(wildcard /usr/X11R6/include/Xm)$(wildcard /usr/openwin/include/Xm))
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

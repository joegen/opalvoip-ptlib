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
# Revision 1.10  2000/10/06 08:19:52  rogerh
# Fix bug introduced in last commit. Only build motif when pwlib source exists
#
# Revision 1.9  2000/10/01 01:08:11  craigs
# Fixed problems with Motif build
#
# Revision 1.8  2000/04/26 03:00:53  robertj
# Fixed GUI determination to include pwlib/src directory
#
# Revision 1.7  2000/03/20 22:43:10  craigs
# Added totally new mechanism for detecting GUI
#
# Revision 1.6  2000/03/14 02:17:21  craigs
# Added autodetect of Motif/Lestif directory
# Added detection of Lestif directory when using default install
#
# Revision 1.5  2000/03/03 22:58:24  robertj
# Added copyright header comment.
#

# this construct looks in the following directories
#
#  /usr/local/qt		Qt 
#  /usr/X11R6/include/Xm	Motif
#  /usr/openwin/include/Xm	Motif
#  /usr/local/include/Xm	Lesstif
#  /usr/X11R6/LessTif 		Lesstif
#  /usr/local/LessTif		Lesstif
#  /usr/X11R6			Xlib
#  /usr/openwin			Xlib

ifdef GUI_TYPE
PWLIB_GUI	= $(GUI_TYPE)
endif


#############################################
## check for QT

ifdef	PWLIB_GUI
TRIAL_GUI	= $(PWLIB_GUI)
else
TRIAL_GUI	= qt
endif

ifeq (qt,$(TRIAL_GUI))
  ifneq (,$(wildcard $(PWLIBDIR)/src/pwlib/qt))
    ifndef PWLIB_GUIDIR
      ifneq (,$(wildcard /usr/local/qt/include))
        PWLIB_GUIDIR = /usr/local/qt
      endif
    endif
  endif
  ifdef PWLIB_GUIDIR
    PWLIB_GUI=qt
    PWLIB_GUI_FLAG=P_QT
  endif
endif

#############################################
## check for Motif

ifdef	PWLIB_GUI
TRIAL_GUI	= $(PWLIB_GUI)
else
TRIAL_GUI	= motif
endif

ifeq (motif,$(TRIAL_GUI))
  ifneq (,$(wildcard $(PWLIBDIR)/src/pwlib/motif))
    ifneq (,$(wildcard /usr/X11R6/LessTif/Motif2.0/include/Xm))
      PWLIB_GUIDIR  = /usr/X11R6/LessTif/Motif2.0
      PWLIB_GUI_FLAG=P_LESSTIF
    else
      ifneq (,$(wildcard /usr/local/LessTif/Motif2.0/include/Xm))
        PWLIB_GUIDIR  = /usr/local/LessTif/Motif2.0
        PWLIB_GUI_FLAG=P_LESSTIF
      else
        ifndef PWLIB_GUIDIR
          ifneq (,$(wildcard /usr/X11R6/include/Xm/ComboBox.h))
            PWLIB_GUIDIR = /usr/X11R6
            PWLIB_GUI_FLAG=P_MOTIF
          else
            ifneq (,$(wildcard /usr/openwin/include/Xm/ComboBox.h))
              PWLIB_GUIDIR  = /usr/openwin
              PWLIB_GUI_FLAG=P_MOTIF
            else
              ifneq (,$(wildcard /usr/local/include/Xm/ComboBox.h))
                PWLIB_GUIDIR  = /usr/local
                PWLIB_GUI_FLAG=P_LESSTIF
              endif
            endif
          endif
        endif
      endif
    endif
  endif
  ifdef PWLIB_GUIDIR
    PWLIB_GUI=motif
  endif
endif

#############################################
## check for X

ifdef	PWLIB_GUI
TRIAL_GUI	= $(PWLIB_GUI)
else
TRIAL_GUI	= xlib
endif

ifeq (xlib,$(TRIAL_GUI))
  ifneq (,$(wildcard $(PWLIBDIR)/src/pwlib/xlib))
    ifndef PWLIB_GUIDIR
      ifneq (,$(wildcard /usr/X11R6/include))
        PWLIB_GUIDIR = /usr/X11R6
      else
        ifneq (,$(wildcard /usr/openwin/include))
          PWLIB_GUIDIR = /usr/openwin
        endif
      endif
    endif
  endif
  ifdef PWLIB_GUIDIR
    PWLIB_GUI=xlib
    PWLIB_GUI_FLAG=P_XLIB
  endif
endif

#############################################
## clean up

ifdef PWLIB_GUI
ifndef  GUI_TYPE
GUI_TYPE = $(PWLIB_GUI)
endif
endif


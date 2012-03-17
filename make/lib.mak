#
# lib.mak
#
# Make rules for building libraries rather than applications.
#
# Portable Windows Library
#
# Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
# $Revision$
# $Author$
# $Date$
#

ifneq ($(P_SHAREDLIB),1)
  STATIC_LIB_FILE = $(libdir)/$(LIB_FILENAME)
else
  STATIC_LIB_FILE = $(libdir)/$(subst .$(LIB_SUFFIX),_s.$(STATICLIBEXT),$(LIB_FILENAME))
endif

CLEAN_FILES += $(libdir)/$(PTLIB_SONAME) $(libdir)/$(LIB_FILENAME) $(STATIC_LIB_FILE)

$(libdir)/$(LIB_FILENAME) : $(TARGETLIB)

ifeq ($(P_SHAREDLIB),1)

  ENDLDLIBS := $(SYSLIBS) $(ENDLDLIBS)
  ifeq ($(target_os),beos)
    # BeOS requires different options when building shared libraries
    # Also, when building a shared library x that references symbols in libraries y,
    # the y libraries need to be added to the linker command
    LDSOOPTS = -nostdlib -nostart
    EXTLIBS = -lstdc++.r4
  else
    ifeq ($(target_os),Darwin)
      LDSOOPTS = -dynamiclib
    else
      LDSOOPTS = -shared
    endif
  endif

  ifeq ($(target_os),rtems)
    EXTLIBS = -lstdc++
  endif

  ifneq ($(target_os), QNX)
    ifneq (,$(findstring $(target_os),FreeBSD OpenBSDs))
      ifdef P_PTHREADS
        EXTLIBS += -pthread
      endif
    else
      ifdef P_PTHREADS
        EXTLIBS += -lpthread
      endif
    endif
  endif

  # Solaris loader doesn't grok -soname  (sees it as -s -oname)
  # We could use -Wl,-h,$(LIB_BASENAME).1 but then we find that the arglist
  # to gcc is 2900+ bytes long and it will barf.  I fix this by invoking ld
  # directly and passing it the equivalent arguments...jpd@louisiana.edu
  ifeq ($(target_os),solaris)
     LDSOOPTS = -Bdynamic -G -h $(PTLIB_SONAME)
  else
    ifeq ($(target_os),mingw)
      LDSOOPTS += -Wl,--kill-at
    else
      ifneq ($(target_os),Darwin)
        LDSOOPTS += -Wl,-soname,$(PTLIB_SONAME)
      endif
    endif
  endif

  $(libdir)/$(LIB_FILENAME): $(libdir)/$(PTLIB_SONAME)
	@cd $(libdir) ; rm -f $(LIB_FILENAME) ; ln -sf $(PTLIB_SONAME) $(LIB_FILENAME)

  $(libdir)/$(PTLIB_SONAME): $(STATIC_LIB_FILE)
	@if [ ! -d $(libdir) ] ; then mkdir $(libdir) ; fi
	$(Q_LD)$(LD) $(LDSOOPTS) -o $(libdir)/$(PTLIB_SONAME) $(LDFLAGS) $(EXTLIBS) $(OBJS) $(ENDLDLIBS)

endif # P_SHAREDLIB

$(STATIC_LIB_FILE): $(OBJS)
	@if [ ! -d $(libdir) ] ; then mkdir $(libdir) ; fi
	$(Q_AR)$(ARCHIVE) $(STATIC_LIB_FILE) $(OBJS)
ifeq ($(P_USE_RANLIB),1)
	$(RANLIB) $(STATIC_LIB_FILE)
endif



# End of file ################################################################


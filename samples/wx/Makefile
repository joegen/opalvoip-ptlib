#
# Makefile
#
# An PTLib/wxWidgets GUI application.
#
# Copyright (c) 2011-2013 Vox Lucida Pty. ltd.
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
# The Original Code is wxSample.
#
# The Initial Developer of the Original Code is Robert Jongbloed
#
# Contributor(s): ______________________________________.
#

PROG		:= $(shell sed -n -e 's/^.*EXE_NAME_TEXT.*"\(.*\)"/\1/p' version.h)
SOURCES		:= main.cxx resource.cxx

WX_CONFIG	+= wx-config
ifeq ($(DEBUG),1)
WX_CONFIG+=--debug=yes
else
WX_CONFIG+=--debug=no
endif

CPPFLAGS += $(shell $(WX_CONFIG) --cxxflags)
CFLAGS	 += $(shell $(WX_CONFIG) --cxxflags) -Wno-strict-aliasing -Wno-float-equal
LDFLAGS  += $(shell $(WX_CONFIG) --libs)

# Need this before the include
default_target: bundle


ifdef PTLIBDIR
  include $(PTLIBDIR)/make/ptlib.mak
else
  include $(shell pkg-config ptlib --variable=makedir)/ptlib.mak
endif


resource.cxx : resource.xrc
	wxrc $? -c -o $@


ifneq ($(target_os),Darwin)

bundle: $(TARGET)

else

bundle: $(PROG).app

ICONFILE    := app.icns
STRINGSFILE := #InfoPlist.strings

SED := sed -e "s/EXE_NAME_TEXT/$(PROG)/g" \
           -e "s/PRODUCT_NAME_TEXT/$(shell sed -n -e 's/^.*PRODUCT_NAME_TEXT.*"\(.*\)"/\1/p' version.h)/g" \
           -e "s/MANUFACTURER_TEXT/$(shell sed -n -e 's/^.*MANUFACTURER_TEXT.*"\(.*\)"/\1/p' version.h)/g" \
           -e "s/COPYRIGHT_HOLDER/$(shell sed -n -e 's/^.*COPYRIGHT_HOLDER.*"\(.*\)"/\1/p' version.h)/g" \
           -e "s/COPYRIGHT_YEAR/$(shell sed -n -e 's/^.*COPYRIGHT_YEAR.*"\(.*\)"/\1/p' version.h)/g" \
           -e "s/MAJOR_VERSION/$(shell sed -n -e 's/^.*MAJOR_VERSION[ ]*\([0-9]*\)/\1/p' version.h)/g" \
           -e "s/MINOR_VERSION/$(shell sed -n -e 's/^.*MINOR_VERSION[ ]*\([0-9]*\)/\1/p' version.h)/g" \
           -e "s/BUILD_TYPE/$(shell sed -n -e 's/^.*BUILD_TYPE[ ]*\([A-Za-z]*\)/\1/p' version.h)/g" \
           -e "s/PATCH_VERSION/$(shell sed -n -e 's/^.*PATCH_VERSION[ ]*\([0-9]*\)/\1/p' version.h)/g" \

$(PROG).app: $(TARGET) Info.plist version.plist $(ICONFILE) $(STRINGSFILE)
	@ for dir in $@ \
		     $@/Contents \
		     $@/Contents/MacOS \
		     $@/Contents/Resources \
		     $@/Contents/Resources/English.lproj; do \
	    if test ! -d $$dir; then \
	      echo Creating $$dir; \
	      mkdir $$dir; \
	    fi \
	  done
	$(SED) Info.plist    > $@/Contents/INfo.plist
	$(SED) version.plist > $@/Contents/version.plist
	echo -n 'APPL????' > $@/Contents/PkgInfo
ifneq ($(STRINGSFILE),)	
	cp InfoPlist.strings $@/Contents/Resources/English.lproj/
endif
	cp $(ICONFILE) $@/Contents/Resources/$(PROG).icns
	cp $(TARGET) $@/Contents/MacOS/$(PROG)

endif

# End of Makefile

cd ..
wzzip -P pwlib_min.zip pwlib/Readme.txt pwlib/mpl-1.0.htm pwlib/*.dsw pwlib/include/ptlib.h
wzzip -rP -x@pwlib/exclusions.txt pwlib_min.zip pwlib/include/ptclib
wzzip -rP -x@pwlib/exclusions.txt pwlib_min.zip pwlib/include/ptlib
wzzip -rP -x@pwlib/exclusions.txt pwlib_min.zip pwlib/src/ptclib
wzzip -rP -x@pwlib/exclusions.txt pwlib_min.zip pwlib/src/ptlib
wzzip -P pwlib_min.zip pwlib/tools/MergeSym/*.ds? pwlib/tools/MergeSym/MergeSym.cxx
wzzip -P pwlib_min.zip pwlib/tools/asnparser/*.ds? pwlib/tools/asnparser/main.* pwlib/tools/asnparser/precompile.cpp pwlib/tools/asnparser/*.y pwlib/tools/asnparser/*.l

wzzip -P pwlib_full.zip pwlib/Readme.txt pwlib/mpl-1.0.htm pwlib/*.dsw
wzzip -rP -x@pwlib/exclusions.txt pwlib_full.zip pwlib/include
wzzip -rP -x@pwlib/exclusions.txt pwlib_full.zip pwlib/src
wzzip -P pwlib_full.zip pwlib/tools/MergeSym/*.ds? pwlib/tools/MergeSym/MergeSym.cxx
wzzip -P pwlib_full.zip pwlib/tools/asnparser/*.ds? pwlib/tools/asnparser/main.* pwlib/tools/asnparser/precompile.cpp pwlib/tools/asnparser/*.y pwlib/tools/asnparser/*.l
wzzip -P pwlib_full.zip pwlib/tools/pwrc/*.ds? pwlib/tools/pwrc/pwrc.h pwlib/tools/pwrc/main.cxx pwlib/tools/pwrc/codegen.cxx pwlib/tools/pwrc/windows.cxx pwlib/tools/pwrc/dwindows.cxx pwlib/tools/pwrc/precompile.cxx pwlib/tools/pwrc/*.y pwlib/tools/pwrc/*.l
wzzip -P pwlib_full.zip pwlib/tools/pwtest/*.ds? pwlib/tools/pwtest/main.* pwlib/tools/pwtest/precompile.cxx pwlib/tools/pwtest/resources.prc

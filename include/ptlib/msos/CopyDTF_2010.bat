mkdir backup

move ptlibd_2010.dtf backup\
copy ..\..\..\lib\Win32\debug\ptlibd.def ptlibd_2010.dtf
move ptlib_2010.dtf backup\
copy ..\..\..\lib\Win32\release\ptlib.def ptlib_2010.dtf
move ptlibn_2010.dtf backup\
copy "..\..\..\lib\Win32\No Trace\ptlibn.def" ptlibn_2010.dtf

move ptlibd_2010_x64.dtf backup\
copy ..\..\..\lib\x64\debug\ptlib64d.def ptlibd_2010_x64.dtf
move ptlib_2010_x64.dtf backup\
copy ..\..\..\lib\x64\release\ptlib64.def ptlib_2010_x64.dtf
move ptlibn_2010_x64.dtf backup\
copy "..\..\..\lib\x64\No Trace\ptlib64n.def" ptlibn_2010_x64.dtf

pause
/*
 * main.h
 *
 * PWLib application header file for vidtest
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: main.h,v $
 * Revision 1.2  2003/04/29 00:57:21  dereks
 * Add user interface, option setting for format/input/fake. Works on Linux.
 *
 * Revision 1.1  2003/04/28 08:18:42  craigs
 * Initial version
 *
 */

#ifndef _Vidtest_MAIN_H
#define _Vidtest_MAIN_H




class VidTest : public PProcess
{
  PCLASSINFO(VidTest, PProcess)

  public:
    VidTest();
    virtual void Main();
    void HandleUserInterface();

 protected:
    PVideoInputDevice * grabber;
    PVideoChannel     * channel;
    PSyncPoint          exitFlag;
};

class UserInterfaceThread : public PThread
{
    PCLASSINFO(UserInterfaceThread, PThread);
  public:
    UserInterfaceThread(VidTest & newVidTest)
      : PThread(1000, NoAutoDeleteThread), vidTest(newVidTest) { Resume(); }
    void Main()
      { vidTest.HandleUserInterface(); }
  protected:
    VidTest &vidTest;
};


#endif  // _Vidtest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

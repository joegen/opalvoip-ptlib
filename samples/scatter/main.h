/*
 * main.h
 *
 * PWLib application header file for ScatterTest
 *
 * Copyright (c) 2010 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 25359 $
 * $Author: rjongbloed $
 * $Date: 2011-03-18 15:47:47 +1100 (Fri, 18 Mar 2011) $
 */

#ifndef _ScatterTest_MAIN_H
#define _ScatterTest_MAIN_H

#include <ptlib/pprocess.h>

class ScatterTest : public PProcess
{
  PCLASSINFO(ScatterTest, PProcess)

  public:
    ScatterTest();
    virtual void Main();
};


#endif  // _ScatterTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

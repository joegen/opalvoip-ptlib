/*
 * main.h
 *
 * PWLib application header file for PluginTest
 *
 * Copyright 2003 Equivalence
 *
 * $Log: main.h,v $
 * Revision 1.1.2.1  2003/10/07 01:52:39  csoutheren
 * Test program for plugins
 *
 * Revision 1.1  2003/04/15 04:12:38  craigs
 * Initial version
 *
 */

#ifndef _PluginTest_MAIN_H
#define _PluginTest_MAIN_H




class PluginTest : public PProcess
{
  PCLASSINFO(PluginTest, PProcess)

  public:
    PluginTest();
    void Main();
};


#endif  // _PluginTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

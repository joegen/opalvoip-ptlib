/*
 * main.h
 *
 * PWLib application header file for vxmltest
 *
 * Copyright 2002 Equivalence
 *
 * $Log: main.h,v $
 * Revision 1.1  2002/08/06 05:26:33  craigs
 * Initial version
 *
 */

#ifndef _Vxmltest_MAIN_H
#define _Vxmltest_MAIN_H




class Vxmltest : public PProcess
{
  PCLASSINFO(Vxmltest, PProcess)

  public:
    Vxmltest();
    void Main();
};


#endif  // _Vxmltest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

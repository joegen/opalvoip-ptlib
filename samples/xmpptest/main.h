/*
 * main.h
 *
 * PWLib application header file for XMPPTest
 *
 * Copyright 2004 Reitek S.p.A.
 *
 * $Log: main.h,v $
 * Revision 1.1  2004/04/26 01:51:58  rjongbloed
 * More implementation of XMPP, thanks a lot to Federico Pinna & Reitek S.p.A.
 *
 */

#ifndef _XMPPTest_MAIN_H
#define _XMPPTest_MAIN_H




class XMPPTest : public PProcess
{
  PCLASSINFO(XMPPTest, PProcess)

public:
  XMPPTest();
  void Main();

  PDECLARE_NOTIFIER(XMPP::Message, XMPPTest, OnMessage);
};


#endif  // _XMPPTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

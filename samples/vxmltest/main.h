/*
 * main.h
 *
 * PWLib application header file for vxmltest
 *
 * Copyright 2002 Equivalence
 *
 */

#ifndef _Vxmltest_MAIN_H
#define _Vxmltest_MAIN_H

#include <ptlib/pprocess.h>

class PVXMLSession;
class PVideoOutputDevice;


class VxmlTest : public PProcess
{
  PCLASSINFO(VxmlTest, PProcess)

  public:
    VxmlTest();
    void Main();

  protected:
    PSoundChannel      * m_player;
    PVideoOutputDevice * m_viewer;
    PVXMLSession       * m_vxml;

#if P_VXML
    void HandleInput(PConsoleChannel & console);
    void CopyAudio();
#if P_VIDEO
    void CopyVideo();
#endif
#endif
};


#endif  // _Vxmltest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

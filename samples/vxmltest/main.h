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
class PVideoInputDevice;
class PVideoOutputDevice;


class TestInstance
{
#if P_VXML
  public:
    TestInstance();
    ~TestInstance();

    bool Initialise(unsigned instance, const PArgList & args);
    void SendInput(char c);

  protected:
    unsigned             m_instance;
    PSoundChannel      * m_player;
    PVideoInputDevice  * m_grabber;
    PVideoOutputDevice * m_viewer;
    PVXMLSession       * m_vxml;

    PThread * m_audioThread;
    void CopyAudio();

#if P_VXML_VIDEO
    PThread * m_videoSenderThread;
    void CopyVideoSender();

    PThread * m_videoReceiverThread;
    void CopyVideoReceiver();
#endif // P_VXML_VIDEO
#endif // P_VXML
};


class VxmlTest : public PProcess
{
  PCLASSINFO(VxmlTest, PProcess)

  public:
    VxmlTest();
    void Main();

  protected:
    PDECLARE_NOTIFIER(PCLI::Arguments, VxmlTest, SimulateInput);
    std::vector<TestInstance> m_tests;
};


#endif  // _Vxmltest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

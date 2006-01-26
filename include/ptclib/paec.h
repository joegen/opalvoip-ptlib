/*
 * paec.h
 *
 * Open Phone Abstraction Library (OPAL)
 * Formally known as the Open H323 project.
 *
 * Copyright (c) 2004 Post Increment
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
 * The Original Code is Open Phone Abstraction Library.
 *
 * The author of this code is Damien Sandras
 *
 * rewritten amd made generic ptlib by Simon Horne
 *
 * Contributor(s): Miguel Rodriguez Perez
 *
 * $Log: paec.h,v $
 * Revision 1.1  2006/01/26 08:05:03  shorne
 * Added AEC support
 *
 */

#ifndef __OPAL_ECHOCANCEL_H
#define __OPAL_ECHOCANCEL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/qchannel.h>

/** This class implements Acoustic Echo Cancellation
  * The principal is to copy to a buffer incoming audio.
  * after it has been decoded and when recording the audio 
  * to remove the echo pattern from the incoming audio 
  * prior to sending to the enooder..
  */


struct SpeexEchoState;
struct SpeexPreprocessState;
class PAEC : public PObject
{
  PCLASSINFO(PAEC, PObject);
public:

  /**@name Construction */
  //@{
  /**Create a new canceler.
   */
     PAEC();
     ~PAEC();
  //@}

  /**@@name Basic operations */
  //@{
  /**Recording Channel. Should be called prior to encoding audio
   */
    void Send(BYTE * buffer, unsigned & length);

  /**Playing Channel  Should be called after decoding and prior to playing.
   */
    void Receive(BYTE * buffer, unsigned & length);
  //@}

protected:

  PQueueChannel *echo_chan;
  SpeexEchoState *echoState;
  SpeexPreprocessState *preprocessState;
  short *ref_buf;
  short *echo_buf;
  short *e_buf;
  float *noise;

};

#endif // __OPAL_ECHOCANCEL_H

/////////////////////////////////////////////////////////////////////////////

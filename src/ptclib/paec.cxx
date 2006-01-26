/*
 * paec.cxx
 *
 * Open Phone Abstraction Library (OPAL)
 * Formally known as the Open H323 project.
 *
 * Copyright (c) 2001 Post Increment
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
 * Contributor(s): Miguel Rodriguez Perez.
 *
 * $Log: paec.cxx,v $
 * Revision 1.1  2006/01/26 08:05:03  shorne
 * Added AEC support
 *
 *
 */

#include <ptlib.h>

#include "ptclib/paec.h"

extern "C" {
#include "speex_echo/speex_echo.h"
#include "speex_echo/speex_preprocess.h"
}

///////////////////////////////////////////////////////////////////////////////

PAEC::PAEC()
{
  echoState = NULL;
  preprocessState = NULL;

  e_buf = NULL;
  echo_buf = NULL;
  ref_buf = NULL;
  noise = NULL;

  echo_chan = new PQueueChannel();
  echo_chan->Open(10000);
  echo_chan->SetReadTimeout(10);
  echo_chan->SetWriteTimeout(10);

  PTRACE(3, "AEC\tCanceller created");
}


PAEC::~PAEC()
{

if (echoState) {
	speex_echo_state_destroy(echoState);
	speex_preprocess_state_destroy(preprocessState);
	free(e_buf);
	free(echo_buf);
	free(noise);
}
 
  echo_chan->Close();
  delete(echo_chan);

}


void PAEC::Receive(BYTE * buffer, unsigned & length)
{
  /* Write to the soundcard, and write the frame to the PQueueChannel */
  echo_chan->Write(buffer, length);
}


void PAEC::Send(BYTE * buffer, unsigned & length)
{

  /* Audio Recording to send */
// Iniiialise the Echo Canceller
  if (echoState == NULL) {
    echoState = speex_echo_state_init(length/sizeof(short), 8*length);
	echo_buf = (short *) malloc(length);
	noise = (float *) malloc((length/sizeof(short)+1)*sizeof(float));
    e_buf = (short *) malloc(length);
    ref_buf = (short *) malloc(length);

	int j=1;
    preprocessState = speex_preprocess_state_init(length/sizeof(short), 8000);
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DENOISE, &j);
    speex_preprocess_ctl(preprocessState, SPEEX_PREPROCESS_SET_DEREVERB, &j);
  }
  
  /* Read from the PQueueChannel a reference echo frame of the size
   * of the captured frame. */
  echo_chan->Read(echo_buf, length);
   
  /* Cancel the echo in this frame */
  speex_echo_cancel(echoState, ref_buf, echo_buf, e_buf, noise);
  
  /* Suppress the noise */
  speex_preprocess(preprocessState, (__int16*)e_buf, noise);

  /* Use the result of the echo cancelation as capture frame */
  memcpy(buffer, e_buf, length);
}

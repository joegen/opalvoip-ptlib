/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * Extract DTMF signals from 16 bit PCM audio
 *
 * Originally written by Poul-Henning Kamp <phk@freebsd.org>
 * Made into a C++ class by Roger Hardiman <roger@freebsd.org>, January 2002
 *
 * $Log: dtmf.h,v $
 * Revision 1.1  2002/01/23 11:43:26  rogerh
 * Add DTMF Decoder class. This can be passed PCM audio data
 * (at 16 bit, 8 KHz) and returns any DTMF codes detected.
 * Tested with NetMeeting sending DTMF over a G.711 stream.
 *
 */
 
#ifndef _DTMF_H
#define _DTMF_H

#ifdef __GNUC__
#pragma interface
#endif

class PDTMFDecoder : public PObject
{
  PCLASSINFO(PDTMFDecoder, PObject)

  public:
    PDTMFDecoder();
    PString Decode(const void *buf, PINDEX bytes);

  protected:
    // key lookup table (initialised once)
    char key[256];

    // frequency table (initialised once)
    int p1[8];

    // variables to be retained on each cycle of the decode function
    int h[8], k[8], y[8];
    int nn, so, ia;
};
#endif /* _DTMF_H */

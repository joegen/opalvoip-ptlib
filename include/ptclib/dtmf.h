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
 * Revision 1.4  2004/09/09 05:23:37  dereksmithies
 * Add utility function to report on dtmf characters used.
 *
 * Revision 1.3  2004/09/09 04:00:00  csoutheren
 * Added DTMF encoding functions
 *
 * Revision 1.2  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.1  2002/01/23 11:43:26  rogerh
 * Add DTMF Decoder class. This can be passed PCM audio data
 * (at 16 bit, 8 KHz) and returns any DTMF codes detected.
 * Tested with NetMeeting sending DTMF over a G.711 stream.
 *
 */
 
#ifndef _DTMF_H
#define _DTMF_H

#ifdef P_USE_PRAGMA
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

/**
  * this class can be used to generate PCM data for tones (such as DTMF) 
  * at a sample rate of 8khz
  */

class PDTMFEncoder : public PBYTEArray
{
  PCLASSINFO(PDTMFEncoder, PBYTEArray)

  public:
    enum { DefaultToneLen = 100 };

    /**
      * Create PCM data for the specified DTMF sequence 
      */
    inline PDTMFEncoder(const char * dtmf = NULL, unsigned len = DefaultToneLen)
    { if (dtmf != NULL) AddTone(dtmf, len); }


    /**
      * Add the PCM data for the specified tone to the buffer
      */
    void AddTone(char ch, unsigned len = DefaultToneLen);

    /**
      * Add the PCM data for the specified tone sequence to the buffer
      */
    void AddTone(const PString & str, unsigned len = DefaultToneLen);

    /**
      * Add the PCM data for the specified dual-frequency tone to the buffer
      * freq2 can be zero, which will generate a single frequency tone
      */
    void AddTone(double freq1, double freq2 = 0, unsigned len = DefaultToneLen);

    /**
      * Generate PCM data for a single cadence of the US standard ring tone
      * of 440/480hz for 2 seconds, followed by 5 seconds of silence
      */
    void GenerateRingBackTone()
    {
      AddTone(440, 480, 2000);
      AddTone(0,   0,   4000);
    }

    /**
      * Generate PCM data for 1 second of US standard dial tone 
      * of 350/440hz 
      */
    void GenerateDialTone()
    {
      AddTone(350, 440, 1000);
    }

    /**
      * Generate PCM data for a single cadence of the US standard busy tone
      * of 480/620hz for 1/2 second, 1/2 second of silence
      */
    void GenerateBusyTone()
    {
      AddTone(480, 620, 500);
      AddTone(0,   0,   500);
    }

    /**
     * Convenience function to get the dtmf symbol for index i, where i is
     * in the range of 0 to 15
     */

    char DtmfChar(PINDEX i);

  protected:
    static PMutex & GetMutex();
    static BOOL sineTabInit;
    static void MakeSineTable();
    static double sinetab[1 << 11];
};

#endif /* _DTMF_H */

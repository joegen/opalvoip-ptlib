/*
 * $Id: debstrm.h,v 1.3 1996/08/17 10:00:40 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: debstrm.h,v $
 * Revision 1.3  1996/08/17 10:00:40  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.2  1994/12/21 11:57:19  robertj
 * Fixed debugging stream.
 *
 * Revision 1.1  1994/08/21  23:43:02  robertj
 * Initial revision
 *
 * Revision 1.1  1994/07/27  05:58:07  robertj
 * Initial revision
 *
 */


#ifndef _PDEBUGSTREAM

///////////////////////////////////////////////////////////////////////////////
// PDebugStream for MS-Windows

class PEXPORT PDebugStream : public ostream {
  public:
    PDebugStream();

  private:
    class PEXPORT Buffer : public streambuf {
      public:
        Buffer();
        virtual int overflow(int=EOF);
        virtual int underflow();
        virtual int sync();
        char buffer[250];
    } buffer;
};



#endif

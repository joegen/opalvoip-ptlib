/*
 * $Id: debstrm.h,v 1.1 1994/08/21 23:43:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: debstrm.h,v $
 * Revision 1.1  1994/08/21 23:43:02  robertj
 * Initial revision
 *
 * Revision 1.1  1994/07/27  05:58:07  robertj
 * Initial revision
 *
 */


#ifndef _PDEBUGSTREAM

///////////////////////////////////////////////////////////////////////////////
// PDebugStream

PCLASS PDebugStream : public ostream {
  public:
    PDebugStream() : ostream(&buffer) { }
    ~PDebugStream() { buffer.sync(); }

  private:
    PDebugStream(const PDebugStream &) { }
    PDebugStream & operator=(const PDebugStream &) { return *this; }

    class Buffer : public streambuf {
      public:
        Buffer();
        virtual int overflow(int=EOF);
        virtual int underflow() { return EOF; }
        virtual int sync() { return overflow(EOF); }
        char buffer[250];
    } buffer;
};



#endif

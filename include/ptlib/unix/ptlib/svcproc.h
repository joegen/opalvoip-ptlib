/*
 * $Id: svcproc.h,v 1.7 1998/05/30 13:30:11 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: svcproc.h,v $
 * Revision 1.7  1998/05/30 13:30:11  robertj
 * Added ability to specify the log file as well as just console output.
 *
 * Revision 1.6  1996/09/21 05:42:12  craigs
 * Changes for new common files, PConfig changes and signal handling
 *
 * Revision 1.5  1996/09/03 11:56:56  craigs
 * Changed PSYSTEMLOG to user cerr
 *
 * Revision 1.4  1996/08/03 12:10:46  craigs
 * Changed for new common directories and added new PSystemLog macro
 *
 * Revision 1.3  1996/06/19 12:33:45  craigs
 * Added ^C handling
 *
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1996/01/26 11:06:31  craigs
 * Initial revision
 *
 */

#ifndef _PSERVICEPROCESS

#pragma interface

#include "../../common/ptlib/svcproc.h"
  protected:
    void _PXShowSystemWarning(PINDEX num, const PString & str);
    void PXOnSignal(int);
    void PXOnAsyncSignal(int);
    PString systemLogFile;
};

#endif

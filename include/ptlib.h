/*
 * $Id: ptlib.h,v 1.1 1994/04/01 14:41:11 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: ptlib.h,v $
 * Revision 1.1  1994/04/01 14:41:11  robertj
 * Initial revision
 *
 */

#ifndef _PTLIB_H
#define _PTLIB_H

#include "contain.h"

#include <windef.h>
#include <winbase.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <direct.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <dos.h>
#include <io.h>


///////////////////////////////////////////////////////////////////////////////
// Operating System dependent declarations

const char PDIR_SEPARATOR = '\\';

const PINDEX P_MAX_PATH = _MAX_PATH;

typedef DWORD PMilliseconds;
const PMilliseconds PMaxMilliseconds = 0xffffffff;

#define EXPORTED __stdcall



///////////////////////////////////////////////////////////////////////////////
// PTime

#include "../../common/ptime.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

#include "../../common/timeint.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTimer

#include "../../common/timer.h"
  protected:
    void SetWindowsTimer();
    int timerID;
};


///////////////////////////////////////////////////////////////////////////////
// PDirectory

#include "../../common/pdirect.h"
  protected:
    HANDLE hFindFile;
    WIN32_FIND_DATA fileinfo;

    BOOL Filtered();
};


///////////////////////////////////////////////////////////////////////////////
// PFile

#include "../../common/file.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTextFile

#include "../../common/textfile.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTextInFile PTextOutFile

#include "../../common/textfio.h"


///////////////////////////////////////////////////////////////////////////////
// PStructuredFile

#include "../../common/sfile.h"
};


///////////////////////////////////////////////////////////////////////////////
// PArgList

#include "../../common/args.h"


///////////////////////////////////////////////////////////////////////////////
// PTextApplication

#include "../../common/textapp.h"
};


///////////////////////////////////////////////////////////////////////////////


#if defined(P_USE_INLINES)

#include "../../common/osutil.inl"
#include "ptlib.inl"

#endif


#endif // _PTLIB_H


// End Of File ///////////////////////////////////////////////////////////////

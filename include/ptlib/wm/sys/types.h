//////////////////////////////////////////////////////
//
// VisualStudio 2005 PWLib Port, 
// (c) 2007 Dinsk.net
// developer@dinsk.net 
//
//////////////////////////////////////////////////////
//
// (c) Yuri Kiryanov, developer@dinsk.net
// for Openh323, www.Openh323.org
//
// Windows CE Port
// sys/types.h - types returned by system level calls for file and time info 
// 
// [Microsoft]
// [System V]
// [Public]

#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#ifndef _OFF_T_DEFINED
typedef long off_t;
#define _OFF_T_DEFINED
#endif

#ifdef __cplusplus

typedef DWORD  SERVICE_STATUS_HANDLE;
typedef struct _SERVICE_STATUS {
    DWORD   dwServiceType;
    DWORD   dwCurrentState;
    DWORD   dwControlsAccepted;
    DWORD   dwWin32ExitCode;
    DWORD   dwServiceSpecificExitCode;
    DWORD   dwCheckPoint;
    DWORD   dwWaitHint;
} SERVICE_STATUS, *LPSERVICE_STATUS;

#endif 
#endif  /* TYPES_H */

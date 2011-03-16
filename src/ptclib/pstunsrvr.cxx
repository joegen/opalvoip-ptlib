#ifdef __GNUC__
#pragma implementation "pstunsrvr.h"
#endif

#include <ptlib.h>
#include <ptclib/pstun.h>
#include <ptclib/random.h>
#include <ptclib/pstunsrvr.h>

#define new PNEW


PSTUNServer::PSTUNServer()
{
}

bool PSTUNServer::Open(WORD /*port*/)
{
  return false;
}

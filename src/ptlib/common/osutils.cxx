/*
 * $Id: osutils.cxx,v 1.1 1993/08/27 18:17:47 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutils.cxx,v $
 * Revision 1.1  1993/08/27 18:17:47  robertj
 * Initial revision
 *
 */

#include "osutil.h"

#include <fcntl.h>
#include <errno.h>


///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

#if defined(_PTIMEINTERVAL)

PTimeInterval::PTimeInterval(long millisecs,
                                 int seconds, int minutes, int hours, int days)
{
  milliseconds = millisecs+1000L*(seconds+60L*(minutes+60L*(hours+24L*days)));
}


PObject::Comparison PTimeInterval::Compare(const PObject & obj) const
{
  const PTimeInterval & other = (const PTimeInterval &)obj;
  return milliseconds < other.milliseconds ? LessThan :
         milliseconds > other.milliseconds ? GreaterThan : EqualTo;
}


ostream & PTimeInterval::PrintOn(ostream & strm) const
{
  int days = (int)(milliseconds/86400000);
  int hours = (int)(milliseconds%86400000/3600000);
  int minutes = (int)(milliseconds%3600000/60000);
  int seconds = (int)(milliseconds%60000/1000);
  int msecs = (int)(milliseconds%1000);
  return strm << days << ':' << hours << ':' << 
                                     minutes << ':' << seconds << '.' << msecs;
}


istream & PTimeInterval::ReadFrom(istream &strm)
{
  return strm;
}


void PTimeInterval::SetInterval(long millisecs,
                                 int seconds, int minutes, int hours, int days)
{
  milliseconds = millisecs+1000L*(seconds+60L*(minutes+60L*(hours+24L*days)));
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PTime

#if defined(_PTIME)

PObject::Comparison PTime::Compare(const PObject & obj) const
{
  const PTime & other = (const PTime &)obj;
  return theTime < other.theTime ? LessThan :
         theTime > other.theTime ? GreaterThan : EqualTo;
}


PTime::PTime(int second, int minute, int hour, int day, int month, int year)
{
  struct tm t;
  PAssert(second >= 0 && second <= 59);
  t.tm_sec = second;
  PAssert(minute >= 0 && minute <= 59);
  t.tm_min = minute;
  PAssert(hour >= 0 && hour <= 23);
  t.tm_hour = hour;
  PAssert(day >= 1 && day <= 31);
  t.tm_mday = day;
  PAssert(month >= 1 && month <= 12);
  t.tm_mon = month-1;
  PAssert(year >= 1900);
  t.tm_year = year-1900;
  theTime = mktime(&t);
  PAssert(theTime != -1);
}


istream & PTime::ReadFrom(istream &strm)
{
  return strm;
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PDirectory

#if defined(_PDIRECTORY)

istream & PDirectory::ReadFrom(istream & strm)
{
  strm >> path;
  Construct();
  return strm;
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PFile

#if defined(_PFILE)

PFile::PFile(const PString & name, OpenMode mode, int opts)
  : fullname(name)
{
  Construct();
  Open(mode, opts);
}


BOOL PFile::Rename(const PString & newname)
{
  if (!Rename(fullname, newname))
    return FALSE;
  fullname = newname;
  return TRUE;
}


BOOL PFile::Close()
{
  if (os_handle < 0)
    return FALSE;

  BOOL ok = close(os_handle) == 0;
  os_errno = ok ? 0 : errno;
  os_handle = -1;
  return ok;
}


BOOL PFile::Read(void * buffer, size_t amount)
{
  BOOL ok = read(GetHandle(), buffer, amount) == (int)amount;
  os_errno = ok ? 0 : errno;
  return ok;
}


BOOL PFile::Write(const void * buffer, size_t amount)
{
  BOOL ok = write(GetHandle(), buffer, amount) == (int)amount;
  os_errno = ok ? 0 : errno;
  return ok;
}


off_t PFile::GetLength()
{
  off_t pos = GetPosition();
  SetPosition(0,End);
  off_t len = GetPosition();
  SetPosition(pos);
  return len;
}


BOOL PFile::Copy(const PString & oldname, const PString & newname)
{
  PFile oldfile(oldname, ReadOnly);
  if (!oldfile.IsOpen())
    return FALSE;

  PFile newfile(newname, WriteOnly, Create|Truncate);
  if (!newfile.IsOpen())
    return FALSE;

  PCharArray buffer(10000);

  off_t amount = oldfile.GetLength();
  while (amount > 10000) {
    if (!oldfile.Read(buffer.GetPointer(), 10000))
      return FALSE;
    if (!newfile.Write(buffer, 10000))
      return FALSE;
    amount -= 10000;
  }

  if (!oldfile.Read(buffer.GetPointer(), (int)amount))
    return FALSE;
  if (!oldfile.Write(buffer, (int)amount))
    return FALSE;

  return newfile.Close();
}


#endif


// End Of File ///////////////////////////////////////////////////////////////

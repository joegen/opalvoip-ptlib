/*
 * $Id: osutils.cxx,v 1.7 1994/03/07 07:47:00 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutils.cxx,v $
 * Revision 1.7  1994/03/07 07:47:00  robertj
 * Major upgrade
 *
 * Revision 1.6  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.5  1993/12/31  06:53:02  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.4  1993/12/29  04:41:26  robertj
 * Mac port.
 *
 * Revision 1.3  1993/11/20  17:26:28  robertj
 * Removed separate osutil.h
 *
 * Revision 1.2  1993/08/31  03:38:02  robertj
 * G++ needs explicit casts for char * / void * interchange.
 *
 * Revision 1.1  1993/08/27  18:17:47  robertj
 * Initial revision
 *
 */

#include "ptlib.h"

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
// PTimer

#if defined(_PTIMER)

PTimer::PTimer(PTextApplication * app,
              long milliseconds, int seconds, int minutes, int hours, int days)
  : PTimeInterval(milliseconds, seconds, minutes, hours, days),
    owner(PAssertNULL(app)),
    state(stopped)
{
}


PTimer::PTimer(const PTimer & timer)
  : PTimeInterval(timer), 
    owner(timer.owner), 
    state(stopped)
{
}


PTimer & PTimer::operator=(const PTimer & timer)
{
  PTimeInterval::operator=(timer);
  owner = timer.owner;
  state = stopped;
  return *this;
}


PTimer::~PTimer()
{
  if (state == running)
    owner->RemoveTimer(this);
}


PObject::Comparison PTimer::Compare(const PObject & obj) const
{
  const PTimer & other = (const PTimer &)obj;
  return targetTime < other.targetTime ? LessThan :
         targetTime > other.targetTime ? GreaterThan : EqualTo;
}


void PTimer::Start(BOOL once)
{
  if (state == running)
    owner->RemoveTimer(this);
  oneshot = once;
  targetTime = Tick() + milliseconds;
  owner->AddTimer(this);
  state = running;
}


void PTimer::Stop()
{
  if (state == running)
    owner->RemoveTimer(this);
  state = stopped;
  targetTime = PMaxMilliseconds;
}


void PTimer::Pause()
{
  if (state == running) {
    owner->RemoveTimer(this);
    pauseLeft = targetTime - Tick();
    state = paused;
  }
}


void PTimer::Resume()
{
  if (state == paused) {
    targetTime = Tick() + pauseLeft;
    owner->AddTimer(this);
    state = running;
  }
}


void PTimer::OnTimeout()
{
  // Empty callback function
}


PMilliseconds PTimerList::Process()
{
  PTimer * timer = (PTimer *)GetAt(0); // Get earliest timer value
  if (timer != NULL) {
    if (PTimer::Tick() > timer->targetTime) {
      if (timer->oneshot)
        timer->Stop();
      else
        timer->Start(FALSE);
      timer->OnTimeout();
    }
    timer = (PTimer *)GetAt(0); // Get new earliest timer value
  }

  return timer != NULL ? timer->targetTime - PTimer::Tick() : PMaxMilliseconds;
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
  BOOL ok = read(GetHandle(), (char *)buffer, amount) == (int)amount;
  os_errno = ok ? 0 : errno;
  return ok;
}


BOOL PFile::Write(const void * buffer, size_t amount)
{
  BOOL ok = write(GetHandle(), (char *)buffer, amount) == (int)amount;
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
    if (!newfile.Write((const char *)buffer, 10000))
      return FALSE;
    amount -= 10000;
  }

  if (!oldfile.Read(buffer.GetPointer(), (int)amount))
    return FALSE;
  if (!oldfile.Write((const char *)buffer, (int)amount))
    return FALSE;

  return newfile.Close();
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PArgList

#if defined(_PARGLIST)

PArgList::PArgList()
{
  arg_values = NULL;
  arg_count  = 0;
}


PArgList::PArgList(int theArgc, char ** theArgv, const char * theArgumentSpec)
{
  // get the program name and path
  SetArgs(theArgc, theArgv);

  if (theArgumentSpec != NULL)
    // we got an argument spec - so process them
    Parse(theArgumentSpec);
  else {
    // we have no argument spec, delay parsing the arguments until later
    arg_values = NULL;
    arg_count  = 0;
  }
}


PArgList::~PArgList()
{
  free(optionCount);
  free(argumentList);
}


void PArgList::SetArgs(int argc, char ** argv)
{
  // save argv and and argc for later
  arg_values = argv;
  arg_count = argc;
  shift = 0;
}


void PArgList::Parse(const char * theArgumentSpec)
{
  char  c;
  char *p;
  int   l;

  // allocate and initialise storage
  argumentSpec = strdup (theArgumentSpec);
  l = strlen (argumentSpec);
  optionCount   = (PINDEX *) calloc (l, sizeof (PINDEX));
  argumentList = (char **) calloc (l, sizeof (char *));

  while (arg_count > 0 && arg_values[0][0] == '-') {
    while ((c = *++arg_values[0]) != 0) {
      if ((p = strchr (argumentSpec, c)) == NULL)
        UnknownOption (c);
      else {
        optionCount[p-argumentSpec]++;
        if (p[1] == ':') {
          if (*++(arg_values[0]))
            argumentList[p-argumentSpec] = arg_values[0];
          else {
            if (arg_count < 2) {
              optionCount[p-argumentSpec] = 0;
              MissingArgument (c);
            }
            else {
              --arg_count;
              argumentList [p-argumentSpec] = *++arg_values;
            }
          }
          break;
        }
      }
    }
    --arg_count;
    ++arg_values;
  }
}


PINDEX PArgList::GetOptionCount(char option) const
{
  char * p = strchr(argumentSpec, option);
  return (p == NULL ? 0 : optionCount[p-argumentSpec]);
}


PINDEX PArgList::GetOptionCount(const char * option) const
{
  // Future enhancement to have long option names
  return GetOptionCount(*option);
}


PString PArgList::GetOptionString(char option, const char * dflt) const
{
  char * p = strchr(argumentSpec, option);
  if (p != NULL)
    return argumentList[p-argumentSpec];

  if (dflt != NULL)
    return dflt;

  return PString();
}


PString PArgList::GetOptionString(const char * option, const char * dflt) const
{
  // Future enhancement to have long option names
  return GetOptionString(*option, dflt);
}


PString PArgList::GetParameter(PINDEX num) const
{
  if ((num+shift) < arg_count)
    return arg_values[num+shift];

  IllegalArgumentIndex(num+shift);
  return PString();
}


void PArgList::Shift(int sh) 
{
  if ((sh < 0) && (shift > 0))
    shift -= sh;
  else if ((sh > 0) && (shift < arg_count))
    shift += sh;
}


void PArgList::IllegalArgumentIndex(PINDEX idx) const
{
#ifdef _WINDLL
  idx = 1;
#else
  cerr << "attempt to access undefined argument at index "
       << idx << endl;
#endif
}
 

void PArgList::UnknownOption(char option) const
{
#ifdef _WINDLL
  option = ' ';
#else
  cerr << "unknown option \"" << option << "\"\n";
#endif
}


void PArgList::MissingArgument(char option) const
{
#ifdef _WINDLL
  option = ' ';
#else
  cerr << "option \"" << option << "\" requires argument\n";
#endif
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PTextApplication

#if defined(_PTEXTAPPLICATION)

PObject::Comparison PTextApplication::Compare(const PObject & obj) const
{
  return applicationName.Compare(((const PTextApplication &)obj).applicationName);
}


void PTextApplication::PreInitialise(int argc, char ** argv)
{
  terminationValue = 1;

  arguments.SetArgs(argc-1, argv+1);

  applicationFile = PFile(PString(argv[0]));
  applicationName = applicationFile.GetTitle().ToLower();
}


PString PTextApplication::GetEnvironment(const PString & varName) const
{
  char * env = getenv(varName);
  if (env == NULL)
    return PString();
  return env;
}


int PTextApplication::Termination()
{
  return terminationValue;
}


void PTextApplication::Terminate()
{
#ifdef _WINDLL
  FatalExit(Termination());
#else
  exit(Termination());
#endif
}


#endif


// End Of File ///////////////////////////////////////////////////////////////

/*
 * object.cxx
 *
 * Global object support.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: object.cxx,v $
 * Revision 1.47  2001/02/13 03:31:02  robertj
 * Added function to do heap validation.
 *
 * Revision 1.46  2001/02/07 04:47:49  robertj
 * Added changes for possible random crashes in multi DLL environment
 *   due to memory allocation wierdness, thanks Milan Dimitrijevic.
 *
 * Revision 1.45  2001/01/25 07:14:39  robertj
 * Fixed spurios memory leak message. Usual static global problem.
 *
 * Revision 1.44  2000/06/26 11:17:20  robertj
 * Nucleus++ port (incomplete).
 *
 * Revision 1.43  2000/01/05 00:29:12  robertj
 * Fixed alignment problems in memory checking debug functions.
 *
 * Revision 1.42  1999/11/01 00:17:20  robertj
 * Added override of new functions for MSVC memory check code.
 *
 * Revision 1.41  1999/08/22 13:38:39  robertj
 * Fixed termination hang up problem with memory check code under unix pthreads.
 *
 * Revision 1.40  1999/08/10 10:45:09  robertj
 * Added mutex in memory check detection code.
 *
 * Revision 1.39  1999/07/18 15:08:48  robertj
 * Fixed 64 bit compatibility
 *
 * Revision 1.38  1999/05/01 11:29:20  robertj
 * Alpha linux port changes.
 *
 * Revision 1.37  1999/03/09 10:30:17  robertj
 * Fixed ability to have PMEMORY_CHECK on/off on both debug/release versions.
 *
 * Revision 1.36  1999/02/22 10:48:14  robertj
 * Fixed delete operator prototypes for MSVC6 and GNU compatibility.
 *
 * Revision 1.35  1998/12/22 10:24:17  robertj
 * Fixed MSVC warnings caused by changes made in linux PPC support.
 *
 * Revision 1.34  1998/12/15 09:01:10  robertj
 * Fixed 8 byte alignment problem in memory leak check code for sparc.
 *
 * Revision 1.33  1998/11/30 05:33:00  robertj
 * Fixed duplicate debug stream class, ther can be only one.
 *
 * Revision 1.32  1998/11/03 03:11:53  robertj
 * Fixed memory leak question so correctly detects leaks and can be ^C'd.
 *
 * Revision 1.31  1998/11/03 00:55:31  robertj
 * Added allocation breakpoint variable.
 *
 * Revision 1.30  1998/10/15 07:48:56  robertj
 * Added hex dump to memory leak.
 * Added ability to ignore G++lib memory leaks.
 *
 * Revision 1.29  1998/10/15 01:53:35  robertj
 * GNU compatibility.
 *
 * Revision 1.28  1998/10/13 14:06:26  robertj
 * Complete rewrite of memory leak detection code.
 *
 * Revision 1.27  1998/09/23 06:22:22  robertj
 * Added open source copyright license.
 *
 * Revision 1.26  1998/05/30 13:27:02  robertj
 * Changed memory check code so global statics are not included in leak check.
 *
 * Revision 1.25  1997/07/08 13:07:07  robertj
 * DLL support.
 *
 * Revision 1.24  1997/02/09 03:45:28  robertj
 * Fixed unix/dos compatibility with include file.
 *
 * Revision 1.23  1997/02/05 11:54:12  robertj
 * Fixed problems with memory check and leak detection.
 *
 * Revision 1.22  1996/08/08 10:08:46  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.21  1996/07/15 10:35:11  robertj
 * Changed memory leak dump to use static class rather than atexit for better portability.
 *
 * Revision 1.20  1996/06/17 11:35:47  robertj
 * Fixed display of memory leak info, needed flush and use of cin as getchar() does not work with services.
 *
 * Revision 1.19  1996/05/09 12:19:29  robertj
 * Fixed up 64 bit integer class for Mac platform.
 * Fixed incorrect use of memcmp/strcmp return value.
 *
 * Revision 1.18  1996/03/26 00:55:20  robertj
 * Added keypress before dumping memory leaks.
 *
 * Revision 1.17  1996/01/28 02:50:27  robertj
 * Added missing bit shift operators to 64 bit integer class.
 * Added assert into all Compare functions to assure comparison between compatible objects.
 *
 * Revision 1.16  1996/01/23 13:15:52  robertj
 * Mac Metrowerks compiler support.
 *
 * Revision 1.15  1996/01/02 12:52:02  robertj
 * Mac OS compatibility changes.
 *
 * Revision 1.14  1995/11/21 11:51:54  robertj
 * Improved streams compatibility.
 *
 * Revision 1.12  1995/04/25 11:30:34  robertj
 * Fixed Borland compiler warnings.
 * Fixed function hiding ancestors virtual.
 *
 * Revision 1.11  1995/03/12 04:59:53  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.10  1995/02/19  04:19:21  robertj
 * Added dynamically linked command processing.
 *
 * Revision 1.9  1995/01/15  04:52:02  robertj
 * Mac compatibility.
 * Added memory stats function.
 *
// Revision 1.8  1995/01/09  12:38:07  robertj
// Changed variable names around during documentation run.
// Fixed smart pointer comparison.
// Fixed serialisation stuff.
//
// Revision 1.7  1995/01/07  04:39:45  robertj
// Redesigned font enumeration code and changed font styles.
//
// Revision 1.6  1995/01/04  10:57:08  robertj
// Changed for HPUX and GNU2.6.x
//
// Revision 1.5  1995/01/03  09:39:10  robertj
// Put standard malloc style memory allocation etc into memory check system.
//
// Revision 1.4  1994/12/21  11:43:29  robertj
// Added extra memory stats.
//
// Revision 1.3  1994/12/13  11:54:54  robertj
// Added some memory usage statistics.
//
// Revision 1.2  1994/12/12  10:08:32  robertj
// Renamed PWrapper to PSmartPointer..
//
// Revision 1.1  1994/10/30  12:02:15  robertj
// Initial revision
//
 */

#include <ptlib.h>
#include <ctype.h>
#ifdef _WIN32
#include <strstrea.h>
#include <ptlib/debstrm.h>
#include <crtdbg.h>
#elif defined(__NUCLEUS_PLUS__)
#include <ptlib/NucleusDebstrm.h>
#else
#include <strstream.h>
#include <signal.h>
#endif

void PAssertFunc(const char * file, int line, PStandardAssertMessage msg)
{
  static const char * const textmsg[PMaxStandardAssertMessage] = {
    NULL,
    "Out of memory",
    "Null pointer reference",
    "Invalid cast to non-descendant class",
    "Invalid array index",
    "Invalid array element",
    "Stack empty",
    "Unimplemented function",
    "Invalid parameter",
    "Operating System error",
    "File not open",
    "Unsupported feature",
    "Invalid or closed operating system window"
  };

  const char * theMsg;
  char msgbuf[20];
  if (msg < PMaxStandardAssertMessage)
    theMsg = textmsg[msg];
  else {
    sprintf(msgbuf, "Error code: %i", msg);
    theMsg = msgbuf;
  }
  PAssertFunc(file, line, theMsg);
}



#if PMEMORY_CHECK

#undef malloc
#undef realloc
#undef free


void * operator new(size_t nSize)
{
  return PMemoryHeap::Allocate(nSize, (const char *)NULL, 0, NULL);
}


void * operator new[](size_t nSize)
{
  return PMemoryHeap::Allocate(nSize, (const char *)NULL, 0, NULL);
}


void operator delete(void * ptr)
{
  PMemoryHeap::Deallocate(ptr, NULL);
}


void operator delete[](void * ptr)
{
  PMemoryHeap::Deallocate(ptr, NULL);
}


DWORD PMemoryHeap::allocationBreakpoint;
char PMemoryHeap::Header::GuardBytes[NumGuardBytes];


PMemoryHeap::Wrapper::Wrapper()
{
  // The following is done like this to get over brain dead compilers that cannot
  // guarentee that a static global is contructed before it is used.
  static PMemoryHeap real_instance;
  instance = &real_instance;
  if (instance->isDestroyed)
    return;

#if defined(_WIN32)
  EnterCriticalSection(&instance->mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_lock(&instance->mutex);
#endif
}


PMemoryHeap::Wrapper::~Wrapper()
{
  if (instance->isDestroyed)
    return;

#if defined(_WIN32)
  LeaveCriticalSection(&instance->mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_unlock(&instance->mutex);
#endif
}


PMemoryHeap::PMemoryHeap()
{
  isDestroyed = FALSE;

  listHead = NULL;
  listTail = NULL;

  allocationRequest = 1;
  firstRealObject = 0;
  flags = NoLeakPrint;

  allocFillChar = '\x5A';
  freeFillChar = '\xA5';

  currentMemoryUsage = 0;
  peakMemoryUsage = 0;
  currentObjects = 0;
  peakObjects = 0;
  totalObjects = 0;

  for (PINDEX i = 0; i < Header::NumGuardBytes; i++)
    Header::GuardBytes[i] = (i&1) == 0 ? '\x55' : '\xaa';

#if defined(_WIN32)
  InitializeCriticalSection(&mutex);
  static PDebugStream debug;
  leakDumpStream = &debug;
#else
#if defined(P_PTHREADS)
  pthread_mutex_init(&mutex, NULL);
#endif
  leakDumpStream = &cerr;
#endif
}


PMemoryHeap::~PMemoryHeap()
{
  isDestroyed = TRUE;

  if (leakDumpStream != NULL) {
    InternalDumpStatistics(*leakDumpStream);
    InternalDumpObjectsSince(firstRealObject, *leakDumpStream);
  }

#if defined(_WIN32)
  DeleteCriticalSection(&mutex);
  extern void PWaitOnExitConsoleWindow();
  PWaitOnExitConsoleWindow();
#elif defined(P_PTHREADS)
  pthread_mutex_destroy(&mutex);
#endif
}


void * PMemoryHeap::Allocate(size_t nSize, const char * file, int line, const char * className)
{
  Wrapper mem;
  return mem->InternalAllocate(nSize, file, line, className);
}


void * PMemoryHeap::Allocate(size_t count, size_t size, const char * file, int line)
{
  Wrapper mem;

  char oldFill = mem->allocFillChar;
  mem->allocFillChar = '\0';

  void * data = mem->InternalAllocate(count*size, file, line, NULL);

  mem->allocFillChar = oldFill;

  return data;
}


void * PMemoryHeap::InternalAllocate(size_t nSize, const char * file, int line, const char * className)
{
  if (isDestroyed)
    return malloc(nSize);

  Header * obj = (Header *)malloc(sizeof(Header) + nSize + sizeof(Header::GuardBytes));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  // Ignore all allocations made before main() is called. This is indicated
  // by PProcess::PreInitialise() clearing the NoLeakPrint flag. Why do we do
  // this? because the GNU compiler is broken in the way it does static global
  // C++ object construction and destruction.
  if (firstRealObject == 0 && (flags&NoLeakPrint) == 0)
    firstRealObject = allocationRequest;

  PAssert(allocationRequest != allocationBreakpoint, "Break on memory allocation.");

  currentMemoryUsage += nSize;
  if (currentMemoryUsage > peakMemoryUsage)
    peakMemoryUsage = currentMemoryUsage;

  currentObjects++;
  if (currentObjects > peakObjects)
    peakObjects = currentObjects;
  totalObjects++;

  char * data = (char *)&obj[1];

  obj->prev      = listTail;
  obj->next      = NULL;
  obj->size      = nSize;
  obj->fileName  = file;
  obj->line      = (WORD)line;
  obj->className = className;
  obj->request   = allocationRequest++;
  obj->flags     = flags;
  memcpy(obj->guard, obj->GuardBytes, sizeof(obj->guard));
  memset(data, allocFillChar, nSize);
  memcpy(&data[nSize], obj->GuardBytes, sizeof(obj->guard));

  if (listTail != NULL)
    listTail->next = obj;

  listTail = obj;

  if (listHead == NULL)
    listHead = obj;

  return data;
}


void * PMemoryHeap::Reallocate(void * ptr, size_t nSize, const char * file, int line)
{
  if (ptr == NULL)
    return Allocate(nSize, file, line, NULL);

  if (nSize == 0) {
    Deallocate(ptr, NULL);
    return NULL;
  }

  Wrapper mem;

  if (mem->isDestroyed)
    return realloc(ptr, nSize);

  if (mem->InternalValidate(ptr, NULL, mem->leakDumpStream) != Ok)
    return NULL;

  Header * obj = (Header *)realloc(((Header *)ptr)-1, sizeof(Header) + nSize + sizeof(obj->guard));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  PAssert(mem->allocationRequest != allocationBreakpoint, "Break on memory allocation.");

  mem->currentMemoryUsage -= obj->size;
  mem->currentMemoryUsage += nSize;
  if (mem->currentMemoryUsage > mem->peakMemoryUsage)
    mem->peakMemoryUsage = mem->currentMemoryUsage;

  char * data = (char *)&obj[1];
  memcpy(&data[nSize], obj->GuardBytes, sizeof(obj->guard));

  obj->size      = nSize;
  obj->fileName  = file;
  obj->line      = (WORD)line;
  obj->request   = mem->allocationRequest++;
  if (obj->prev != NULL)
    obj->prev->next = obj;
  else
    mem->listHead = obj;
  if (obj->next != NULL)
    obj->next->prev = obj;
  else
    mem->listTail = obj;

  return data;
}


void PMemoryHeap::Deallocate(void * ptr, const char * className)
{
  if (ptr == NULL)
    return;

  Wrapper mem;

  if (mem->isDestroyed) {
    free(ptr);
    return;
  }

  if (mem->InternalValidate(ptr, className, mem->leakDumpStream) != Ok) {
    free(ptr);
    return;
  }

  Header * obj = ((Header *)ptr)-1;
  if (obj->prev != NULL)
    obj->prev->next = obj->next;
  else
    mem->listHead = obj->next;
  if (obj->next != NULL)
    obj->next->prev = obj->prev;
  else
    mem->listTail = obj->prev;

  mem->currentMemoryUsage -= obj->size;
  mem->currentObjects--;

  memset(ptr, mem->freeFillChar, obj->size);  // Make use of freed data noticable
  free(obj);
}


PMemoryHeap::Validation PMemoryHeap::Validate(void * ptr,
                                              const char * className,
                                              ostream * error)
{
  Wrapper mem;
  return mem->InternalValidate(ptr, className, error);
}


PMemoryHeap::Validation PMemoryHeap::InternalValidate(void * ptr,
                                                      const char * className,
                                                      ostream * error)
{
  if (isDestroyed)
    return Bad;

  if (ptr == NULL)
    return Trashed;

  Header * obj = ((Header *)ptr)-1;

  if (memcmp(obj->guard, obj->GuardBytes, sizeof(obj->guard)) != 0) {
    if (error != NULL)
      *error << "Underrun at " << ptr << '[' << obj->size << "] #" << obj->request << endl;
    return Bad;
  }
  
  if (memcmp((char *)ptr+obj->size, obj->GuardBytes, sizeof(obj->guard)) != 0) {
    if (error != NULL)
      *error << "Overrun at " << ptr << '[' << obj->size << "] #" << obj->request << endl;
    return Bad;
  }
  
  if (obj->className != NULL && strcmp(obj->className, className) != 0) {
    if (error != NULL)
      *error << "PObject " << ptr << '[' << obj->size << "] #" << obj->request
             << " allocated as \"" << obj->className
             << "\" and should be \"" << className << "\"." << endl;
    return Bad;
  }

  Header * forward = obj;
  Header * backward = obj;
  while (forward->next != NULL && backward->prev != NULL) {
    forward = forward->next;
    backward = backward->prev;
  }

  if (forward != listTail && backward != listHead) {
    if (error != NULL)
      *error << "Block " << ptr << '[' << obj->size << "] #" << obj->request
             << " not in heap!" << endl;
    return Trashed;
  }

  return Ok;
}


BOOL PMemoryHeap::ValidateHeap(ostream * error)
{
  Wrapper mem;

  if (error == NULL)
    error = mem->leakDumpStream;

  Header * obj = mem->listHead;
  while (obj != NULL) {
    if (memcmp(obj->guard, obj->GuardBytes, sizeof(obj->guard)) != 0) {
      if (error != NULL)
        *error << "Underrun at " << (obj+1) << '[' << obj->size << "] #" << obj->request << endl;
      return FALSE;
    }
  
    if (memcmp((char *)(obj+1)+obj->size, obj->GuardBytes, sizeof(obj->guard)) != 0) {
      if (error != NULL)
        *error << "Overrun at " << (obj+1) << '[' << obj->size << "] #" << obj->request << endl;
      return FALSE;
    }

    obj = obj->next;
  }

#if defined(_WIN32) && defined(_DEBUG)
  if (!_CrtCheckMemory()) {
    if (error != NULL)
      *error << "Heap failed MSVCRT validation!" << endl;
    return FALSE;
  }
#endif
  if (error != NULL)
    *error << "Heap passed validation." << endl;
  return TRUE;
}


void PMemoryHeap::SetIgnoreAllocations(BOOL ignore)
{
  Wrapper mem;

  if (ignore)
    mem->flags |= NoLeakPrint;
  else
    mem->flags &= ~NoLeakPrint;
}


void PMemoryHeap::DumpStatistics()
{
  Wrapper mem;
  if (mem->leakDumpStream != NULL)
    mem->InternalDumpStatistics(*mem->leakDumpStream);
}


void PMemoryHeap::DumpStatistics(ostream & strm)
{
  Wrapper mem;
  mem->InternalDumpStatistics(strm);
}


void PMemoryHeap::InternalDumpStatistics(ostream & strm)
{
  strm << "\nCurrent memory usage: " << currentMemoryUsage << " bytes";
  if (currentMemoryUsage > 2048)
    strm << ", " << (currentMemoryUsage+1023)/1024 << "kb";
  if (currentMemoryUsage > 2097152)
    strm << ", " << (currentMemoryUsage+1048575)/1048576 << "Mb";

  strm << ".\nCurrent objects count: " << currentObjects
       << "\nPeak memory usage: " << peakMemoryUsage << " bytes";
  if (peakMemoryUsage > 2048)
    strm << ", " << (peakMemoryUsage+1023)/1024 << "kb";
  if (peakMemoryUsage > 2097152)
    strm << ", " << (peakMemoryUsage+1048575)/1048576 << "Mb";

  strm << ".\nPeak objects created: " << peakObjects
       << "\nTotal objects created: " << totalObjects
       << '\n' << endl;
}


DWORD PMemoryHeap::GetAllocationRequest()
{
  Wrapper mem;
  return mem->allocationRequest;
}


void PMemoryHeap::SetAllocationBreakpoint(DWORD point)
{
  allocationBreakpoint = point;
}


void PMemoryHeap::DumpObjectsSince(DWORD objectNumber)
{
  Wrapper mem;
  if (mem->leakDumpStream != NULL)
    mem->InternalDumpObjectsSince(objectNumber, *mem->leakDumpStream);
}


void PMemoryHeap::DumpObjectsSince(DWORD objectNumber, ostream & strm)
{
  Wrapper mem;
  mem->InternalDumpObjectsSince(objectNumber, strm);
}


void PMemoryHeap::InternalDumpObjectsSince(DWORD objectNumber, ostream & strm)
{
  BOOL first = TRUE;
  for (Header * obj = listHead; obj != NULL; obj = obj->next) {
    if (obj->request < objectNumber || (obj->flags&NoLeakPrint) != 0)
      continue;

    if (first && isDestroyed) {
      *leakDumpStream << "\nMemory leaks detected, press Enter to display . . ." << flush;
#if !defined(_WIN32)
#if defined(P_PTHREADS)
      sigset_t blockedSignals;
      sigemptyset(&blockedSignals);
      sigaddset(&blockedSignals, SIGINT);
      sigaddset(&blockedSignals, SIGQUIT);
      sigaddset(&blockedSignals, SIGTERM);
      pthread_sigmask(SIG_UNBLOCK, &blockedSignals, NULL);
#endif
      ::signal(SIGINT, SIG_DFL);
      ::signal(SIGQUIT, SIG_DFL);
      ::signal(SIGTERM, SIG_DFL);
      cin.get();
#endif
      first = FALSE;
    }

    BYTE * data = (BYTE *)&obj[1];

    if (obj->fileName != NULL)
      strm << obj->fileName << '(' << obj->line << ") : ";

    strm << '#' << obj->request << ' ' << (void *)data << " [" << obj->size << "] ";

    if (obj->className != NULL)
      strm << '"' << obj->className << "\" ";

    strm << '\n' << hex << setfill('0') << PBYTEArray(data, PMIN(16, obj->size), FALSE)
                 << dec << setfill(' ') << endl;
  }
}


#else // PMEMORY_CHECK

void * operator new[](size_t nSize)
{
  return malloc(nSize);
}

void operator delete[](void * ptr)
{
  free(ptr);
}

#endif // PMEMORY_CHECK


const char * PObject::GetClass(unsigned) const
{
  return Class();
}


BOOL PObject::IsClass(const char * clsName) const
{
  return strcmp(clsName, Class()) == 0;
}


BOOL PObject::IsDescendant(const char * clsName) const
{
  return strcmp(clsName, Class()) == 0;
}


PObject::Comparison PObject::CompareObjectMemoryDirect(const PObject&obj) const
{
  int retval = memcmp(this, &obj, sizeof(PObject));
  if (retval < 0)
    return LessThan;
  if (retval > 0)
    return GreaterThan;
  return EqualTo;
}


PObject * PObject::Clone() const
{
  PAssertAlways(PUnimplementedFunction);
  return NULL;
}


PObject::Comparison PObject::Compare(const PObject & obj) const
{
  return (Comparison)CompareObjectMemoryDirect(obj);
}


void PObject::PrintOn(ostream & strm) const
{
  strm << GetClass();
}


void PObject::ReadFrom(istream &)
{
}


PINDEX PObject::PreSerialise(PSerialiser &)
{
  return 0;
}


void PObject::Serialise(PSerialiser &)
{
}


void PObject::UnSerialise(PUnSerialiser &)
{
}


PINDEX PObject::HashFunction() const
{
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Serialisation support

PSerialRegistration * PSerialRegistration::creatorHashTable[
                                           PSerialRegistration::HashTableSize];

PINDEX PSerialRegistration::HashFunction(const char * className)
{
  PINDEX hash = (BYTE)className[0];
  if (className[0] != '\0') {
    hash += (BYTE)className[1];
    if (className[1] != '\0')
      hash += (BYTE)className[2];
  }
  return hash%HashTableSize;
}


PSerialRegistration::PSerialRegistration(const char * clsNam,
                                                          CreatorFunction func)
  : className(clsNam), creator(func)
{
  clash = NULL;

  PINDEX hash = HashFunction(className);
  if (creatorHashTable[hash] != NULL)
    creatorHashTable[hash]->clash = this;
  creatorHashTable[hash] = this;
}


PSerialRegistration::CreatorFunction
                           PSerialRegistration::GetCreator(const char * clsNam)
{
  PINDEX hash = HashFunction(clsNam);
  PSerialRegistration * reg = creatorHashTable[hash];
  while (reg != NULL) {
    if (strcmp(reg->className, clsNam) == 0)
      return reg->creator;
  }
  return NULL;
}


PSerialiser::PSerialiser(ostream & strm)
  : stream(strm)
{
}


PSerialiser & PSerialiser::operator<<(PObject & obj)
{
  obj.Serialise(*this);
  return *this;
}


PTextSerialiser::PTextSerialiser(ostream & strm, PObject & data)
  : PSerialiser(strm)
{
  data.Serialise(*this);
}


PSerialiser & PTextSerialiser::operator<<(char)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(unsigned char)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(signed char)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(short)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(unsigned short)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(int)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(unsigned int)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(long)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(unsigned long)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(float)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(double)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(long double)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(const char *)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(const unsigned char *)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(const signed char *)
  { return *this; }

PSerialiser & PTextSerialiser::operator<<(PObject & obj)
  { return PSerialiser::operator<<(obj); }


PBinarySerialiser::PBinarySerialiser(ostream & strm, PObject & data)
  : PSerialiser(strm)
{
  classesUsed = new PSortedStringList;
  data.PreSerialise(*this);
  stream << *classesUsed;
  data.Serialise(*this);
}


PBinarySerialiser::~PBinarySerialiser()
{
  delete classesUsed;
}


PSerialiser & PBinarySerialiser::operator<<(char)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(unsigned char)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(signed char)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(short)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(unsigned short)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(int)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(unsigned int)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(long)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(unsigned long)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(float)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(double)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(long double)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(const char *)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(const unsigned char *)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(const signed char *)
  { return *this; }

PSerialiser & PBinarySerialiser::operator<<(PObject & obj)
  { return PSerialiser::operator<<(obj); }


PUnSerialiser::PUnSerialiser(istream & strm)
  : stream(strm)
{
}


PTextUnSerialiser::PTextUnSerialiser(istream & strm)
  : PUnSerialiser(strm)
{
}


PUnSerialiser & PTextUnSerialiser::operator>>(char &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(unsigned char &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(signed char &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(short &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(unsigned short &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(int &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(unsigned int &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(long &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(unsigned long &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(float &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(double &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(long double &)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(char *)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(unsigned char *)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(signed char *)
  { return *this; }

PUnSerialiser & PTextUnSerialiser::operator>>(PObject &)
  { return *this; }


PBinaryUnSerialiser::PBinaryUnSerialiser(istream & strm)
  : PUnSerialiser(strm)
{
  classesUsed = new PStringArray;
}


PBinaryUnSerialiser::~PBinaryUnSerialiser()
{
  delete classesUsed;
}


PUnSerialiser & PBinaryUnSerialiser::operator>>(char &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(unsigned char &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(signed char &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(short &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(unsigned short &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(int &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(unsigned int &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(long &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(unsigned long &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(float &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(double &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(long double &)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(char *)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(unsigned char *)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(signed char *)
  { return *this; }

PUnSerialiser & PBinaryUnSerialiser::operator>>(PObject &)
  { return *this; }


///////////////////////////////////////////////////////////////////////////////
// General reference counting support

PSmartPointer::PSmartPointer(const PSmartPointer & ptr)
{
  object = ptr.object;
  if (object != NULL)
    ++object->referenceCount;
}


PSmartPointer & PSmartPointer::operator=(const PSmartPointer & ptr)
{
  if (object == ptr.object)
    return *this;

  if (object != NULL && --object->referenceCount == 0)
    delete object;

  object = ptr.object;
  if (object != NULL)
    ++object->referenceCount;

  return *this;
}


PSmartPointer::~PSmartPointer()
{
  if (object != NULL && --object->referenceCount == 0)
    delete object;
}


PObject::Comparison PSmartPointer::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PSmartPointer::Class()), PInvalidCast);
  PSmartObject * other = ((const PSmartPointer &)obj).object;
  if (object == other)
    return EqualTo;
  return object < other ? LessThan : GreaterThan;
}


///////////////////////////////////////////////////////////////////////////////
// Large integer support

#ifdef P_NEEDS_INT64

void PInt64__::Add(const PInt64__ & v)
{
  unsigned long old = low;
  high += v.high;
  low += v.low;
  if (low < old)
    high++;
}


void PInt64__::Sub(const PInt64__ & v)
{
  unsigned long old = low;
  high -= v.high;
  low -= v.low;
  if (low > old)
    high--;
}


void PInt64__::Mul(const PInt64__ & v)
{
  DWORD p1 = (low&0xffff)*(v.low&0xffff);
  DWORD p2 = (low >> 16)*(v.low >> 16);
  DWORD p3 = (high&0xffff)*(v.high&0xffff);
  DWORD p4 = (high >> 16)*(v.high >> 16);
  low = p1 + (p2 << 16);
  high = (p2 >> 16) + p3 + (p4 << 16);
}


void PInt64__::Div(const PInt64__ & v)
{
  long double dividend = high;
  dividend *=  4294967296.0;
  dividend += low;
  long double divisor = high;
  divisor *=  4294967296.0;
  divisor += low;
  long double quotient = dividend/divisor;
  low = quotient;
  high = quotient/4294967296.0;
}


void PInt64__::Mod(const PInt64__ & v)
{
  PInt64__ t = *this;
  t.Div(v);
  t.Mul(t);
  Sub(t);
}


void PInt64__::ShiftLeft(int bits)
{
  if (bits >= 32) {
    high = low << (bits - 32);
    low = 0;
  }
  else {
    high <<= bits;
    high |= low >> (32 - bits);
    low <<= bits;
  }
}


void PInt64__::ShiftRight(int bits)
{
  if (bits >= 32) {
    low = high >> (bits - 32);
    high = 0;
  }
  else {
    low >>= bits;
    low |= high << (32 - bits);
    high >>= bits;
  }
}


BOOL PInt64::Lt(const PInt64 & v) const
{
  if ((long)high < (long)v.high)
    return TRUE;
  if ((long)high > (long)v.high)
    return FALSE;
  if ((long)high < 0)
    return (long)low > (long)v.low;
  return (long)low < (long)v.low;
}


BOOL PInt64::Gt(const PInt64 & v) const
{
  if ((long)high > (long)v.high)
    return TRUE;
  if ((long)high < (long)v.high)
    return FALSE;
  if ((long)high < 0)
    return (long)low < (long)v.low;
  return (long)low > (long)v.low;
}


BOOL PUInt64::Lt(const PUInt64 & v) const
{
  if (high < v.high)
    return TRUE;
  if (high > v.high)
    return FALSE;
  return low < high;
}


BOOL PUInt64::Gt(const PUInt64 & v) const
{
  if (high > v.high)
    return TRUE;
  if (high < v.high)
    return FALSE;
  return low > high;
}


static void Out64(ostream & stream, PUInt64 num)
{
  char buf[25];
  char * p = &buf[sizeof(buf)];
  *--p = '\0';

  switch (stream.flags()&ios::basefield) {
    case ios::oct :
      while (num != 0) {
        *--p = (num&7) + '0';
        num >>= 3;
      }
      break;

    case ios::hex :
      while (num != 0) {
        *--p = (num&15) + '0';
        if (*p > '9')
          *p += 7;
        num >>= 4;
      }
      break;

    default :
      while (num != 0) {
        *--p = num%10 + '0';
        num /= 10;
      }
  }

  if (*p == '\0')
    *--p = '0';

  stream << p;
}


ostream & operator<<(ostream & stream, const PInt64 & v)
{
  if (v >= 0)
    Out64(stream, v);
  else {
    int w = stream.width();
    stream.put('-');
    if (w > 0)
      stream.width(w-1);
    Out64(stream, -v);
  }

  return stream;
}


ostream & operator<<(ostream & stream, const PUInt64 & v)
{
  Out64(stream, v);
  return stream;
}


static PUInt64 Inp64(istream & stream)
{
  int base;
  switch (stream.flags()&ios::basefield) {
    case ios::oct :
      base = 8;
      break;
    case ios::hex :
      base = 16;
      break;
    default :
      base = 10;
  }

  if (isspace(stream.peek()))
    stream.get();

  PInt64 num = 0;
  while (isxdigit(stream.peek())) {
    int c = stream.get() - '0';
    if (c > 9)
      c -= 7;
    if (c > 9)
      c -= 32;
    num = num*base + c;
  }

  return num;
}


istream & operator>>(istream & stream, PInt64 & v)
{
  if (isspace(stream.peek()))
    stream.get();

  switch (stream.peek()) {
    case '-' :
      stream.ignore();
      v = -(PInt64)Inp64(stream);
      break;
    case '+' :
      stream.ignore();
    default :
      v = (PInt64)Inp64(stream);
  }

  return stream;
}


istream & operator>>(istream & stream, PUInt64 & v)
{
  v = Inp64(stream);
  return stream;
}


#endif


// End Of File ///////////////////////////////////////////////////////////////

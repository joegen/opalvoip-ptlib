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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "pfactory.h"
#endif // __GNUC__

#include <ptlib.h>
#include <ptlib/pfactory.h>
#include <ptlib/pprocess.h>
#include <sstream>
#include <fstream>
#include <ctype.h>
#include <limits>
#ifdef _WIN32
#include <ptlib/msos/ptlib/debstrm.h>
#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#include <crtdbg.h>
#endif
#elif defined(__NUCLEUS_PLUS__)
#include <ptlib/NucleusDebstrm.h>
#else
#include <signal.h>
#endif


PFactoryBase::FactoryMap & PFactoryBase::GetFactories()
{
  static FactoryMap factories;
  return factories;
}


PFactoryBase::FactoryMap::~FactoryMap()
{
  FactoryMap::iterator entry;
  for (entry = begin(); entry != end(); ++entry){
    delete entry->second;
    entry->second = NULL;
  }  
}



void PFactoryBase::FactoryMap::DestroySingletons()
{
  Wait();
  for (iterator it = begin(); it != end(); ++it)
    it->second->DestroySingletons();
  Signal();
}


PFactoryBase & PFactoryBase::InternalGetFactory(const std::string & className, PFactoryBase * (*createFactory)())
{
  FactoryMap & factories = GetFactories();
  PWaitAndSignal mutex(factories);

  FactoryMap::const_iterator entry = factories.find(className);
  if (entry != factories.end()) {
    PAssert(entry->second != NULL, "Factory map returned NULL for existing key");
    return *entry->second;
  }

  PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
  PFactoryBase * factory = createFactory();
  factories[className] = factory;

  return *factory;
}


#if !P_USE_ASSERTS

#define PAssertFunc(a, b, c, d) { }

#else

static PCriticalSection s_AssertMutex;
extern void PPlatformAssertFunc(const char * msg, char defaultAction);
extern void PPlatformWalkStack(ostream & strm, PThreadIdentifier id, PUniqueThreadIdentifier uid, unsigned framesToSkip);

#if PTRACING
  void PTrace::WalkStack(ostream & strm, PThreadIdentifier id, PUniqueThreadIdentifier uid)
  {
    s_AssertMutex.Wait();
    PPlatformWalkStack(strm, id, uid, 1); // 1 means skip reporting PTrace::WalkStack
    s_AssertMutex.Signal();
  }
#endif // PTRACING


bool PAssertWalksStack = true;

static void InternalAssertFunc(const char * file, int line, const char * className, const char * msg)
{
#if defined(_WIN32)
  DWORD errorCode = GetLastError();
#else
  int errorCode = errno;
#endif

  const char * env;
#if P_EXCEPTIONS
  //Throw a runtime exception if the environment variable is set
  env = ::getenv("PTLIB_ASSERT_EXCEPTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_EXCEPTION");
  if (env != NULL) {
    throw std::runtime_error(msg);
    return;
  }
#endif

  PWaitAndSignal lock(s_AssertMutex);
  static bool s_RecursiveAssert = false;
  if (s_RecursiveAssert)
    return;

  s_RecursiveAssert = true;

  std::string str;
  {
    ostringstream strm;
    strm << "Assertion fail: ";
    if (msg != NULL)
      strm << msg << ", ";
    strm << "file " << file << ", line " << line;
    if (className != NULL)
      strm << ", class " << className;
    if (errorCode != 0)
      strm << ", error=" << errorCode;
    strm << ", when=" << PTime().AsString(PTime::LoggingFormat);
    if (PAssertWalksStack)
      PPlatformWalkStack(strm, PNullThreadIdentifier, 0, 2); // 2 means skip reporting InternalAssertFunc & PAssertFunc
    strm << ends;
    str = strm.str();
  }

  env = ::getenv("PTLIB_ASSERT_ACTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_ACTION");

  PPlatformAssertFunc(str.c_str(), env != NULL ? *env : '\0');

  s_RecursiveAssert = false;
}


bool PAssertFunc(const char * file,
                 int line,
                 const char * className,
                 PStandardAssertMessage msg)
{
  if (msg == POutOfMemory) {
    // Special case, do not use ostrstream in other PAssertFunc if have
    // a memory out situation as that would probably also fail!
    static const char fmt[] = "Out of memory at file %.100s, line %u, class %.30s";
    char msgbuf[sizeof(fmt)+100+10+30];
    sprintf(msgbuf, fmt, file, line, className);
    PPlatformAssertFunc(msgbuf, 'A');
    return false;
  }

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

  if (msg < PMaxStandardAssertMessage)
    InternalAssertFunc(file, line, className, textmsg[msg]);
  else {
    char msgbuf[20];
    sprintf(msgbuf, "Assertion %i", msg);
    InternalAssertFunc(file, line, className, msgbuf);
  }
  return false;
}


bool PAssertFunc(const char * file, int line, const char * className, const char * msg)
{
  // Done this way so WalkStack removes the correct number of irrelevant frames
  InternalAssertFunc(file, line, className, msg);
  return false;
}


#endif // !P_USE_ASSERTS

PObject::Comparison PObject::CompareObjectMemoryDirect(const PObject & obj) const
{
  return InternalCompareObjectMemoryDirect(this, &obj, sizeof(obj));
}


PObject::Comparison PObject::InternalCompareObjectMemoryDirect(const PObject * obj1,
                                                               const PObject * obj2,
                                                               PINDEX size)
{
  if (obj2 == NULL)
    return PObject::LessThan;
  if (obj1 == NULL)
    return PObject::GreaterThan;
  int retval = memcmp((const void *)obj1, (const void *)obj2, size);
  if (retval < 0)
    return PObject::LessThan;
  if (retval > 0)
    return PObject::GreaterThan;
  return PObject::EqualTo;
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


PINDEX PObject::HashFunction() const
{
  return 0;
}


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

  if ((object != NULL) && (--object->referenceCount == 0))
      delete object;

  // This reduces, but does not close, the multi-threading window for failure
  if (ptr.object != NULL && PAssert(++ptr.object->referenceCount > 1, "Multi-thread failure in PSmartPointer"))
    object = ptr.object;
  else
    object = NULL;

  return *this;
}


PSmartPointer::~PSmartPointer()
{
  if ((object != NULL) && (--object->referenceCount == 0))
    delete object;
}


PObject::Comparison PSmartPointer::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PSmartPointer), PInvalidCast);
  PSmartObject * other = ((const PSmartPointer &)obj).object;
  if (object == other)
    return EqualTo;
  return object < other ? LessThan : GreaterThan;
}



//////////////////////////////////////////////////////////////////////////////////////////

#if PMEMORY_CHECK

#undef malloc
#undef calloc
#undef realloc
#undef free

#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void * operator new(size_t nSize) throw (std::bad_alloc)
#else
void * operator new(size_t nSize)
#endif
{
  return PMemoryHeap::Allocate(nSize, (const char *)NULL, 0, NULL);
}


#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void * operator new[](size_t nSize) throw (std::bad_alloc)
#else
void * operator new[](size_t nSize)
#endif
{
  return PMemoryHeap::Allocate(nSize, (const char *)NULL, 0, NULL);
}


#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void operator delete(void * ptr) throw()
#else
void operator delete(void * ptr)
#endif
{
  PMemoryHeap::Deallocate(ptr, NULL);
}


#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void operator delete[](void * ptr) throw()
#else
void operator delete[](void * ptr)
#endif
{
  PMemoryHeap::Deallocate(ptr, NULL);
}


char PMemoryHeap::Header::GuardBytes[NumGuardBytes];
static const size_t MaxMemoryDumBytes = 16;
static void * DeletedPtr;
static void * UninitialisedPtr;
static void * GuardedPtr;


PMemoryHeap & PMemoryHeap::GetInstance()
{
  // The following is done like this to get over brain dead compilers that cannot
  // guarentee that a static global is contructed before it is used.
  static PMemoryHeap real_instance;
  return real_instance;
}


PMemoryHeap::PMemoryHeap()
  : m_listHead(NULL)
  , m_listTail(NULL)
  , m_allocationRequest(1)
  , m_allocationBreakpoint(0)
  , m_firstRealObject(0)
  , m_flags(NoLeakPrint)
  , m_allocFillChar('\xCD') // Microsoft debug heap values
  , m_freeFillChar('\xDD')
  , m_currentMemoryUsage(0)
  , m_peakMemoryUsage(0)
  , m_currentObjects(0)
  , m_peakObjects(0)
  , m_totalObjects(0)
  , m_leakDumpStream(NULL)
{
  const char * env = getenv("PTLIB_MEMORY_CHECK");
  switch (atoi(env != NULL ? env : "1")) {
    case 0 :
      m_state = e_Disabled;
      break;

    case 2 :
      m_state = e_TrackOnly;
      break;

    default :
      m_state = e_Active;
      break;
  }

  for (PINDEX i = 0; i < Header::NumGuardBytes; i++)
    Header::GuardBytes[i] = '\xFD';

  memset(&DeletedPtr, m_freeFillChar, sizeof(DeletedPtr));
  memset(&UninitialisedPtr, m_allocFillChar, sizeof(UninitialisedPtr));
  memset(&GuardedPtr, '\xFD', sizeof(GuardedPtr));

#if defined(_WIN32)
  InitializeCriticalSection(&m_mutex);
  static PDebugStream debug;
  m_leakDumpStream = &debug;
#else
#if defined(P_PTHREADS)
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
  pthread_mutex_t recursiveMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  m_mutex = recursiveMutex;
#else
 pthread_mutex_init(&m_mutex, NULL);
#endif
#elif defined(P_VXWORKS)
  m_mutex = semMCreate(SEM_Q_FIFO);
#endif
  m_leakDumpStream = &cerr;
#endif
}


PMemoryHeap::~PMemoryHeap()
{
  m_state = e_Destroyed;

  if (m_leakDumpStream != NULL) {
    *m_leakDumpStream << "Final memory statistics:\n";
    InternalDumpStatistics(*m_leakDumpStream);
    InternalDumpObjectsSince(m_firstRealObject, *m_leakDumpStream);
  }

#if defined(_WIN32)
  DeleteCriticalSection(&m_mutex);
  PProcess::Current().WaitOnExitConsoleWindow();
#elif defined(P_PTHREADS)
  pthread_mutex_destroy(&m_mutex);
#elif defined(P_VXWORKS)
  semDelete((SEM_ID)m_mutex);
#endif
}


void PMemoryHeap::Lock()
{
#if defined(_WIN32)
  EnterCriticalSection(&m_mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_lock(&m_mutex);
#elif defined(P_VXWORKS)
  semTake((SEM_ID)m_mutex, WAIT_FOREVER);
#endif
}


void PMemoryHeap::Unlock()
{
#if defined(_WIN32)
  LeaveCriticalSection(&m_mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_unlock(&m_mutex);
#elif defined(P_VXWORKS)
  semGive((SEM_ID)m_mutex);
#endif
}


void * PMemoryHeap::Allocate(size_t nSize, const char * file, int line, const char * className)
{
  return GetInstance().InternalAllocate(nSize, file, line, className, false);
}


void * PMemoryHeap::Allocate(size_t count, size_t size, const char * file, int line)
{
  return GetInstance().InternalAllocate(count*size, file, line, NULL, true);
}


void * PMemoryHeap::InternalAllocate(size_t nSize, const char * file, int line, const char * className, bool zeroFill)
{
  if (m_state <= e_Disabled)
    return malloc(nSize);

  Header * obj = (Header *)malloc(sizeof(Header) + nSize + sizeof(Header::GuardBytes));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  Lock();

  if (m_allocationBreakpoint != 0 && m_allocationRequest == m_allocationBreakpoint)
    PBreakToDebugger();

  // Ignore all allocations made before main() is called. This is indicated
  // by PProcess::PreInitialise() clearing the NoLeakPrint flag. Why do we do
  // this? because the GNU compiler is broken in the way it does static global
  // C++ object construction and destruction.
  if (m_firstRealObject == 0 && (m_flags&NoLeakPrint) == 0) {
    if (m_leakDumpStream != NULL)
      *m_leakDumpStream << "PTLib memory checking activated." << endl;
    m_firstRealObject = m_allocationRequest;
  }

  m_currentMemoryUsage += nSize;
  if (m_currentMemoryUsage > m_peakMemoryUsage)
    m_peakMemoryUsage = m_currentMemoryUsage;

  m_currentObjects++;
  if (m_currentObjects > m_peakObjects)
    m_peakObjects = m_currentObjects;
  m_totalObjects++;

  obj->m_request = m_allocationRequest++;

  obj->m_prev = m_listTail;
  if (m_listTail != NULL)
    m_listTail->m_next = obj;
  m_listTail = obj;
  if (m_listHead == NULL)
    m_listHead = obj;
  obj->m_next = NULL;

  Unlock();

  obj->m_size      = nSize;
  obj->m_fileName  = file;
  obj->m_line      = (WORD)line;
  obj->m_threadId  = PThread::GetCurrentThreadId();
  obj->m_className = className;
  obj->m_flags     = m_flags;

  char * data = (char *)&obj[1];

  if (m_state == e_Active) {
    memcpy(obj->m_guard, obj->GuardBytes, sizeof(obj->m_guard));
    memset(data, zeroFill ? '\0' : m_allocFillChar, nSize);
    memcpy(&data[nSize], obj->GuardBytes, sizeof(obj->m_guard));
  }
  else if (zeroFill)
    memset(data, '\0', nSize);

  return data;
}


void * PMemoryHeap::Reallocate(void * ptr, size_t nSize, const char * file, int line)
{
  return GetInstance().InternalReallocate(ptr, nSize, file, line);
}


void * PMemoryHeap::InternalReallocate(void * ptr, size_t nSize, const char * file, int line)
{
  if (m_state <= e_Disabled)
    return realloc(ptr, nSize);

  if (ptr == NULL)
    return Allocate(nSize, file, line, NULL);

  if (nSize == 0) {
    Deallocate(ptr, NULL);
    return NULL;
  }

  if (InternalValidate(ptr, NULL, m_leakDumpStream) != Ok)
    return NULL;

  Header * obj = (Header *)realloc(((Header *)ptr)-1, sizeof(Header) + nSize + sizeof(obj->m_guard));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  Lock();

  if (m_allocationBreakpoint != 0 && m_allocationRequest == m_allocationBreakpoint)
    PBreakToDebugger();

  m_currentMemoryUsage -= obj->m_size;
  m_currentMemoryUsage += nSize;
  if (m_currentMemoryUsage > m_peakMemoryUsage)
    m_peakMemoryUsage = m_currentMemoryUsage;

  obj->m_request = m_allocationRequest++;

  if (obj->m_prev != NULL)
    obj->m_prev->m_next = obj;
  else
    m_listHead = obj;
  if (obj->m_next != NULL)
    obj->m_next->m_prev = obj;
  else
    m_listTail = obj;

  Unlock();

  obj->m_size     = nSize;
  obj->m_fileName = file;
  obj->m_line     = (WORD)line;

  char * data = (char *)&obj[1];

  if (m_state == e_Active)
    memcpy(&data[nSize], obj->GuardBytes, sizeof(obj->m_guard));

  return data;
}


void PMemoryHeap::Deallocate(void * ptr, const char * className)
{
  GetInstance().InternalDeallocate(ptr, className);
}


void PMemoryHeap::InternalDeallocate(void * ptr, const char * className)
{
  if (m_state <= e_Disabled)
    return free(ptr);

  if (ptr == NULL)
    return;

  Header * obj = ((Header *)ptr)-1;

  switch (InternalValidate(ptr, className, m_leakDumpStream)) {
    case Trashed :
      free(obj);  // Try and free out extended version, and continue
      return;

    case Inactive :
    case Bad :
      free(ptr);  // Try and free it as if we didn't allocate it, and continue
      return;

    default :
      break;
  }

  Lock();

  if (obj->m_prev != NULL)
    obj->m_prev->m_next = obj->m_next;
  else
    m_listHead = obj->m_next;
  if (obj->m_next != NULL)
    obj->m_next->m_prev = obj->m_prev;
  else
    m_listTail = obj->m_prev;

  m_currentMemoryUsage -= obj->m_size;
  m_currentObjects--;

  Unlock();

  if (m_state == e_Active)
    memset(ptr, m_freeFillChar, obj->m_size);  // Make use of freed data noticable

  free(obj);
}


PMemoryHeap::Validation PMemoryHeap::Validate(const void * ptr,
                                              const char * className,
                                              ostream * error)
{
  return GetInstance().InternalValidate(ptr, className, error);
}


#ifdef PTRACING
  #define PMEMORY_VALIDATE_ERROR(arg) if (error != NULL) { *error << arg << '\n'; PTrace::WalkStack(*error); error->flush(); }
#else
  #define PMEMORY_VALIDATE_ERROR(arg) if (error != NULL) *error << arg << endl
#endif

PMemoryHeap::Validation PMemoryHeap::InternalValidate(const void * ptr,
                                                      const char * className,
                                                      ostream * error)
{
  if (m_state <= e_Disabled)
    return Inactive;

  if (ptr == NULL)
    return Bad;

  Header * obj = ((Header *)ptr) - 1;
  Header * link = obj;

  if (m_state == e_Active) {
    Lock();

    unsigned count = m_currentObjects;
    Header * link = m_listTail;
    while (link != NULL && link != obj && count-- > 0) {
      if (link->m_prev == DeletedPtr || link->m_prev == UninitialisedPtr || link->m_prev == GuardedPtr) {
        PMEMORY_VALIDATE_ERROR("Block " << ptr << " trashed!");
        Unlock();
        return Trashed;
      }
      link = link->m_prev;
    }

    Unlock();
  }

  if (link != obj) {
    PMEMORY_VALIDATE_ERROR("Block " << ptr << " not in heap!");
    return Bad;
  }

  if (m_state == e_Active) {
    if (memcmp(obj->m_guard, obj->GuardBytes, sizeof(obj->m_guard)) != 0) {
      PMEMORY_VALIDATE_ERROR("Underrun at " << ptr << '[' << obj->m_size << "] #" << obj->m_request);
      return Corrupt;
    }

    if (memcmp((char *)ptr + obj->m_size, obj->GuardBytes, sizeof(obj->m_guard)) != 0) {
      PMEMORY_VALIDATE_ERROR("Overrun at " << ptr << '[' << obj->m_size << "] #" << obj->m_request);
      return Corrupt;
    }
  }

  if (!(className == NULL && obj->m_className == NULL) &&
       (className == NULL || obj->m_className == NULL ||
       (className != obj->m_className && strcmp(obj->m_className, className) != 0))) {
    PMEMORY_VALIDATE_ERROR("PObject " << ptr << '[' << obj->m_size << "] #" << obj->m_request
                           << " allocated as \"" << (obj->m_className != NULL ? obj->m_className : "<NULL>")
                           << "\" and should be \"" << (className != NULL ? className : "<NULL>") << "\".");
    return Corrupt;
  }

  return Ok;
}


PBoolean PMemoryHeap::ValidateHeap(ostream * error)
{
  return GetInstance().InternalValidateHeap(error);
}


bool PMemoryHeap::InternalValidateHeap(ostream * error)
{
  if (error == NULL) {
    error = m_leakDumpStream;
    if (error == NULL)
      return false;
  }

  if (m_state == e_Active) {
    Lock();

    Header * obj = m_listHead;
    while (obj != NULL) {
      if (memcmp(obj->m_guard, obj->GuardBytes, sizeof(obj->m_guard)) != 0) {
        if (error != NULL)
          *error << "Underrun at " << (obj + 1) << '[' << obj->m_size << "] #" << obj->m_request << endl;
        Unlock();
        return false;
      }

      if (memcmp((char *)(obj + 1) + obj->m_size, obj->GuardBytes, sizeof(obj->m_guard)) != 0) {
        if (error != NULL)
          *error << "Overrun at " << (obj + 1) << '[' << obj->m_size << "] #" << obj->m_request << endl;
        Unlock();
        return false;
      }

      obj = obj->m_next;
    }

    Unlock();
  }

#if defined(_WIN32) && defined(_DEBUG)
  if (!_CrtCheckMemory()) {
    if (error != NULL)
      *error << "Heap failed MSVCRT validation!" << endl;
    return false;
  }
#endif
  if (error != NULL)
    *error << "Heap passed validation." << endl;
  return true;
}


PBoolean PMemoryHeap::SetIgnoreAllocations(PBoolean ignore)
{
  PMemoryHeap & mem = GetInstance();

  PBoolean ignoreAllocations = (mem.m_flags&NoLeakPrint) != 0;

  if (ignore)
    mem.m_flags |= NoLeakPrint;
  else
    mem.m_flags &= ~NoLeakPrint;

  return ignoreAllocations;
}


void PMemoryHeap::DumpStatistics()
{
  ostream * strm = GetInstance().m_leakDumpStream;
  if (strm != NULL)
    DumpStatistics(*strm);
}


static void OutputMemory(ostream & strm, size_t bytes)
{
  if (bytes < 10000) {
    strm << bytes << " bytes";
    return;
  }

  if (bytes < 10240000)
    strm << (bytes+1023)/1024 << "kb";
  else
    strm << (bytes+1048575)/1048576 << "Mb";
  strm << " (" << bytes << ')';
}

void PMemoryHeap::DumpStatistics(ostream & strm)
{
  if (PProcess::IsInitialised()) {
    PProcess::MemoryUsage usage;
    PProcess::Current().GetMemoryUsage(usage);
    strm << "\n"
            "Virtual memory usage     : ";
    OutputMemory(strm, usage.m_virtual);
    strm << "\n"
            "Resident memory usage    : ";
    OutputMemory(strm, usage.m_resident);
    strm << "\n"
            "Process heap memory max  : ";
    OutputMemory(strm, usage.m_max);
    strm << "\n"
            "Process memory heap usage: ";
    OutputMemory(strm, usage.m_current);
  }

  GetInstance().InternalDumpStatistics(strm);
}


void PMemoryHeap::InternalDumpStatistics(ostream & strm)
{
  Lock();

  strm << "\n"
          "Current memory usage     : ";
  OutputMemory(strm, m_currentMemoryUsage);
  strm << "\n"
          "Current objects count    : " << m_currentObjects << "\n"
          "Peak memory usage        : ";
  OutputMemory(strm, m_peakMemoryUsage);
  strm << "\n"
          "Peak objects created     : " << m_peakObjects << "\n"
          "Total objects created    : " << m_totalObjects << "\n"
          "Next allocation request  : " << m_allocationRequest << '\n'
       << endl;

  Unlock();
}


void PMemoryHeap::GetState(State & state)
{
  state.allocationNumber = GetInstance().m_allocationRequest;
}


void PMemoryHeap::SetAllocationBreakpoint(alloc_t point)
{
  GetInstance().m_allocationBreakpoint = point;
}


void PMemoryHeap::DumpObjectsSince(const State & state)
{
  PMemoryHeap & mem = GetInstance();
  if (mem.m_leakDumpStream != NULL)
    mem.InternalDumpObjectsSince(state.allocationNumber, *mem.m_leakDumpStream);
}


void PMemoryHeap::DumpObjectsSince(const State & state, ostream & strm)
{
  GetInstance().InternalDumpObjectsSince(state.allocationNumber, strm);
}


void PMemoryHeap::InternalDumpObjectsSince(DWORD objectNumber, ostream & strm)
{
  Lock();

  bool first = true;
  for (Header * obj = m_listHead; obj != NULL; obj = obj->m_next) {
    if (obj->m_request < objectNumber || (obj->m_flags&NoLeakPrint) != 0)
      continue;

    if (first && m_state == e_Destroyed) {
      *m_leakDumpStream << "\nMemory leaks detected, press Enter to display . . ." << flush;
#if !defined(_WIN32)
      cin.get();
#endif
      *m_leakDumpStream << '\n';
      first = false;
    }

    BYTE * data = (BYTE *)&obj[1];

    if (obj->m_fileName != NULL)
      strm << obj->m_fileName << '(' << obj->m_line << ") : ";

    strm << '#' << obj->m_request << ' ' << (void *)data << " [" << obj->m_size << "] ";

    if (obj->m_className != NULL)
      strm << "class=\"" << obj->m_className << "\" ";

    if (PProcess::IsInitialised() && obj->m_threadId != PNullThreadIdentifier) {
      strm << "thread=";
      PThread * thread = PProcess::Current().GetThread(obj->m_threadId);
      if (thread != NULL)
        strm << '"' << thread->GetThreadName() << "\" ";
      else
        strm << "0x" << hex << obj->m_threadId << dec << ' ';
    }

    strm << '\n' << hex << setfill('0') << PBYTEArray(data, std::min(MaxMemoryDumBytes, obj->m_size), false)
                 << dec << setfill(' ') << endl;
  }

  Unlock();
}


#else // PMEMORY_CHECK

#if defined(_MSC_VER) && defined(_DEBUG) && !defined(_WIN32_WCE)

static _CRT_DUMP_CLIENT pfnOldCrtDumpClient;
static bool hadCrtDumpLeak = false;

static void __cdecl MyCrtDumpClient(void * ptr, size_t size)
{
  if(_CrtReportBlockType(ptr) == P_CLIENT_BLOCK) {
    const PObject * obj = (PObject *)ptr;
    _RPT1(_CRT_WARN, "Class %s\n", obj->GetClass());
    hadCrtDumpLeak = true;
  }

  if (pfnOldCrtDumpClient != NULL)
    pfnOldCrtDumpClient(ptr, size);
}


PMemoryHeap::PMemoryHeap()
{
  _CrtMemCheckpoint(&initialState);
  pfnOldCrtDumpClient = _CrtSetDumpClient(MyCrtDumpClient);
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_ALLOC_MEM_DF);
}


PMemoryHeap::~PMemoryHeap()
{
  _CrtMemDumpAllObjectsSince(&initialState);
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_LEAK_CHECK_DF);
}


void PMemoryHeap::CreateInstance()
{
  static PMemoryHeap instance;
}


void * PMemoryHeap::Allocate(size_t nSize, const char * file, int line, const char * className)
{
  CreateInstance();
  return _malloc_dbg(nSize, className != NULL ? P_CLIENT_BLOCK : _NORMAL_BLOCK, file, line);
}


void * PMemoryHeap::Allocate(size_t count, size_t iSize, const char * file, int line)
{
  CreateInstance();
  return _calloc_dbg(count, iSize, _NORMAL_BLOCK, file, line);
}


void * PMemoryHeap::Reallocate(void * ptr, size_t nSize, const char * file, int line)
{
  CreateInstance();
  return _realloc_dbg(ptr, nSize, _NORMAL_BLOCK, file, line);
}


void PMemoryHeap::Deallocate(void * ptr, const char * className)
{
  _free_dbg(ptr, className != NULL ? P_CLIENT_BLOCK : _NORMAL_BLOCK);
}


PMemoryHeap::Validation PMemoryHeap::Validate(const void * ptr, const char * className, ostream * /*strm*/)
{
  CreateInstance();
  if (!_CrtIsValidHeapPointer(ptr))
    return Bad;

  if (_CrtReportBlockType(ptr) != P_CLIENT_BLOCK)
    return Ok;

  const PObject * obj = (PObject *)ptr;
  return strcmp(obj->GetClass(), className) == 0 ? Ok : Trashed;
}


PBoolean PMemoryHeap::ValidateHeap(ostream * /*strm*/)
{
  CreateInstance();
  return _CrtCheckMemory();
}


PBoolean PMemoryHeap::SetIgnoreAllocations(PBoolean ignoreAlloc)
{
  CreateInstance();
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  if (ignoreAlloc)
    _CrtSetDbgFlag(flags & ~_CRTDBG_ALLOC_MEM_DF);
  else
    _CrtSetDbgFlag(flags | _CRTDBG_ALLOC_MEM_DF);
  return (flags & _CRTDBG_ALLOC_MEM_DF) == 0;
}


void PMemoryHeap::DumpStatistics()
{
  CreateInstance();
  _CrtMemState state;
  _CrtMemCheckpoint(&state);
  _CrtMemDumpStatistics(&state);
}


void PMemoryHeap::DumpStatistics(ostream & /*strm*/)
{
  DumpStatistics();
}


void PMemoryHeap::GetState(State & state)
{
  CreateInstance();
  _CrtMemCheckpoint(&state);
}


void PMemoryHeap::DumpObjectsSince(const State & state)
{
  CreateInstance();
  _CrtMemDumpAllObjectsSince(&state);
}


void PMemoryHeap::DumpObjectsSince(const State & state, ostream & /*strm*/)
{
  CreateInstance();
  DumpObjectsSince(state);
}


void PMemoryHeap::SetAllocationBreakpoint(alloc_t objectNumber)
{
  CreateInstance();
  _CrtSetBreakAlloc(objectNumber);
}


#else // defined(_MSC_VER) && defined(_DEBUG)

#if !defined(P_VXWORKS) && !defined(_WIN32_WCE) && !defined(P_ANDROID)

#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void * operator new[](size_t nSize) throw (std::bad_alloc)
#else
void * operator new[](size_t nSize)
#endif
{
  return malloc(nSize);
}

#if (__GNUC__ >= 3) || ((__GNUC__ == 2)&&(__GNUC_MINOR__ >= 95)) //2.95.X & 3.X
void operator delete[](void * ptr) throw ()
#else
void operator delete[](void * ptr)
#endif
{
  free(ptr);
}

#endif // !P_VXWORKS

#endif // defined(_MSC_VER) && defined(_DEBUG)

#endif // PMEMORY_CHECK




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


PBoolean PInt64::Lt(const PInt64 & v) const
{
  if ((long)high < (long)v.high)
    return true;
  if ((long)high > (long)v.high)
    return false;
  if ((long)high < 0)
    return (long)low > (long)v.low;
  return (long)low < (long)v.low;
}


PBoolean PInt64::Gt(const PInt64 & v) const
{
  if ((long)high > (long)v.high)
    return true;
  if ((long)high < (long)v.high)
    return false;
  if ((long)high < 0)
    return (long)low < (long)v.low;
  return (long)low > (long)v.low;
}


PBoolean PUInt64::Lt(const PUInt64 & v) const
{
  if (high < v.high)
    return true;
  if (high > v.high)
    return false;
  return low < high;
}


PBoolean PUInt64::Gt(const PUInt64 & v) const
{
  if (high > v.high)
    return true;
  if (high < v.high)
    return false;
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


#ifdef P_TORNADO

// the library provided with Tornado 2.0 does not contain implementation 
// for the functions defined below, therefor the own implementation

ostream & ostream::operator<<(PInt64 v)
{
  return *this << (long)(v >> 32) << (long)(v & 0xFFFFFFFF);
}


ostream & ostream::operator<<(PUInt64 v)
{
  return *this << (long)(v >> 32) << (long)(v & 0xFFFFFFFF);
}

istream & istream::operator>>(PInt64 & v)
{
  return *this >> (long)(v >> 32) >> (long)(v & 0xFFFFFFFF);
}


istream & istream::operator>>(PUInt64 & v)
{
  return *this >> (long)(v >> 32) >> (long)(v & 0xFFFFFFFF);
}

#endif // P_TORNADO


///////////////////////////////////////////////////////////////////////////////

namespace PProfiling
{
  static uint64_t InitFrequency()
  {
#if defined(P_LINUX)
    ifstream cpuinfo("/proc/cpuinfo", ios::in);
    while (cpuinfo.good()) {
      char line[100];
      cpuinfo.getline(line, sizeof(line));
      if (strncmp(line, "cpu MHz", 7) == 0)
        return (uint64_t)(atof(strchr(line, ':')+1)*1000000);
    }
    return 2000000000; // 2GHz
#elif defined(_M_IX86) || defined(_M_X64) || defined(_WIN32)
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    return li.QuadPart * 1000;
#elif defined(CLOCK_MONOTONIC)
    return 1000000000ULL;
#else
    return 1000000ULL;
#endif
  }

  static uint64_t gs_Frequency = InitFrequency();

  int64_t CyclesToNanoseconds(uint64_t cycles)
  {
    static long double const gs_CyclesPerNanoseconds = (long double)gs_Frequency/PTimeInterval::SecsToNano;
    return (int64_t)(cycles/gs_CyclesPerNanoseconds);
  }


  float CyclesToSeconds(uint64_t cycles)
  {
    return (float)((double)cycles/gs_Frequency);
  }




#if P_PROFILING
// Currently only supported in GNU && *nix

  enum FunctionType
  {
    e_AutoEntry,
    e_AutoExit,
    e_ManualEntry,
    e_ManualExit,
    e_SystemEntry,
    e_SystemExit
  };

  struct FunctionRawData
  {
    PPROFILE_EXCLUDE(FunctionRawData(FunctionType type, void * function, void * caller));
    PPROFILE_EXCLUDE(FunctionRawData(FunctionType type, const char * name, const char * file, unsigned line));

    PPROFILE_EXCLUDE(void Dump(ostream & out) const);

    // Do not use memory check allocation
    PPROFILE_EXCLUDE(void * operator new(size_t nSize));
    PPROFILE_EXCLUDE(void operator delete(void * ptr));

    union
    {
      // Note for correct operation m_pointer must overlay m_name
      struct
      {
        void * m_pointer;
        void * m_caller;
      };
      struct
      {
        const char * m_name;
        const char * m_file;
        unsigned     m_line;
      };
    } m_function;

    FunctionType            m_type;
    PThreadIdentifier       m_threadIdentifier;
    PUniqueThreadIdentifier m_threadUniqueId;
    uint64_t                m_when;
    FunctionRawData       * m_link;

  private:
    FunctionRawData(const FunctionRawData &) { }
    void operator=(const FunctionRawData &) { }
  };


  struct ThreadRawData : Thread
  {
    PPROFILE_EXCLUDE(ThreadRawData(
      PThreadIdentifier threadId,
      PUniqueThreadIdentifier uniqueId,
      const char * name,
      float real,
      float systemCPU,
      float userCPU
    ));

    PPROFILE_EXCLUDE(void Dump(ostream & out) const);

    // Do not use memory check allocation
    PPROFILE_EXCLUDE(void * operator new(size_t nSize));
    PPROFILE_EXCLUDE(void operator delete(void * ptr));

    ThreadRawData * m_link;
  };


  struct Database
  {
  public:
    PPROFILE_EXCLUDE(Database());
    PPROFILE_EXCLUDE(~Database());

    bool     m_enabled;
    atomic<FunctionRawData *> m_functions;
    atomic<ThreadRawData *>   m_threads;
    uint64_t m_start;
  };
  static Database s_database;


  /////////////////////////////////////////////////////////////////////

  FunctionRawData::FunctionRawData(FunctionType type, void * function, void * caller)
    : m_type(type)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
    , m_when(GetCycles())
  {
    m_function.m_pointer = function;
    m_function.m_caller = caller;

    m_link = s_database.m_functions.exchange(this);
  }


  FunctionRawData::FunctionRawData(FunctionType type, const char * name, const char * file, unsigned line)
    : m_type(type)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
    , m_when(GetCycles())
  {
    m_function.m_name = name;
    m_function.m_file = file;
    m_function.m_line = line;

    m_link = s_database.m_functions.exchange(this);
  }


  void * FunctionRawData::operator new(size_t nSize)
  {
    return runtime_malloc(nSize);

  }
  void FunctionRawData::operator delete(void * ptr)
  {
    runtime_free(ptr);
  }


   void FunctionRawData::Dump(ostream & out) const
  {
    switch (m_type) {
      case e_AutoEntry:
        out << "AutoEnter\t" << m_function.m_pointer << '\t' << m_function.m_caller;
        break;
      case e_AutoExit:
        out << "AutoExit\t" << m_function.m_pointer << '\t' << m_function.m_caller;
        break;
      case e_ManualEntry:
        out << "ManualEnter\t" << m_function.m_name << '\t' << m_function.m_file << '(' << m_function.m_line << ')';
        break;
      case e_ManualExit:
        out << "ManualExit\t" << m_function.m_name << '\t';
        break;
      default :
        PAssertAlways(PLogicError);
    }

    out << '\t' << m_threadUniqueId << '\t' << m_when << '\n';
  }


  /////////////////////////////////////////////////////////////////////

  ThreadRawData::ThreadRawData(PThreadIdentifier threadId,
                               PUniqueThreadIdentifier uniqueId,
                               const char * name,
                               float realTime,
                               float systemCPU,
                               float userCPU)
    : Thread(threadId, uniqueId, name, realTime, systemCPU, userCPU)
  {
  }


  void * ThreadRawData::operator new(size_t nSize)
  {
    return runtime_malloc(nSize);

  }
  void ThreadRawData::operator delete(void * ptr)
  {
    runtime_free(ptr);
  }


  void ThreadRawData::Dump(ostream & out) const
  {
    out << "Thread\t" << m_name << '\t' << m_threadId << '\t' << m_uniqueId << '\t' << m_realTime << '\t' << m_userCPU << '\t' << m_systemCPU << '\n';
  }


  void OnThreadEnded(const PThread & thread, const PTimeInterval & realTime, const PTimeInterval & systemCPU, const PTimeInterval & userCPU)
  {
    if (s_database.m_enabled) {
      ThreadRawData * info = new ThreadRawData(thread.GetThreadId(),
                                               thread.GetUniqueIdentifier(),
                                               thread.GetThreadName(),
                                               realTime.GetMilliSeconds()/1000.0f,
                                               systemCPU.GetMilliSeconds()/1000.0f,
                                               userCPU.GetMilliSeconds()/1000.0f);
      info->m_link = s_database.m_threads.exchange(info);
    }
  }


  /////////////////////////////////////////////////////////////////////

  Block::Block(const char * name, const char * file, unsigned line)
    : m_name(name)
  {
    if (s_database.m_enabled)
      new FunctionRawData(e_ManualEntry, name, file, line);
  }


  Block::~Block()
  {
    if (s_database.m_enabled)
      new FunctionRawData(e_ManualExit, m_name, NULL, 0);
  }


  /////////////////////////////////////////////////////////////////////

  Database::Database()
    : m_enabled(getenv("PTLIB_PROFILING_ENABLED") != NULL)
    , m_functions(NULL)
    , m_threads(NULL)
    , m_start(GetCycles())
  {
  }


  Database::~Database()
  {
    if (static_cast<FunctionRawData *>(m_functions) == NULL)
      return;

    const char * filename;

    if ((filename = getenv("PTLIB_RAW_PROFILING_FILENAME")) != NULL) {
      ofstream out(filename, ios::out | ios::trunc);
      if (out.is_open())
        Dump(out);
    }

    if ((filename = getenv("PTLIB_PROFILING_FILENAME")) != NULL) {
      ofstream out(filename, ios::out | ios::trunc);
      if (out.is_open())
        Analyse(out, strstr(filename, ".html") != NULL);
    }

    Reset();
  }


  void Enable(bool enab)
  {
    s_database.m_enabled = enab;
  }


  bool IsEnabled()
  {
    return s_database.m_enabled;
  }


  void Reset()
  {
    s_database.m_start = GetCycles();
    ThreadRawData * thrd = s_database.m_threads.exchange(NULL);
    FunctionRawData * func = s_database.m_functions.exchange(NULL);

    while (func != NULL) {
      FunctionRawData * del = func;
      func = func->m_link;
      delete del;
    }

    while (thrd != NULL) {
      ThreadRawData * del = thrd;
      thrd = thrd->m_link;
      delete del;
    }
  }


  void PreSystem()
  {
    if (s_database.m_enabled)
      new FunctionRawData(e_SystemEntry, NULL, NULL, 0);
  }


  void PostSystem()
  {
    if (s_database.m_enabled)
      new FunctionRawData(e_SystemExit, NULL, NULL, 0);
  }


  void Dump(ostream & strm)
  {
    for (ThreadRawData * info = s_database.m_threads; info != NULL; info = info->m_link)
      info->Dump(strm);
    for (FunctionRawData * info = s_database.m_functions; info != NULL; info = info->m_link)
      info->Dump(strm);
  }


  class CpuTime
  {
    private:
      uint64_t m_cycles;
      double   m_time;

    public:
      CpuTime(uint64_t cycles)
        : m_cycles(cycles)
        , m_time(CyclesToSeconds(cycles))
      {
      }

    private:
      void PrintOn(ostream & strm) const
      {
        strm << m_cycles << " (";
        if (m_time >= 1) {
          if (m_time >= 1000)
            strm.precision(0);
          else if (m_time >= 100)
            strm.precision(1);
          else if (m_time >= 10)
            strm.precision(2);
          else
            strm.precision(3);
          strm << m_time;
        }
        else if (m_time >= 0.001) {
          if (m_time >= 0.1)
            strm.precision(1);
          else if (m_time >= 0.01)
            strm.precision(2);
          else
            strm.precision(3);
          strm << 1000.0*m_time << 'm';
        }
        else {
          if (m_time >= 0.0001)
            strm.precision(1);
          else if (m_time >= 0.00001)
            strm.precision(2);
          else
            strm.precision(3);
          strm << 1000000.0*m_time << 'µ';
        }
        strm << "s)";
      }

    friend ostream & operator<<(ostream & strm, const CpuTime & c)
      {
        if (strm.width() < 10)
          c.PrintOn(strm);
        else {
          ostringstream str;
          c.PrintOn(str);
          strm << str.str();
        }
        return strm;
      }
  };


  __inline static float Percentage(float v1, float v2)
  {
    return v2 > 0 ? 100.0f * v1 / v2 : 0.0f;
  }


  void Analysis::ToText(ostream & strm) const
  {
    std::streamsize threadNameWidth = 0;
    std::streamsize functionNameWidth = 0;
    for (ThreadByUsage::const_iterator thrd = m_threadByUsage.begin(); thrd != m_threadByUsage.end(); ++thrd) {
      std::streamsize len = thrd->second.m_name.length();
      if (len > threadNameWidth)
        threadNameWidth = len;

      for (FunctionMap::const_iterator func = thrd->second.m_functions.begin(); func != thrd->second.m_functions.end(); ++func) {
        len = func->first.length();
        if (len > functionNameWidth)
          functionNameWidth = len;
      }
    }
    threadNameWidth += 3;
    functionNameWidth += 2;

    strm << "Summary profile:"
            " threads="   << m_threadByID.size() << ","
            " functions=" << m_functionCount  << ","
            " cycles="    << m_durationCycles << ","
            " frequency=" << gs_Frequency      << ","
            " time="      << left << fixed << setprecision(3) << CyclesToSeconds(m_durationCycles) << '\n';
    for (ThreadByUsage::const_iterator thrd = m_threadByUsage.begin(); thrd != m_threadByUsage.end(); ++thrd) {
      strm << "   Thread \"" << setw(threadNameWidth) << (thrd->second.m_name+'"')
            <<   "  id=" << setw(8) << thrd->second.m_uniqueId
            << "  real=" << setw(10) << setprecision(3) << thrd->second.m_realTime
            <<  "  sys=" << setw(10) << setprecision(3) << thrd->second.m_systemCPU
            << "  user=" << setw(10) << setprecision(3) << thrd->second.m_userCPU;
      if (thrd->first >= 0)
        strm << " (" << setprecision(2) << thrd->first << "%)";
      strm << '\n';

      for (FunctionMap::const_iterator func = thrd->second.m_functions.begin(); func != thrd->second.m_functions.end(); ++func) {
        uint64_t avg = func->second.m_sum / func->second.m_count;
        strm << "      " << left << setw(functionNameWidth) << func->first
              << " count=" << setw(10) << func->second.m_count
              << " min=" << setw(24) << CpuTime(func->second.m_minimum)
              << " max=" << setw(24) << CpuTime(func->second.m_maximum)
              << " avg=" << setw(24) << CpuTime(avg);
        if (thrd->second.m_realTime > 0)
          strm << " (" << setprecision(2) << Percentage(CyclesToSeconds(func->second.m_sum), thrd->second.m_realTime) << "%)";
        strm << '\n';
      }
    }
  }


  class EscapedHTML
  {
    private:
      const std::string m_str;

    public:
      EscapedHTML(const std::string & str)
        : m_str(str)
      {
      }

    friend ostream & operator<<(ostream & strm, const EscapedHTML & e)
    {
      for (size_t i = 0; i < e.m_str.length(); ++i) {
        switch (e.m_str[i]) {
          case '"':
            strm << "&quot;";
            break;
          case '<':
            strm << "&lt;";
            break;
          case '>':
            strm << "&gt;";
            break;
          case '&':
            strm << "&amp;";
            break;
          default:
            strm << e.m_str[i];
        }
      }
      return strm;
    }
  };


  void Analysis::ToHTML(ostream & strm) const
  {
    strm << "<H2>Summary profile</H2>"
            "<table border=1 cellspacing=1 cellpadding=12>"
            "<tr>"
            "<th>Threads<th>Functions<th>Cycles<th>Frequency<th>Time"
            "<tr>"
            "<td align=center>" << m_threadByID.size()
         << "<td align=center>" << m_functionCount
         << "<td align=center>" << m_durationCycles
         << "<td align=center>" << gs_Frequency
         << "<td align=center>" << fixed << setprecision(3) << CyclesToSeconds(m_durationCycles)
         << "</table>"
            "<p>"
            "<table width=\"100%\" border=1 cellspacing=0 cellpadding=8>"
            "<tr><th width=\"1%\">ID"
                "<th align=left>Thread"
                "<th width=\"5%\" nowrap>Real Time"
                "<th width=\"5%\" nowrap>System CPU"
                "<th width=\"5%\" align=right nowrap>System Core %"
                "<th width=\"5%\" nowrap>User CPU"
                "<th width=\"5%\" align=right nowrap>User Core %";
    for (ThreadByUsage::const_iterator thrd = m_threadByUsage.begin(); thrd != m_threadByUsage.end(); ++thrd) {
      strm << "<tr>"
              "<td width=\"1%\" align=center>" << thrd->second.m_uniqueId
           << "<td>" << EscapedHTML(thrd->second.m_name)
           << setprecision(3)
           << "<td width=\"5%\" align=center>" << thrd->second.m_realTime << 's'
           << "<td width=\"5%\" align=center>" << thrd->second.m_systemCPU << 's'
           << "<td width=\"5%\" align=right>";
      if (thrd->first >= 0)
        strm << setprecision(2) << Percentage(thrd->second.m_systemCPU, thrd->second.m_realTime) << '%';
      else
        strm << "&nbsp;";
      strm << "<td width=\"5%\" align=center>" << thrd->second.m_userCPU << 's'
           << "<td width=\"5%\" align=right>";
      if (thrd->first >= 0)
        strm << setprecision(2) << thrd->first << '%';
      else
        strm << "&nbsp;";
      if (!thrd->second.m_functions.empty()) {
        strm << "<tr><td>&nbsp;<td colspan=\"9999\">"
                "<table border=1 cellspacing=1 cellpadding=4 width=100%>"
                "<th align=left>Function"
                "<th>Count"
                "<th>Minimum"
                "<th>Maximum"
                "<th>Average"
                "<th align=right nowrap>Core %";
        for (FunctionMap::const_iterator func = thrd->second.m_functions.begin(); func != thrd->second.m_functions.end(); ++func) {
          uint64_t avg = func->second.m_sum / func->second.m_count;
          strm << "<tr>"
                  "<td>" << EscapedHTML(func->first)
               << "<td align=center>" << func->second.m_count
               << "<td align=center nowrap>" << CpuTime(func->second.m_minimum)
               << "<td align=center nowrap>" << CpuTime(func->second.m_maximum)
               << "<td align=center nowrap>" << CpuTime(avg);
          if (thrd->second.m_realTime > 0)
            strm << "<td align=right>" << setprecision(2) << Percentage(CyclesToSeconds(func->second.m_sum), thrd->second.m_realTime) << '%';
        }
        strm << "</table>";
      }
    }
    strm << "</table>";
  }


  static ThreadByID::iterator AddThreadByID(ThreadByID & threadByID, const PThread::Times & times)
  {
    Thread threadInfo(times.m_threadId, times.m_uniqueId);
    threadInfo.m_name = times.m_name.GetPointer();
    threadInfo.m_realTime = times.m_real.GetMilliSeconds()/1000.0f;
    threadInfo.m_systemCPU = times.m_kernel.GetMilliSeconds()/1000.0f;
    threadInfo.m_userCPU = times.m_user.GetMilliSeconds()/1000.0f;
    threadInfo.m_running = true;
    return threadByID.insert(make_pair(times.m_uniqueId, threadInfo)).first;
  }

  void Analyse(Analysis & analysis)
  {
    analysis.m_durationCycles = GetCycles() - s_database.m_start;

    std::list<PThread::Times> times;
    PThread::GetTimes(times);
    for (std::list<PThread::Times>::iterator it = times.begin(); it != times.end(); ++it)
      AddThreadByID(analysis.m_threadByID, *it);

    for (ThreadRawData * thrd = s_database.m_threads; thrd != NULL; thrd = thrd->m_link)
      analysis.m_threadByID.insert(make_pair(thrd->m_uniqueId, *thrd));

    for (FunctionRawData * exit = s_database.m_functions; exit != NULL; exit = exit->m_link) {
      std::string functionName;
      switch (exit->m_type) {
        default :
          continue;

        case e_ManualExit:
          functionName = exit->m_function.m_name;
          break;

        case e_AutoExit:
          stringstream strm;
          strm << exit->m_function.m_pointer;
          functionName = strm.str();
          break;
      }

      uint64_t subFunctionAccumulator = 0;

      // Find the next entry in that thread
      FunctionRawData * entry;
      for (entry = exit->m_link; entry != NULL; entry = entry->m_link) {
        if (entry->m_threadUniqueId != exit->m_threadUniqueId)
          continue;

        switch (entry->m_type) {
          case e_ManualEntry :
          case e_AutoEntry:
          case e_SystemEntry :
            if (entry->m_function.m_pointer == exit->m_function.m_pointer)
              break;
            continue; // Should not happen!

          default :
            // Have an exit, so look for matching entry
            FunctionRawData * subEntry;
            for (subEntry = entry->m_link; subEntry != NULL; subEntry = subEntry->m_link) {
              if (subEntry->m_threadUniqueId == entry->m_threadUniqueId && subEntry->m_function.m_pointer == entry->m_function.m_pointer)
                break;
            }
            if (subEntry == NULL)
              continue; // Should not happen!

            // Amount of time in sub-function, subtract it off later
            subFunctionAccumulator += entry->m_when - subEntry->m_when;
            entry = subEntry;
            continue;
        }

        ThreadByID::iterator thrd = analysis.m_threadByID.find(entry->m_threadUniqueId);
        if (thrd == analysis.m_threadByID.end()) {
          PThread::Times threadTimes;
          PThread::GetTimes(entry->m_threadIdentifier, threadTimes);
          thrd = AddThreadByID(analysis.m_threadByID, threadTimes);
        }

        FunctionMap & functions = thrd->second.m_functions;
        FunctionMap::iterator func = functions.find(functionName);
        if (func == functions.end()) {
          func = functions.insert(make_pair(functionName, Function())).first;
          ++analysis.m_functionCount;
        }

        uint64_t diff = exit->m_when - entry->m_when - subFunctionAccumulator;

        if (func->second.m_minimum > diff)
          func->second.m_minimum = diff;
        if (func->second.m_maximum < diff)
          func->second.m_maximum = diff;

        func->second.m_sum += diff;
        ++func->second.m_count;
        break;
      }
    }

    for (ThreadByID::iterator thrd = analysis.m_threadByID.begin(); thrd != analysis.m_threadByID.end(); ++thrd)
      analysis.m_threadByUsage.insert(make_pair(Percentage(thrd->second.m_userCPU, thrd->second.m_realTime), thrd->second));
  }


  void Analyse(ostream & strm, bool html)
  {
    Analysis analysis;
    Analyse(analysis);

    if (html)
      analysis.ToHTML(strm);
    else
      analysis.ToText(strm);
  }

#endif // P_PROFILING

#if PTRACING

  struct TimeScope::Implementation
  {
    const char  * const m_name;
    const char  * const m_file;
    unsigned      const m_line;
    PTimeInterval const m_thresholdTime;
    PTimeInterval const m_throttleTime;
    unsigned      const m_throttledLogLevel;
    unsigned      const m_unthrottledLogLevel;
    unsigned      const m_thresholdPercent;
    unsigned      const m_maxHistory;

    PMinMaxAvg<PTimeInterval> m_mma;
    unsigned                  m_countTimesOverThreshold;
    PTime                     m_lastOutputTime;
    PTimeInterval             m_lastDuration;

    struct History
    {
        explicit History(const PTimeInterval & duration)
            : m_when(), m_duration(duration) { }

        PTime         m_when;
        PTimeInterval m_duration;
    };

    std::list<History> m_history;

    PCriticalSection m_mutex;

    Implementation(const char * name,
                   const char * file,
                   unsigned line,
                   unsigned thresholdTime,
                   unsigned throttleTime,
                   unsigned throttledLogLevel,
                   unsigned unthrottledLogLevel,
                   unsigned thresholdPercent,
                   unsigned maxHistory)
      : m_name(name)
      , m_file(file)
      , m_line(line)
      , m_thresholdTime(thresholdTime)
      , m_throttleTime(throttleTime)
      , m_throttledLogLevel(throttledLogLevel)
      , m_unthrottledLogLevel(unthrottledLogLevel)
      , m_thresholdPercent(thresholdPercent)
      , m_maxHistory(maxHistory)
      , m_mma("s")
      , m_countTimesOverThreshold(0)
      , m_lastOutputTime(0)
    {
    }

    void EndMeasurement(const void *, const PObject * object, const PTimeInterval & duration)
    {
      PWaitAndSignal lock(m_mutex);

      m_lastDuration = duration;
      m_mma.Accumulate(duration);

      if (!PTrace::CanTrace(m_throttledLogLevel) || duration < m_thresholdTime)
        return;

      ++m_countTimesOverThreshold;

      if (m_mma.GetCount() < 3)
        return;

      PTime now;
      unsigned percentOver = m_countTimesOverThreshold * 100 / m_mma.GetCount();
      bool isTime = (now - m_lastOutputTime) > m_throttleTime;
      unsigned level = isTime && percentOver >= m_thresholdPercent ? m_throttledLogLevel : m_unthrottledLogLevel;

      if (PTrace::CanTrace(level)) {
        ostream & trace = PTrace::Begin(level, m_file, m_line, object, "TimeScope");
        trace << m_name << ":"
                  " since=" << m_lastOutputTime.AsString(PTime::TodayFormat, PTrace::GetTimeZone()) << ","
              << setprecision(3) << scientific << showbase << m_mma << noshowbase
              << " thresh=" << m_thresholdTime.AsString(3, PTimeInterval::SecondsSI) << "s;" << m_thresholdPercent << "%,"
                   " slow=" << m_countTimesOverThreshold << '/' << m_mma.GetCount() << ' ' << percentOver << '%';
        for (list<History>::iterator it = m_history.begin(); it != m_history.end(); ++it)
          trace << "\n    when=" << it->m_when.AsString(PTime::TodayFormat, PTrace::GetTimeZone()) << ","
                    " duration=" << it->m_duration.AsString(3, PTimeInterval::SecondsSI) << 's';
        trace << PTrace::End;
      }

      if (isTime) {
        m_mma.Reset();
        m_countTimesOverThreshold = 0;
        m_lastOutputTime = now;
        m_history.clear();
      }

      if (m_maxHistory > 0) {
          m_history.push_back(History(duration));
          if (m_history.size() > m_maxHistory)
              m_history.pop_front();
      }
    }
  };

  TimeScope::TimeScope(const char * name,
                       const char * file,
                       unsigned line,
                       unsigned thresholdTime,
                       unsigned throttleTime,
                       unsigned throttledLogLevel,
                       unsigned unthrottledLogLevel,
                       unsigned thresholdPercent,
                       unsigned maxHistory)
    // Note that there is a race here on he static definition of the TimeScope isntance,
    // that would leak precisely one Implementation object, big deal.
    : m_implementation(new Implementation(name,
                                          file,
                                          line,
                                          thresholdTime,
                                          throttleTime,
                                          throttledLogLevel,
                                          unthrottledLogLevel,
                                          thresholdPercent,
                                          maxHistory))
  {
  }


  TimeScope::TimeScope(const TimeScope & other)
    : m_implementation(new Implementation(other.m_implementation->m_name,
                                          other.m_implementation->m_file,
                                          other.m_implementation->m_line,
                                          other.m_implementation->m_thresholdTime.GetSeconds(),
                                          other.m_implementation->m_throttleTime.GetSeconds(),
                                          other.m_implementation->m_throttledLogLevel,
                                          other.m_implementation->m_unthrottledLogLevel,
                                          other.m_implementation->m_thresholdPercent,
                                          other.m_implementation->m_maxHistory))
  {
  }


  TimeScope::~TimeScope()
  {
    delete m_implementation;
  }

  
  void TimeScope::EndMeasurement(const void * context, const PObject * object, uint64_t startCycle)
  {
    m_implementation->EndMeasurement(context, object, PTimeInterval::NanoSeconds(CyclesToNanoseconds(GetCycles() - startCycle)));
  }

  const PTimeInterval & TimeScope::GetLastDuration() const
  {
    return m_implementation->m_lastDuration;
  }
#endif // PTRACING

}; // namespace PProfiling

#if P_PROFILING

#ifdef __GNUC__
extern "C"
{
  #undef new

  PPROFILE_EXCLUDE(void __cyg_profile_func_enter(void * function, void * caller));
  PPROFILE_EXCLUDE(void __cyg_profile_func_exit(void * function, void * caller));

  void __cyg_profile_func_enter(void * function, void * caller)
  {
    if (PProfiling::s_database.m_enabled)
      new PProfiling::FunctionRawData(PProfiling::e_AutoEntry, function, caller);
  }

  void __cyg_profile_func_exit(void * function, void * caller)
  {
    if (PProfiling::s_database.m_enabled)
      new PProfiling::FunctionRawData(PProfiling::e_AutoExit, function, caller);
  }
};
#endif // __GNUC__

#endif // P_PROFILING


// End Of File ///////////////////////////////////////////////////////////////

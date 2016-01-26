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
    return PAssertFunc(msgbuf);
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

  const char * theMsg;
  char msgbuf[20];
  if (msg < PMaxStandardAssertMessage)
    theMsg = textmsg[msg];
  else {
    sprintf(msgbuf, "Assertion %i", msg);
    theMsg = msgbuf;
  }
  return PAssertFunc(file, line, className, theMsg);
}


bool PAssertFunc(const char * file, int line, const char * className, const char * msg)
{
#if defined(_WIN32)
  DWORD err = GetLastError();
#else
  int err = errno;
#endif

  ostringstream str;
  str << "Assertion fail: ";
  if (msg != NULL)
    str << msg << ", ";
  str << "file " << file << ", line " << line;
  if (className != NULL)
    str << ", class " << className;
  if (err != 0)
    str << ", error=" << err;
  str << ", when=" << PTime().AsString(PTime::LoggingFormat) << ends;
  
  return PAssertFunc(str.str().c_str());
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


DWORD PMemoryHeap::allocationBreakpoint = 0;
char PMemoryHeap::Header::GuardBytes[NumGuardBytes];
static const size_t MaxMemoryDumBytes = 16;
static void * DeletedPtr;
static void * UninitialisedPtr;
static void * GuardedPtr;


PMemoryHeap::Wrapper::Wrapper()
{
  // The following is done like this to get over brain dead compilers that cannot
  // guarentee that a static global is contructed before it is used.
  static PMemoryHeap real_instance;
  instance = &real_instance;
  if (instance->m_state != e_Active)
    return;

#if defined(_WIN32)
  EnterCriticalSection(&instance->mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_lock(&instance->mutex);
#elif defined(P_VXWORKS)
  semTake((SEM_ID)instance->mutex, WAIT_FOREVER);
#endif
}


PMemoryHeap::Wrapper::~Wrapper()
{
  if (instance->m_state != e_Active)
    return;

#if defined(_WIN32)
  LeaveCriticalSection(&instance->mutex);
#elif defined(P_PTHREADS)
  pthread_mutex_unlock(&instance->mutex);
#elif defined(P_VXWORKS)
  semGive((SEM_ID)instance->mutex);
#endif
}


PMemoryHeap::PMemoryHeap()
{
  const char * env = getenv("PTLIB_MEMORY_CHECK");
  m_state = env == NULL || atoi(env) > 0 ? e_Active : e_Disabled;

  listHead = NULL;
  listTail = NULL;

  allocationRequest = 1;
  firstRealObject = 0;
  flags = NoLeakPrint;

  allocFillChar = '\xCD'; // Microsoft debug heap values
  freeFillChar = '\xDD';

  currentMemoryUsage = 0;
  peakMemoryUsage = 0;
  currentObjects = 0;
  peakObjects = 0;
  totalObjects = 0;

  for (PINDEX i = 0; i < Header::NumGuardBytes; i++)
    Header::GuardBytes[i] = '\xFD';

  memset(&DeletedPtr, freeFillChar, sizeof(DeletedPtr));
  memset(&UninitialisedPtr, allocFillChar, sizeof(UninitialisedPtr));
  memset(&GuardedPtr, '\xFD', sizeof(GuardedPtr));

#if defined(_WIN32)
  InitializeCriticalSection(&mutex);
  static PDebugStream debug;
  leakDumpStream = &debug;
#else
#if defined(P_PTHREADS)
#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
  pthread_mutex_t recursiveMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  mutex = recursiveMutex;
#else
 pthread_mutex_init(&mutex, NULL);
#endif
#elif defined(P_VXWORKS)
  mutex = semMCreate(SEM_Q_FIFO);
#endif
  leakDumpStream = &cerr;
#endif
}


PMemoryHeap::~PMemoryHeap()
{
  m_state = e_Destroyed;

  if (leakDumpStream != NULL) {
    *leakDumpStream << "Final memory statistics:\n";
    InternalDumpStatistics(*leakDumpStream);
    InternalDumpObjectsSince(firstRealObject, *leakDumpStream);
  }

#if defined(_WIN32)
  DeleteCriticalSection(&mutex);
  PProcess::Current().WaitOnExitConsoleWindow();
#elif defined(P_PTHREADS)
  pthread_mutex_destroy(&mutex);
#elif defined(P_VXWORKS)
  semDelete((SEM_ID)mutex);
#endif
}


void * PMemoryHeap::Allocate(size_t nSize, const char * file, int line, const char * className)
{
  Wrapper mem;
  if (mem->m_state == e_Disabled)
    return malloc(nSize);
  return mem->InternalAllocate(nSize, file, line, className);
}


void * PMemoryHeap::Allocate(size_t count, size_t size, const char * file, int line)
{
  Wrapper mem;
  if (mem->m_state == e_Disabled)
    return calloc(count, size);

  char oldFill = mem->allocFillChar;
  mem->allocFillChar = '\0';

  void * data = mem->InternalAllocate(count*size, file, line, NULL);

  mem->allocFillChar = oldFill;

  return data;
}


void * PMemoryHeap::InternalAllocate(size_t nSize, const char * file, int line, const char * className)
{
  if (m_state != e_Active)
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
  if (firstRealObject == 0 && (flags&NoLeakPrint) == 0) {
    if (leakDumpStream != NULL)
      *leakDumpStream << "PTLib memory checking activated." << endl;
    firstRealObject = allocationRequest;
  }

  if (allocationBreakpoint != 0 && allocationRequest == allocationBreakpoint)
    PBreakToDebugger();

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
   obj->threadId = PThread::GetCurrentThreadId();
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

  if (mem->m_state != e_Active)
    return realloc(ptr, nSize);

  if (mem->InternalValidate(ptr, NULL, mem->leakDumpStream) != Ok)
    return NULL;

  Header * obj = (Header *)realloc(((Header *)ptr)-1, sizeof(Header) + nSize + sizeof(obj->guard));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  if (mem->allocationBreakpoint != 0 && mem->allocationRequest == mem->allocationBreakpoint)
    PBreakToDebugger();

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

  Header * obj = ((Header *)ptr)-1;

  switch (mem->InternalValidate(ptr, className, mem->leakDumpStream)) {
    case Trashed :
      free(obj);  // Try and free out extended version, and continue
      return;

    case Inactive :
    case Bad :
      free(ptr);  // Try and free it as if we didn't extend it, and continue
      return;

    default :
      break;
  }

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


PMemoryHeap::Validation PMemoryHeap::Validate(const void * ptr,
                                              const char * className,
                                              ostream * error)
{
  Wrapper mem;
  return mem->InternalValidate(ptr, className, error);
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
  if (m_state != e_Active)
    return Inactive;

  if (ptr == NULL)
    return Bad;

  Header * obj = ((Header *)ptr)-1;

  unsigned count = currentObjects;
  Header * link = listTail;
  while (link != NULL && link != obj && count-- > 0) {
    if (link->prev == DeletedPtr || link->prev == UninitialisedPtr || link->prev == GuardedPtr) {
      PMEMORY_VALIDATE_ERROR("Block " << ptr << " trashed!");
      return Trashed;
    }
    link = link->prev;
  }

  if (link != obj) {
    PMEMORY_VALIDATE_ERROR("Block " << ptr << " not in heap!");
    return Bad;
  }

  if (memcmp(obj->guard, obj->GuardBytes, sizeof(obj->guard)) != 0) {
    PMEMORY_VALIDATE_ERROR("Underrun at " << ptr << '[' << obj->size << "] #" << obj->request);
    return Corrupt;
  }
  
  if (memcmp((char *)ptr+obj->size, obj->GuardBytes, sizeof(obj->guard)) != 0) {
    PMEMORY_VALIDATE_ERROR("Overrun at " << ptr << '[' << obj->size << "] #" << obj->request);
    return Corrupt;
  }
  
  if (!(className == NULL && obj->className == NULL) &&
       (className == NULL || obj->className == NULL ||
       (className != obj->className && strcmp(obj->className, className) != 0))) {
    PMEMORY_VALIDATE_ERROR("PObject " << ptr << '[' << obj->size << "] #" << obj->request
                           << " allocated as \"" << (obj->className != NULL ? obj->className : "<NULL>")
                           << "\" and should be \"" << (className != NULL ? className : "<NULL>") << "\".");
    return Corrupt;
  }

  return Ok;
}


PBoolean PMemoryHeap::ValidateHeap(ostream * error)
{
  Wrapper mem;

  if (error == NULL)
    error = mem->leakDumpStream;

  Header * obj = mem->listHead;
  while (obj != NULL) {
    if (memcmp(obj->guard, obj->GuardBytes, sizeof(obj->guard)) != 0) {
      if (error != NULL)
        *error << "Underrun at " << (obj+1) << '[' << obj->size << "] #" << obj->request << endl;
      return false;
    }
  
    if (memcmp((char *)(obj+1)+obj->size, obj->GuardBytes, sizeof(obj->guard)) != 0) {
      if (error != NULL)
        *error << "Overrun at " << (obj+1) << '[' << obj->size << "] #" << obj->request << endl;
      return false;
    }

    obj = obj->next;
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
  Wrapper mem;

  PBoolean ignoreAllocations = (mem->flags&NoLeakPrint) != 0;

  if (ignore)
    mem->flags |= NoLeakPrint;
  else
    mem->flags &= ~NoLeakPrint;

  return ignoreAllocations;
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

void PMemoryHeap::InternalDumpStatistics(ostream & strm)
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

  strm << "\n"
          "Current memory usage     : ";
  OutputMemory(strm, currentMemoryUsage);
  strm << "\n"
          "Current objects count    : " << currentObjects << "\n"
          "Peak memory usage        : ";
  OutputMemory(strm, peakMemoryUsage);
  strm << "\n"
          "Peak objects created     : " << peakObjects << "\n"
          "Total objects created    : " << totalObjects << "\n"
          "Next allocation request  : " << allocationRequest << '\n'
       << endl;
}


void PMemoryHeap::GetState(State & state)
{
  Wrapper mem;
  state.allocationNumber = mem->allocationRequest;
}


void PMemoryHeap::SetAllocationBreakpoint(DWORD point)
{
  allocationBreakpoint = point;
}


void PMemoryHeap::DumpObjectsSince(const State & state)
{
  Wrapper mem;
  if (mem->leakDumpStream != NULL)
    mem->InternalDumpObjectsSince(state.allocationNumber, *mem->leakDumpStream);
}


void PMemoryHeap::DumpObjectsSince(const State & state, ostream & strm)
{
  Wrapper mem;
  mem->InternalDumpObjectsSince(state.allocationNumber, strm);
}


void PMemoryHeap::InternalDumpObjectsSince(DWORD objectNumber, ostream & strm)
{
  bool first = true;
  for (Header * obj = listHead; obj != NULL; obj = obj->next) {
    if (obj->request < objectNumber || (obj->flags&NoLeakPrint) != 0)
      continue;

    if (first && m_state == e_Destroyed) {
      *leakDumpStream << "\nMemory leaks detected, press Enter to display . . ." << flush;
#if !defined(_WIN32)
      cin.get();
#endif
      first = false;
    }

    BYTE * data = (BYTE *)&obj[1];

    if (obj->fileName != NULL)
      strm << obj->fileName << '(' << obj->line << ") : ";

    strm << '#' << obj->request << ' ' << (void *)data << " [" << obj->size << "] ";

    if (obj->className != NULL)
      strm << "class=\"" << obj->className << "\" ";

    if (PProcess::IsInitialised() && obj->threadId != PNullThreadIdentifier) {
      strm << "thread=";
      PThread * thread = PProcess::Current().GetThread(obj->threadId);
      if (thread != NULL)
        strm << '"' << thread->GetThreadName() << "\" ";
      else
        strm << "0x" << hex << obj->threadId << dec << ' ';
    }

    strm << '\n' << hex << setfill('0') << PBYTEArray(data, std::min(MaxMemoryDumBytes, obj->size), false)
                 << dec << setfill(' ') << endl;
  }
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


PBoolean PMemoryHeap::SetIgnoreAllocations(PBoolean ignore)
{
  CreateInstance();
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  if (ignore)
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


void PMemoryHeap::SetAllocationBreakpoint(DWORD objectNumber)
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

#if P_PROFILING
// Currently only supported in GNU && *nix

namespace PProfiling
{

  #if defined(__i386__) || defined(__x86_64__)
    void GetFrequency(uint64_t & freq)
    {
      freq = 2000000000; // 2GHz
      ifstream cpuinfo("/proc/cpuinfo", ios::in);
      while (cpuinfo.good()) {
        char line[100];
        cpuinfo.getline(line, sizeof(line));
        if (strncmp(line, "cpu MHz", 7) == 0) {
          freq = (uint64_t)(atof(strchr(line, ':')+1)*1000000);
          break;
        }
      }
    }
  #elif defined(_M_IX86) || defined(_M_X64)
    void GetFrequency(uint64_t & freq)
    {
      LARGE_INTEGER li;
      QueryPerformanceFrequency(&li);
      freq = li.QuadPart*1000;
    }
  #elif defined(_WIN32)
    void GetFrequency(uint64_t & freq)
    {
      LARGE_INTEGER li;
      QueryPerformanceFrequency(&li);
      freq = li.QuadPart;
    }
  #elif defined(CLOCK_MONOTONIC)
    void GetFrequency(uint64_t & freq)
    {
      freq = 1000000000ULL;
    }
  #else
    void GetFrequency(uint64_t & freq)
    {
      freq = 1000000ULL;
    }
  #endif


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

    uint64_t m_start;
    bool     m_enabled;
    atomic<FunctionRawData *> m_functions;
    atomic<ThreadRawData *>   m_threads;
  };
  static Database s_database;


  /////////////////////////////////////////////////////////////////////

  FunctionRawData::FunctionRawData(FunctionType type, void * function, void * caller)
    : m_type(type)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
  {
    m_function.m_pointer = function;
    m_function.m_caller = caller;

    PProfilingGetCycles(m_when);
    m_link = s_database.m_functions.exchange(this);
  }


  FunctionRawData::FunctionRawData(FunctionType type, const char * name, const char * file, unsigned line)
    : m_type(type)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
  {
    m_function.m_name = name;
    m_function.m_file = file;
    m_function.m_line = line;

    PProfilingGetCycles(m_when);
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
  {
    PProfilingGetCycles(m_start);
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
    PProfilingGetCycles(s_database.m_start);
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
      CpuTime(const Analysis & analysis, uint64_t cycles)
        : m_cycles(cycles)
        , m_time(analysis.CyclesToSeconds(cycles))
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


  float Analysis::CyclesToSeconds(uint64_t cycles) const
  {
    return (float)((double)cycles / m_frequency);
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
        std::streamsize len = func->first.length();
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
            " frequency=" << m_frequency      << ","
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
              << " min=" << setw(24) << CpuTime(*this, func->second.m_minimum)
              << " max=" << setw(24) << CpuTime(*this, func->second.m_maximum)
              << " avg=" << setw(24) << CpuTime(*this, avg);
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
         << "<td align=center>" << m_frequency
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
               << "<td align=center nowrap>" << CpuTime(*this, func->second.m_minimum)
               << "<td align=center nowrap>" << CpuTime(*this, func->second.m_maximum)
               << "<td align=center nowrap>" << CpuTime(*this, avg);
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
    PProfilingGetCycles(analysis.m_durationCycles);
    analysis.m_durationCycles -= s_database.m_start;

    GetFrequency(analysis.m_frequency);

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
          PThread::Times times;
          PThread::GetTimes(entry->m_threadIdentifier, times);
          thrd = AddThreadByID(analysis.m_threadByID, times);
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
};

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

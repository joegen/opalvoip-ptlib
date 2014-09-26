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
    str << ", Error=" << err;
  str << ends;
  
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

  object = ptr.object;
  if (object != NULL)
    ++object->referenceCount;

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
#elif defined(P_VXWORKS)
  semTake((SEM_ID)instance->mutex, WAIT_FOREVER);
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
#elif defined(P_VXWORKS)
  semGive((SEM_ID)instance->mutex);
#endif
}


PMemoryHeap::PMemoryHeap()
{
  isDestroyed = false;

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
  isDestroyed = true;

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

  if (mem->isDestroyed)
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

  if (mem->isDestroyed) {
    free(obj);
    return;
  }

  switch (mem->InternalValidate(ptr, className, mem->leakDumpStream)) {
    case Ok :
      break;
    case Trashed :
      free(ptr);
      return;
    case Bad :
      free(obj);
      return;
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


PMemoryHeap::Validation PMemoryHeap::InternalValidate(const void * ptr,
                                                      const char * className,
                                                      ostream * error)
{
  if (isDestroyed)
    return Bad;

  if (ptr == NULL)
    return Trashed;

  Header * obj = ((Header *)ptr)-1;

  unsigned count = currentObjects;
  Header * link = listTail;  
  while (link != NULL && link != obj && count-- > 0) 
    link = link->prev;  

  if (link != obj) {
    if (error != NULL)
      *error << "Block " << ptr << " not in heap!" << endl;
    return Trashed;
  }

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
  
  if (!(className == NULL && obj->className == NULL) &&
       (className == NULL || obj->className == NULL ||
       (className != obj->className && strcmp(obj->className, className) != 0))) {
    if (error != NULL)
      *error << "PObject " << ptr << '[' << obj->size << "] #" << obj->request
             << " allocated as \"" << (obj->className != NULL ? obj->className : "<NULL>")
             << "\" and should be \"" << (className != NULL ? className : "<NULL>")
             << "\"." << endl;
    return Bad;
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
       << "\nNext allocation request: " << allocationRequest
       << '\n' << endl;
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

    if (first && isDestroyed) {
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

#include <fstream>

namespace PProfiling
{

#if defined(__i386__) || defined(__x86_64__)
#define GetTimestamp(when) { uint32_t l,h; __asm__ __volatile__ ("rdtsc" : "=a"(l), "=d"(h)); when = ((uint64_t)h<<32)|l; }
PPROFILE_EXCLUDE(static void GetFrequency(uint64_t & freq));
static void GetFrequency(uint64_t & freq)
{
  freq = 2500000000; // 2.5GHz pretty typical
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
#define GetTimestamp(when) when = __rdtsc()
#define GetFrequency(freq) { LARGE_INTEGER li; QueryPerformanceFrequency(&li); freq = li.QuadPart*1000; }
#elif defined(_WIN32)
#define GetTimestamp(when) { LARGE_INTEGER li; QueryPerformanceCounter(&li); when = li.QuadPart; }
#define GetFrequency(freq) { LARGE_INTEGER li; QueryPerformanceFrequency(&li); freq = li.QuadPart; }
#else
#define GetTimestamp(when) { timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); when = t.tv_sec*1000000000ULL+t.tv_nsec; }
#define GetFrequency(freq) freq = 1000000000ULL
#endif


  enum FunctionType
  {
    e_AutoEntry,
    e_AutoExit,
    e_ManualEntry,
    e_ManualExit,
  };

  struct FunctionInfo
  {
    PPROFILE_EXCLUDE(FunctionInfo(bool entry, void * function, void * caller));
    PPROFILE_EXCLUDE(FunctionInfo(bool entry, const char * name, const char * file, unsigned line));

    PPROFILE_EXCLUDE(FunctionInfo(const FunctionInfo & other));
    PPROFILE_EXCLUDE(FunctionInfo & operator=(const FunctionInfo & other));
    PPROFILE_EXCLUDE(bool operator<(const FunctionInfo & other) const);

    PPROFILE_EXCLUDE(void PrintOn(ostream & out) const);

    union
    {
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
    FunctionInfo          * m_link;
  };


  struct ThreadInfo
  {
    PPROFILE_EXCLUDE(ThreadInfo());
    PPROFILE_EXCLUDE(ThreadInfo(
      PThreadIdentifier threadId,
      PUniqueThreadIdentifier uniqueId,
      const char * name,
      const PTimeInterval & real,
      const PTimeInterval & cpu
    ));
    PPROFILE_EXCLUDE(friend ostream & operator<<(ostream & strm, const ThreadInfo & info));

    PThreadIdentifier       m_threadId;
    PUniqueThreadIdentifier m_uniqueId;
    std::string             m_name;
    PTimeInterval           m_real;
    PTimeInterval           m_cpu;
    ThreadInfo            * m_link;
  };


  struct Database
  {
  public:
    PPROFILE_EXCLUDE(Database());
    PPROFILE_EXCLUDE(~Database());

    uint64_t m_start;
    atomic<FunctionInfo *> m_functions;
    atomic<ThreadInfo *>   m_threads;
  };
  static Database s_database;


  /////////////////////////////////////////////////////////////////////

  FunctionInfo::FunctionInfo(bool entry, void * function, void * caller)
    : m_type(entry ? e_AutoEntry : e_AutoExit)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
  {
    m_function.m_pointer = function;
    m_function.m_caller = caller;

    GetTimestamp(m_when);
    m_link = s_database.m_functions.exchange(this);
  }


  FunctionInfo::FunctionInfo(bool entry, const char * name, const char * file, unsigned line)
    : m_type(entry ? e_ManualEntry : e_ManualExit)
    , m_threadIdentifier(PThread::GetCurrentThreadId())
    , m_threadUniqueId(PThread::GetCurrentUniqueIdentifier())
  {
    m_function.m_name = name;
    m_function.m_file = file;
    m_function.m_line = line;

    GetTimestamp(m_when);
    m_link = s_database.m_functions.exchange(this);
  }


  FunctionInfo::FunctionInfo(const FunctionInfo & other)
    : m_function(other.m_function)
    , m_type(other.m_type)
    , m_threadIdentifier(other.m_threadIdentifier)
    , m_threadUniqueId(other.m_threadUniqueId)
    , m_when(other.m_when)
    , m_link(NULL)
  {
  }


  FunctionInfo & FunctionInfo::operator=(const FunctionInfo & other)
  {
    m_function = other.m_function;
    m_type = other.m_type;
    m_threadIdentifier = other.m_threadIdentifier;
    m_threadUniqueId = other.m_threadUniqueId;
    m_when = other.m_when;
    m_link = NULL;
    return *this;
  }


  bool FunctionInfo::operator<(const FunctionInfo & other) const
  {
    if (m_threadUniqueId < other.m_threadUniqueId)
      return true;
    if (m_threadUniqueId > other.m_threadUniqueId)
      return false;

    switch (m_type) {
      case e_AutoEntry:
      case e_AutoExit:
        return m_function.m_pointer < other.m_function.m_pointer;

      case e_ManualEntry:
      case e_ManualExit:
        return m_function.m_name < other.m_function.m_name;
    }

    return false;
  }


  void FunctionInfo::PrintOn(ostream & out) const
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
    }

    out << '\t' << m_threadUniqueId << '\t' << m_when << '\n';
  }

  Database::Database()
    : m_functions(NULL)
    , m_threads(NULL)
  {
    GetTimestamp(m_start);
  }


  Database::~Database()
  {
    if (static_cast<FunctionInfo *>(m_functions) == NULL)
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
        Analyse(out);
    }

    Reset();
  }


  void Reset()
  {
    FunctionInfo * func = s_database.m_functions.exchange(NULL);
    while (func != NULL) {
      FunctionInfo * del = func;
      func = func->m_link;
      delete del;
    }

    ThreadInfo * thrd = s_database.m_threads.exchange(NULL);
    while (thrd != NULL) {
      ThreadInfo * del = thrd;
      thrd = thrd->m_link;
      delete del;
    }
  }


  void OnThreadEnded(const PThread & thread, const PTimeInterval & real, const PTimeInterval & cpu)
  {
    ThreadInfo * info = new ThreadInfo(thread.GetThreadId(), thread.GetUniqueIdentifier(), thread.GetThreadName(), real, cpu);
    info->m_link = s_database.m_threads.exchange(info);
  }


  ThreadInfo::ThreadInfo()
    : m_threadId(PNullThreadIdentifier)
    , m_uniqueId(0)
  {
  }


  ThreadInfo::ThreadInfo(PThreadIdentifier threadId,
                         PUniqueThreadIdentifier uniqueId,
                         const char * name,
                         const PTimeInterval & real,
                         const PTimeInterval & cpu)
    : m_threadId(threadId)
    , m_uniqueId(uniqueId)
    , m_name(name)
    , m_real(real)
    , m_cpu(cpu)
  {
  }


  ostream & operator<<(ostream & strm, const ThreadInfo & info)
  {
    std::streamsize nameWidth = strm.width();
    strm << setw(0) << "Thread \"" << info.m_name << left << setw(nameWidth - info.m_name.length()) << '"';
    if (info.m_threadId != info.m_uniqueId)
      strm <<   "  id=" << left << setw(8) << info.m_uniqueId;
    strm   << "  real=" << scientific << setprecision(3) << setw(10) << info.m_real
           <<  "  cpu=" << scientific << setprecision(3) << setw(10) << info.m_cpu;
    if (info.m_real > 0)
      strm <<  " ("  <<   fixed    << setprecision(2) << (100.0*info.m_cpu.GetMilliSeconds()/info.m_real.GetMilliSeconds()) << "%)";
    return strm;
  }


  Block::Block(const char * name, const char * file, unsigned line)
    : m_name(name)
  {
    new FunctionInfo(true, name, file, line);
  }


  Block::~Block()
  {
    new FunctionInfo(false, m_name, NULL, 0);
  }


  void Dump(ostream & strm)
  {
    for (FunctionInfo * info = s_database.m_functions; info != NULL; info = info->m_link)
      info->PrintOn(strm);
  }


  struct Accumulator
  {
    unsigned m_count;
    uint64_t m_sum;
    uint64_t m_minimum;
    uint64_t m_maximum;

    PPROFILE_EXCLUDE(Accumulator());
    PPROFILE_EXCLUDE(void Accumulate(FunctionInfo * entry, FunctionInfo * exit));
  };

  Accumulator::Accumulator()
    : m_count(0)
    , m_sum(0)
    , m_minimum(std::numeric_limits<uint64_t>::max())
    , m_maximum(0)
  {
  }

  void Accumulator::Accumulate(FunctionInfo * entry, FunctionInfo * exit)
  {
    uint64_t diff = exit->m_when - entry->m_when;

    if (m_minimum > diff)
      m_minimum = diff;
    if (m_maximum < diff)
      m_maximum = diff;

    m_sum += diff;
    ++m_count;
  }

  PPROFILE_EXCLUDE(static std::string FormatTime(uint64_t cycles, uint64_t frequency));
  static std::string FormatTime(uint64_t cycles, uint64_t frequency)
  {
    std::stringstream strm;
    strm << cycles << " (" << fixed << setprecision(3) << (1000.0*cycles / frequency) << "ms)";
    return strm.str();
  }

  void Analyse(ostream & strm)
  {
    uint64_t duration;
    GetTimestamp(duration);
    duration -= s_database.m_start;

    std::map<FunctionInfo, Accumulator> accumulators;

    for (FunctionInfo * exit = s_database.m_functions; exit != NULL; exit = exit->m_link) {
      switch (exit->m_type) {
        case e_AutoExit :
          for (FunctionInfo * entry = exit->m_link; entry != NULL; entry = entry->m_link) {
            if (entry->m_function.m_pointer == exit->m_function.m_pointer && entry->m_threadUniqueId == exit->m_threadUniqueId) {
              accumulators[*entry].Accumulate(entry, exit);
              break;
            }
          }
          break;

        case e_ManualExit :
          for (FunctionInfo * entry = exit->m_link; entry != NULL; entry = entry->m_link) {
            if (entry->m_function.m_name == exit->m_function.m_name && entry->m_threadUniqueId == exit->m_threadUniqueId) {
              accumulators[*entry].Accumulate(entry, exit);
              break;
            }
          }
      }
    }

    std::streamsize threadNameWidth = 0;
    map<PUniqueThreadIdentifier, ThreadInfo> threads;
    for (ThreadInfo * thrd = s_database.m_threads; thrd != NULL; thrd = thrd->m_link) {
      threads[thrd->m_uniqueId] = *thrd;

      std::streamsize len = thrd->m_name.length();
      if (len > threadNameWidth)
        threadNameWidth = len;
    }
    threadNameWidth += 3;

    std::streamsize functionNameWidth = 0;
    for (std::map<FunctionInfo, Accumulator>::iterator it = accumulators.begin(); it != accumulators.end(); ++it) {
      std::streamsize len = it->first.m_type == e_ManualEntry ? strlen(it->first.m_function.m_name) : 18;
      if (len > functionNameWidth)
        functionNameWidth = len;
    }
    functionNameWidth += 2;

    PUniqueThreadIdentifier lastId = 0;

    uint64_t frequency;
    GetFrequency(frequency);

    strm << "Summary profile: " << threads.size() << " completed threads, "
         << accumulators.size() << " functions,"
            " cycles=" << duration << ","
            " frequency=" << frequency << ","
            " time=" << PTimeInterval(int64_t(1000.0*duration/frequency)) << '\n'
         << fixed;
    for (std::map<FunctionInfo, Accumulator>::iterator it = accumulators.begin(); it != accumulators.end(); ++it) {
      PTimeInterval threadTime;
      if (lastId != it->first.m_threadUniqueId) {
        lastId = it->first.m_threadUniqueId;
        strm << "\n  ";
        map<PUniqueThreadIdentifier, ThreadInfo>::iterator thrd = threads.find(lastId);
        if (thrd != threads.end()) {
          strm << setw(threadNameWidth) << thrd->second;
          threadTime = thrd->second.m_real;
          threads.erase(thrd);
        }
        else {
          PThread::Times times;
          if (PThread::GetTimes(it->first.m_threadIdentifier, times))
            strm << setw(threadNameWidth)
                 << ThreadInfo(it->first.m_threadIdentifier,
                               it->first.m_threadUniqueId,
                               PThread::GetThreadName(it->first.m_threadIdentifier),
                               times.m_real, times.m_kernel + times.m_user)
                 << "  ** Running **";
          else {
            strm << "Thread info not available: id=" << it->first.m_threadIdentifier;
            if (it->first.m_threadIdentifier != lastId)
              strm << " (" << lastId << ')';
          }
        }
        strm << '\n';
      }
      strm << "    " << left << setw(functionNameWidth);
      if (it->first.m_type == e_ManualEntry)
        strm << it->first.m_function.m_name;
      else
        strm << it->first.m_function.m_pointer;
      uint64_t avg = it->second.m_sum / it->second.m_count;
      strm << " count=" << setw(10) << it->second.m_count
           <<   " min=" << setw(20) << FormatTime(it->second.m_minimum, frequency)
           <<   " max=" << setw(20) << FormatTime(it->second.m_maximum, frequency)
           <<   " avg=" << setw(20) << FormatTime(avg                 , frequency);
      if (threadTime > 0)
        strm << " (" << fixed << setprecision(2) << (100000.0*it->second.m_sum/frequency/threadTime.GetMilliSeconds()) << "%)";
      strm << '\n';
    }

    for ( map<PUniqueThreadIdentifier, ThreadInfo>::iterator it = threads.begin(); it != threads.end(); ++it)
      strm << "\n  " << setw(threadNameWidth) << it->second;
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
    new PProfiling::FunctionInfo(true, function, caller);
  }

  void __cyg_profile_func_exit(void * function, void * caller)
  {
    new PProfiling::FunctionInfo(false, function, caller);
  }
};
#endif // __GNUC__

#endif // P_PROFILING


// End Of File ///////////////////////////////////////////////////////////////

/*
 * safecoll.cxx
 *
 * Thread safe collection classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 */

#ifdef __GNUC__
#pragma implementation "safecoll.h"
#endif

#include <ptlib.h>
#include <ptlib/safecoll.h>


#define PTraceModule() "SafeColl"
#define new PNEW


/////////////////////////////////////////////////////////////////////////////

PSafeObject::PSafeObject(PSafeObject * indirectLock)
  : m_safeReferenceCount(0)
  , m_safelyBeingRemoved(false)
  , m_safeInUse(indirectLock != NULL ? indirectLock->m_safeInUse : &m_safeInUseMutex)
{
}


PSafeObject::PSafeObject(PReadWriteMutex & mutex)
  : m_safeReferenceCount(0)
  , m_safelyBeingRemoved(false)
  , m_safeInUse(&mutex)
{
}


PBoolean PSafeObject::SafeReference()
{
#if PTRACING
  unsigned count = 0;
  {
    PWaitAndSignal mutex(m_safetyMutex);
    if (!m_safelyBeingRemoved)
      count = ++m_safeReferenceCount;
  }

  unsigned level = count == 0  || m_traceContextIdentifier == 1234567890 ? 3 : 7;
  if (PTrace::CanTrace(level)) {
    ostream & trace = PTRACE_BEGIN(level);
    trace << GetClass() << ' ' << (void *)this;
    if (count != 0)
      trace << " incremented reference count to " << count;
    else {
      trace << " removed, cannot reference";
      if (PTrace::CanTrace(6))
        PTrace::WalkStack(trace);
    }
    trace << PTrace::End;
  }
  return count > 0;
#else
  PWaitAndSignal mutex(m_safetyMutex);
  if (m_safelyBeingRemoved)
    return false;
  ++m_safeReferenceCount;
  return true;
#endif
}


PBoolean PSafeObject::SafeDereference()
{
  bool mayBeDeleted = false;

  m_safetyMutex.Wait();
  if (PAssert(m_safeReferenceCount > 0, PLogicError)) {
    m_safeReferenceCount--;
    mayBeDeleted = m_safeReferenceCount == 0 && !m_safelyBeingRemoved;
  }
#if PTRACING
  unsigned count = m_safeReferenceCount;
#endif
  m_safetyMutex.Signal();

  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7,
         GetClass() << ' ' << (void *)this << " decremented reference count to " << count);

  // At this point the object could be deleted in another trhead, do not use it anymore!
  return mayBeDeleted;
}


PBoolean PSafeObject::LockReadOnly() const
{
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Waiting read ("<<(void *)this<<")");
  m_safetyMutex.Wait();

  if (m_safelyBeingRemoved) {
    m_safetyMutex.Signal();
    PTRACE(6, "Being removed while waiting read ("<<(void *)this<<")");
    return false;
  }

  if (m_safeInUse->m_fileOrName == NULL)
      m_safeInUse->m_fileOrName = typeid(*this).name();

  m_safetyMutex.Signal();

  m_safeInUse->StartRead();
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Locked read ("<<(void *)this<<")");
  return true;
}


void PSafeObject::UnlockReadOnly() const
{
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Unlocked read ("<<(void *)this<<")");
  m_safeInUse->EndRead();
}


PBoolean PSafeObject::LockReadWrite()
{
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Waiting readWrite ("<<(void *)this<<")");
  m_safetyMutex.Wait();

  if (m_safelyBeingRemoved) {
    m_safetyMutex.Signal();
    PTRACE(6, "Being removed while waiting readWrite ("<<(void *)this<<")");
    return false;
  }

  if (m_safeInUse->m_fileOrName == NULL)
      m_safeInUse->m_fileOrName = typeid(*this).name();

  m_safetyMutex.Signal();

  m_safeInUse->StartWrite();
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Locked readWrite ("<<(void *)this<<")");
  return true;
}


void PSafeObject::UnlockReadWrite()
{
  PTRACE(m_traceContextIdentifier == 1234567890 ? 3 : 7, "Unlocked readWrite ("<<(void *)this<<")");
  m_safeInUse->EndWrite();
}


void PSafeObject::SafeRemove()
{
  m_safetyMutex.Wait();
  m_safelyBeingRemoved = true;
  m_safetyMutex.Signal();
}


PBoolean PSafeObject::SafelyCanBeDeleted() const
{
  PWaitAndSignal mutex(m_safetyMutex);
  return m_safelyBeingRemoved && m_safeReferenceCount == 0;
}


bool PSafeObject::GarbageCollection()
{
  return true;
}


/////////////////////////////////////////////////////////////////////////////

PSafeLockReadOnly::PSafeLockReadOnly(const PSafeObject & object)
  : m_safeObject(const_cast<PSafeObject &>(object))
  , m_locked(object.LockReadOnly())
{
}


PSafeLockReadOnly::~PSafeLockReadOnly()
{
  if (m_locked)
    m_safeObject.UnlockReadOnly();
}


PBoolean PSafeLockReadOnly::Lock()
{
  m_locked = m_safeObject.LockReadOnly();
  return m_locked;
}


void PSafeLockReadOnly::Unlock()
{
  if (m_locked) {
    m_safeObject.UnlockReadOnly();
    m_locked = false;
  }
}



/////////////////////////////////////////////////////////////////////////////

PSafeLockReadWrite::PSafeLockReadWrite(const PSafeObject & object)
  : m_safeObject(const_cast<PSafeObject &>(object))
  , m_locked(m_safeObject.LockReadWrite())
{
}


PSafeLockReadWrite::~PSafeLockReadWrite()
{
  if (m_locked)
    m_safeObject.UnlockReadWrite();
}


PBoolean PSafeLockReadWrite::Lock()
{
  m_locked = m_safeObject.LockReadWrite();
  return m_locked;
}


void PSafeLockReadWrite::Unlock()
{
  if (m_locked) {
    m_safeObject.UnlockReadWrite();
    m_locked = false;
  }
}


/////////////////////////////////////////////////////////////////////////////

PSafeCollection::PSafeCollection(PCollection * coll)
  : m_collection(PAssertNULL(coll))
  , m_deleteObjects(true)
#if P_TIMERS
  , m_deleteObjectsTimer(NULL)
#endif
{
  m_collection->DisallowDeleteObjects();
  m_toBeRemoved.DisallowDeleteObjects();
}


PSafeCollection::~PSafeCollection()
{
#if P_TIMERS
  delete m_deleteObjectsTimer;
#endif

  RemoveAll();

  /* Delete objects moved to deleted list in RemoveAll(), we don't use
     DeleteObjectsToBeRemoved() as that will do a garbage collection which might
     prevent deletion. Need to be a bit more forceful here. */
  for (PList<PSafeObject>::iterator i = m_toBeRemoved.begin(); i != m_toBeRemoved.end(); ++i) {
    i->GarbageCollection();
    if (i->SafelyCanBeDeleted())
      delete &*i;
    else {
      // If anything still has a PSafePtr .. "detach" it from the collection so
      // will be deleted whan that PSafePtr finally goes out of scope.
      i->m_safelyBeingRemoved = false;
    }
  }

  delete m_collection;
}


PBoolean PSafeCollection::SafeRemove(PSafeObject * obj)
{
  if (obj == NULL)
    return false;

  PWaitAndSignal mutex(m_collectionMutex);
  if (!m_collection->Remove(obj))
    return false;

  SafeRemoveObject(obj);
  return true;
}


PBoolean PSafeCollection::SafeRemoveAt(PINDEX idx)
{
  PWaitAndSignal mutex(m_collectionMutex);
  PSafeObject * obj = PDownCast(PSafeObject, m_collection->RemoveAt(idx));
  if (obj == NULL)
    return false;

  SafeRemoveObject(obj);
  return true;
}


void PSafeCollection::PrintOn(ostream &strm) const
{
  m_collectionMutex.Wait();
  if (m_collection != NULL)
    m_collection->PrintOn(strm);
  m_collectionMutex.Signal();
}


void PSafeCollection::RemoveAll(PBoolean synchronous)
{
  m_collectionMutex.Wait();

  while (m_collection->GetSize() > 0)
    SafeRemoveObject(PDownCast(PSafeObject, m_collection->RemoveAt(0)));

  m_collectionMutex.Signal();

  if (synchronous) {
    // Have unfortunate busy loop here, but it should be very
    // rare that it will be here for long
    while (!DeleteObjectsToBeRemoved())
      PThread::Sleep(100);
  }
}


bool PSafeCollection::SafeAddObject(PSafeObject * obj, PSafeObject * old)
{
  if (obj == old)
    return false;

  SafeRemoveObject(old);

  if (obj == NULL)
    return false;

  return PAssert(m_collection->GetObjectsIndex(obj) == P_MAX_INDEX, "Cannot insert safe object twice") && obj->SafeReference();
}


void PSafeCollection::SafeRemoveObject(PSafeObject * obj)
{
  if (obj == NULL)
    return;

  // Make sure SfeRemove() called before SafeDereference() to avoid race condition
  if (m_deleteObjects) {
    obj->SafeRemove();

    m_removalMutex.Wait();
    m_toBeRemoved.Append(obj);
    m_removalMutex.Signal();
  }

  /* Even though we are marked as not to delete objects, we still need to obey
     the rule that if there are no references left the object is deleted. If
     the object is still "owned" by a PSafeCollection that has NOT got
     deleteObjects false, then SafeDereference returns false so we don't delete
     is here. If there are no PSafePtr()s or PSafeCollections()'s anywhere we
     need to delete it.
     */
  if (obj->SafeDereference() && !m_deleteObjects)
    delete obj;
}


PBoolean PSafeCollection::DeleteObjectsToBeRemoved()
{
  PWaitAndSignal lock(m_removalMutex);

  PList<PSafeObject>::iterator it = m_toBeRemoved.begin();
  while (it != m_toBeRemoved.end()) {
    if (it->GarbageCollection() && it->SafelyCanBeDeleted()) {
      PObject * obj = &*it;
      m_toBeRemoved.Remove(obj);
      m_removalMutex.Signal();
      DeleteObject(obj);
      m_removalMutex.Wait();

      it = m_toBeRemoved.begin(); // Restart looking through list
    }
    else
      ++it;
  }

  return m_toBeRemoved.IsEmpty() && m_collection->IsEmpty();
}


void PSafeCollection::DeleteObject(PObject * object) const
{
  delete object;
}


void PSafeCollection::SetAutoDeleteObjects()
{
#if P_TIMERS
  if (m_deleteObjectsTimer != NULL)
    return;

  m_deleteObjectsTimer = new PTimer();
  m_deleteObjectsTimer->SetNotifier(PCREATE_NOTIFIER(DeleteObjectsTimeout), "SafeDelObj");
  m_deleteObjectsTimer->RunContinuous(1000); // Every second
#endif
}


#if P_TIMERS
void PSafeCollection::DeleteObjectsTimeout(PTimer &, P_INT_PTR)
{
  DeleteObjectsToBeRemoved();
}
#endif


PINDEX PSafeCollection::GetSize() const
{
  PWaitAndSignal lock(m_collectionMutex);
  return m_collection->GetSize();
}


void PSafeCollection::CopySafeCollection(PCollection * other)
{
  DisallowDeleteObjects();

  for (PINDEX i = 0; i < other->GetSize(); ++i) {
    PSafeObject * obj = dynamic_cast<PSafeObject *>(other->GetAt(i));
    if (obj != NULL && obj->SafeReference())
      m_collection->Append(obj);
  }
}


void PSafeCollection::CopySafeDictionary(PAbstractDictionary * other)
{
  DisallowDeleteObjects();

  for (PINDEX i = 0; i < other->GetSize(); ++i) {
    PSafeObject * obj = dynamic_cast<PSafeObject *>(&other->AbstractGetDataAt(i));
    if (obj != NULL && obj->SafeReference())
      m_collection->Insert(other->AbstractGetKeyAt(i), obj);
  }
}


/////////////////////////////////////////////////////////////////////////////

PSafePtrBase::PSafePtrBase(PSafeObject * obj, PSafetyMode mode)
  : m_collection(NULL)
  , m_currentObject(obj)
  , m_lockMode(mode)
{
  EnterSafetyMode(WithReference);
}


PSafePtrBase::PSafePtrBase(const PSafeCollection & safeCollection,
                           PSafetyMode mode,
                           PINDEX idx)
  : m_collection(safeCollection.CloneAs<PSafeCollection>())
  , m_currentObject(NULL)
  , m_lockMode(mode)
{
  Assign(idx);
}


PSafePtrBase::PSafePtrBase(const PSafeCollection & safeCollection,
                           PSafetyMode mode,
                           PSafeObject * obj)
  : m_collection(safeCollection.CloneAs<PSafeCollection>())
  , m_currentObject(NULL)
  , m_lockMode(mode)
{
  Assign(obj);
}


PSafePtrBase::PSafePtrBase(const PSafePtrBase & enumerator)
  : m_collection(enumerator.m_collection != NULL ? enumerator.m_collection->CloneAs<PSafeCollection>() : NULL)
  , m_currentObject(enumerator.m_currentObject)
  , m_lockMode(enumerator.m_lockMode)
{
  EnterSafetyMode(WithReference);
}


PSafePtrBase::~PSafePtrBase()
{
  ExitSafetyMode(WithDereference);
  delete m_collection;
}


PObject::Comparison PSafePtrBase::Compare(const PObject & obj) const
{
  const PSafePtrBase * other = PDownCast(const PSafePtrBase, &obj);
  if (other == NULL)
    return GreaterThan;

  if (m_currentObject < other->m_currentObject)
    return LessThan;
  if (m_currentObject > other->m_currentObject)
    return GreaterThan;
  return EqualTo;
}


void PSafePtrBase::PrintOn(ostream &strm) const
{
  strm << m_currentObject;
}


void PSafePtrBase::Assign(const PSafePtrBase & enumerator)
{
  if (this == &enumerator)
    return;

  // lockCount ends up zero after this
  ExitSafetyMode(WithDereference);

  delete m_collection;
  m_collection = enumerator.m_collection != NULL ? enumerator.m_collection->CloneAs<PSafeCollection>() : NULL;
  m_currentObject = enumerator.m_currentObject;
  m_lockMode = enumerator.m_lockMode;

  EnterSafetyMode(WithReference);
}


void PSafePtrBase::Assign(const PSafeCollection & safeCollection)
{
  // lockCount ends up zero after this
  ExitSafetyMode(WithDereference);

  delete m_collection;
  m_collection = safeCollection.CloneAs<PSafeCollection>();
  m_currentObject = NULL;
  m_lockMode = PSafeReadWrite;

  Assign((PINDEX)0);
}


void PSafePtrBase::Assign(PSafeObject * newObj)
{
  ExitSafetyMode(WithDereference);

  m_currentObject = newObj;

  if (newObj == NULL)
    return;

  if (m_collection == NULL) {
    m_lockMode = PSafeReference;
    if (!EnterSafetyMode(WithReference))
      m_currentObject = NULL;
    return;
  }

  m_collection->m_collectionMutex.Wait();

  if (m_collection->m_collection->GetObjectsIndex(newObj) == P_MAX_INDEX) {
    m_collection->m_collectionMutex.Signal();
    delete m_collection;
    m_collection = NULL;
    m_lockMode = PSafeReference;
    if (!EnterSafetyMode(WithReference))
      m_currentObject = NULL;
  }
  else {
    if (!newObj->SafeReference())
      m_currentObject = NULL;
    m_collection->m_collectionMutex.Signal();
    EnterSafetyMode(AlreadyReferenced);
  }
}


void PSafePtrBase::Assign(PINDEX idx)
{
  ExitSafetyMode(WithDereference);

  m_currentObject = NULL;

  if (m_collection == NULL)
    return;

  m_collection->m_collectionMutex.Wait();

  while (idx < m_collection->m_collection->GetSize()) {
    m_currentObject = dynamic_cast<PSafeObject *>(m_collection->m_collection->GetAt(idx));
    if (m_currentObject != NULL) {
      if (m_currentObject->SafeReference())
        break;
      m_currentObject = NULL;
    }
    idx++;
  }

  m_collection->m_collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::Next()
{
  if (m_collection == NULL || m_currentObject == NULL)
    return;

  ExitSafetyMode(NoDereference);

  m_collection->m_collectionMutex.Wait();

  PINDEX idx = m_collection->m_collection->GetObjectsIndex(m_currentObject);

  m_currentObject->SafeDereference();
  m_currentObject = NULL;

  if (idx != P_MAX_INDEX) {
    while (++idx < m_collection->m_collection->GetSize()) {
      m_currentObject = dynamic_cast<PSafeObject *>(m_collection->m_collection->GetAt(idx));
      if (m_currentObject != NULL) {
        if (!m_currentObject->IsSafelyBeingRemoved() && m_currentObject->SafeReference())
          break;
        m_currentObject = NULL;
      }
    }
  }

  m_collection->m_collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::Previous()
{
  if (m_collection == NULL || m_currentObject == NULL)
    return;

  ExitSafetyMode(NoDereference);

  m_collection->m_collectionMutex.Wait();

  PINDEX idx = m_collection->m_collection->GetObjectsIndex(m_currentObject);

  m_currentObject->SafeDereference();
  m_currentObject = NULL;

  if (idx != P_MAX_INDEX) {
    while (idx-- > 0) {
      m_currentObject = dynamic_cast<PSafeObject *>(m_collection->m_collection->GetAt(idx));
      if (m_currentObject != NULL) {
        if (!m_currentObject->IsSafelyBeingRemoved() && m_currentObject->SafeReference())
          break;
        m_currentObject = NULL;
      }
    }
  }

  m_collection->m_collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::SetNULL()
{
  // lockCount ends up zero after this
  ExitSafetyMode(WithDereference);

  delete m_collection;
  m_collection = NULL;
  m_currentObject = NULL;
  m_lockMode = PSafeReference;
}


PBoolean PSafePtrBase::SetSafetyMode(PSafetyMode mode)
{
  if (m_lockMode == mode)
    return true;

  ExitSafetyMode(NoDereference);
  m_lockMode = mode;
  return EnterSafetyMode(AlreadyReferenced);
}


PBoolean PSafePtrBase::EnterSafetyMode(EnterSafetyModeOption ref)
{
  if (m_currentObject == NULL)
    return false;

  if (ref == WithReference && !m_currentObject->SafeReference()) {
    m_currentObject = NULL;
    return false;
  }

  switch (m_lockMode) {
    case PSafeReadOnly :
      if (m_currentObject->LockReadOnly())
        return true;
      break;

    case PSafeReadWrite :
      if (m_currentObject->LockReadWrite())
        return true;
      break;

    case PSafeReference :
      return true;
  }

  m_currentObject->SafeDereference();
  m_currentObject = NULL;
  return false;
}


void PSafePtrBase::ExitSafetyMode(ExitSafetyModeOption ref)
{
  if (m_currentObject == NULL)
    return;

  switch (m_lockMode) {
    case PSafeReadOnly :
      m_currentObject->UnlockReadOnly();
      break;

    case PSafeReadWrite :
      m_currentObject->UnlockReadWrite();
      break;

    case PSafeReference :
      break;
  }

  if (ref == WithDereference && m_currentObject->SafeDereference()) {
    PSafeObject * objectToDelete = m_currentObject;
    m_currentObject = NULL;
    DeleteObject(objectToDelete);
  }
}


void PSafePtrBase::DeleteObject(PSafeObject * obj)
{
  if (obj == NULL)
    return;

  PTRACE(6, "Deleting object (" << obj << ')');
  delete obj;
}


/////////////////////////////////////////////////////////////////////////////

PSafePtrMultiThreaded::PSafePtrMultiThreaded(PSafeObject * obj, PSafetyMode mode)
  : PSafePtrBase(NULL, mode)
  , m_objectToDelete(NULL)
{
  LockPtr();

  m_currentObject = obj;
  EnterSafetyMode(WithReference);

  UnlockPtr();
}


PSafePtrMultiThreaded::PSafePtrMultiThreaded(const PSafeCollection & safeCollection,
                                             PSafetyMode mode,
                                             PINDEX idx)
  : PSafePtrBase(NULL, mode)
  , m_objectToDelete(NULL)
{
  LockPtr();

  m_collection = safeCollection.CloneAs<PSafeCollection>();
  Assign(idx);

  UnlockPtr();
}


PSafePtrMultiThreaded::PSafePtrMultiThreaded(const PSafeCollection & safeCollection,
                                             PSafetyMode mode,
                                             PSafeObject * obj)
  : PSafePtrBase(NULL, mode)
  , m_objectToDelete(NULL)
{
  LockPtr();

  m_collection = safeCollection.CloneAs<PSafeCollection>();
  Assign(obj);

  UnlockPtr();
}


PSafePtrMultiThreaded::PSafePtrMultiThreaded(const PSafePtrMultiThreaded & enumerator)
  : m_objectToDelete(NULL)
{
  LockPtr();
  enumerator.m_mutex.Wait();

  m_collection = enumerator.m_collection != NULL ? enumerator.m_collection->CloneAs<PSafeCollection>() : NULL;
  m_currentObject = enumerator.m_currentObject;
  m_lockMode = enumerator.m_lockMode;

  EnterSafetyMode(WithReference);

  enumerator.m_mutex.Signal();
  UnlockPtr();
}


PSafePtrMultiThreaded::~PSafePtrMultiThreaded()
{
  LockPtr();
  ExitSafetyMode(WithDereference);
  m_currentObject = NULL;
  UnlockPtr();
}


PObject::Comparison PSafePtrMultiThreaded::Compare(const PObject & obj) const
{
  PWaitAndSignal mutex(m_mutex);
  return PSafePtrBase::Compare(obj);
}


void PSafePtrMultiThreaded::SetNULL()
{
  LockPtr();
  PSafePtrBase::SetNULL();
  UnlockPtr();
}


PBoolean PSafePtrMultiThreaded::SetSafetyMode(PSafetyMode mode)
{
  PWaitAndSignal mutex(m_mutex);
  return PSafePtrBase::SetSafetyMode(mode);
}


void PSafePtrMultiThreaded::Assign(const PSafePtrMultiThreaded & ptr)
{
  LockPtr();
  ptr.m_mutex.Wait();
  PSafePtrBase::Assign(ptr);
  ptr.m_mutex.Signal();
  UnlockPtr();
}


void PSafePtrMultiThreaded::Assign(const PSafePtrBase & ptr)
{
  LockPtr();
  PSafePtrBase::Assign(ptr);
  UnlockPtr();
}


void PSafePtrMultiThreaded::Assign(const PSafeCollection & safeCollection)
{
  LockPtr();
  PSafePtrBase::Assign(safeCollection);
  UnlockPtr();
}


void PSafePtrMultiThreaded::Assign(PSafeObject * obj)
{
  LockPtr();
  PSafePtrBase::Assign(obj);
  UnlockPtr();
}


void PSafePtrMultiThreaded::Assign(PINDEX idx)
{
  LockPtr();
  PSafePtrBase::Assign(idx);
  UnlockPtr();
}


void PSafePtrMultiThreaded::Next()
{
  LockPtr();
  PSafePtrBase::Next();
  UnlockPtr();
}


void PSafePtrMultiThreaded::Previous()
{
  LockPtr();
  PSafePtrBase::Previous();
  UnlockPtr();
}


void PSafePtrMultiThreaded::DeleteObject(PSafeObject * obj)
{
  m_objectToDelete = obj;
}


void PSafePtrMultiThreaded::UnlockPtr()
{
  PSafeObject * obj = m_objectToDelete;
  m_objectToDelete = NULL;
  m_mutex.Signal();
  PSafePtrBase::DeleteObject(obj);
}


// End of File ///////////////////////////////////////////////////////////////

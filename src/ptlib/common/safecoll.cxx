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
 *
 * $Log: safecoll.cxx,v $
 * Revision 1.3  2002/05/06 00:44:45  robertj
 * Made the lock/unlock read only const so can be used in const functions.
 *
 * Revision 1.2  2002/05/01 04:48:05  robertj
 * GNU compatibility.
 *
 * Revision 1.1  2002/05/01 04:16:44  robertj
 * Added thread safe collection classes.
 *
 */

#ifdef __GNUC__
#pragma implementation "safecoll.h"
#endif

#include <ptlib.h>
#include <ptlib/safecoll.h>


#define new PNEW


/////////////////////////////////////////////////////////////////////////////

PSafeObject::PSafeObject()
{
  safeReferenceCount = 0;
  safelyBeingRemoved = FALSE;
}


BOOL PSafeObject::SafeReference()
{
  PWaitAndSignal mutex(safetyMutex);

  if (safelyBeingRemoved)
    return FALSE;

  safeReferenceCount++;
  return TRUE;
}


void PSafeObject::SafeDereference()
{
  safetyMutex.Wait();
  safeReferenceCount--;
  safetyMutex.Signal();
}


BOOL PSafeObject::LockReadOnly() const
{
  PSafeObject * non_const_this = (PSafeObject *)this;

  non_const_this->safetyMutex.Wait();

  if (safelyBeingRemoved) {
    non_const_this->safetyMutex.Signal();
    return FALSE;
  }

  non_const_this->safetyMutex.Signal();
  non_const_this->safeInUseFlag.StartRead();
  return TRUE;
}


void PSafeObject::UnlockReadOnly() const
{
  ((PSafeObject *)this)->safeInUseFlag.EndRead();
}


BOOL PSafeObject::LockReadWrite()
{
  safetyMutex.Wait();

  if (safelyBeingRemoved) {
    safetyMutex.Signal();
    return FALSE;
  }

  safetyMutex.Signal();
  safeInUseFlag.StartWrite();
  return TRUE;
}


void PSafeObject::UnlockReadWrite()
{
  safeInUseFlag.EndWrite();
}


void PSafeObject::SafeRemove()
{
  safetyMutex.Wait();
  safelyBeingRemoved = TRUE;
  safetyMutex.Signal();
}


BOOL PSafeObject::SafelyCanBeDeleted()
{
  PWaitAndSignal mutex(safetyMutex);
  return safelyBeingRemoved && safeReferenceCount == 0;
}


/////////////////////////////////////////////////////////////////////////////

PSafeCollection::PSafeCollection(PCollection * coll)
{
  collection = coll;
  collection->DisallowDeleteObjects();
  toBeRemoved.DisallowDeleteObjects();
}


PSafeCollection::~PSafeCollection()
{
  deleteObjectsTimer.Stop();

  toBeRemoved.AllowDeleteObjects();
  toBeRemoved.RemoveAll();

  collection->AllowDeleteObjects();
  delete collection;
}


PINDEX PSafeCollection::SafeAppend(PSafeObject * obj)
{
  collectionMutex.Wait();
  PINDEX idx = collection->Append(obj);
  collectionMutex.Signal();
  return idx;
}


BOOL PSafeCollection::SafeRemove(PSafeObject * obj)
{
  if (obj == NULL)
    return FALSE;

  collectionMutex.Wait();

  BOOL ok = collection->Remove(obj);
  if (ok)
    SafeRemoveObject(obj);

  collectionMutex.Signal();

  return ok;
}


PSafeObject * PSafeCollection::SafeRemoveAt(PINDEX idx)
{
  collectionMutex.Wait();
  SafeRemoveObject((PSafeObject *)collection->RemoveAt(idx));
  collectionMutex.Signal();

  return NULL;
}


void PSafeCollection::RemoveAll()
{
  collectionMutex.Wait();

  while (collection->GetSize() > 0)
    SafeRemoveObject((PSafeObject *)collection->RemoveAt(0));

  collectionMutex.Signal();
}


void PSafeCollection::SafeRemoveObject(PSafeObject * obj)
{
  if (obj == NULL)
    return;

  obj->SafeRemove();
  if (obj->SafelyCanBeDeleted())
    delete obj;
  else
    toBeRemoved.Append(obj);
}


void PSafeCollection::DeleteObjectsToBeRemoved()
{
  collectionMutex.Wait();

  PAbstractList toBeDeleted;
  PINDEX i = 0;
  while (i < toBeRemoved.GetSize()) {
    if (((PSafeObject *)toBeRemoved.GetAt(i))->SafelyCanBeDeleted())
      toBeDeleted.Append(toBeRemoved.RemoveAt(i));
    else
      i++;
  }

  collectionMutex.Signal();

  // toBeDeleted goes out of scope so deletes the objects
}


void PSafeCollection::SetAutoDeleteObjects()
{
  if (deleteObjectsTimer.IsRunning())
    return;

  deleteObjectsTimer.SetNotifier(PCREATE_NOTIFIER(DeleteObjectsTimeout));
  deleteObjectsTimer.RunContinuous(1000); // EVery second
}


void PSafeCollection::DeleteObjectsTimeout(PTimer &, INT)
{
  DeleteObjectsToBeRemoved();
}


/////////////////////////////////////////////////////////////////////////////

PSafePtrBase::PSafePtrBase(PSafeObject * obj, PSafetyMode mode)
{
  collection = NULL;
  currentObject = obj;
  lockMode = mode;
  lockThread = NULL;
  lockCount = 0;

  EnterSafetyMode(WithReference);
}


PSafePtrBase::PSafePtrBase(PSafeCollection & safeCollection,
                           PSafetyMode mode,
                           PINDEX idx)
{
  collection = &safeCollection;
  currentObject = NULL;
  lockMode = mode;
  lockThread = NULL;
  lockCount = 0;

  Assign(idx);
}


PSafePtrBase::PSafePtrBase(PSafeCollection & safeCollection,
                           PSafetyMode mode,
                           PSafeObject * obj)
{
  collection = &safeCollection;
  currentObject = NULL;
  lockMode = mode;
  lockThread = NULL;
  lockCount = 0;

  Assign(obj);
}


PSafePtrBase::PSafePtrBase(const PSafePtrBase & enumerator)
{
  collection = enumerator.collection;
  currentObject = enumerator.currentObject;
  lockMode = enumerator.lockMode;
  lockThread = enumerator.lockThread;
  lockCount = enumerator.lockCount;

  EnterSafetyMode(WithReference);
}


PSafePtrBase::~PSafePtrBase()
{
  ExitSafetyMode(WithDereference);
}


PObject::Comparison PSafePtrBase::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(Class()), PInvalidCast);
  PSafeObject * otherObject = ((const PSafePtrBase &)obj).currentObject;
  if (currentObject < otherObject)
    return LessThan;
  if (currentObject > otherObject)
    return GreaterThan;
  return EqualTo;
}


void PSafePtrBase::Assign(const PSafePtrBase & enumerator)
{
  if (this == &enumerator)
    return;

  ExitSafetyMode(WithDereference);

  collection = enumerator.collection;
  currentObject = enumerator.currentObject;
  lockMode = enumerator.lockMode;
  lockThread = enumerator.lockThread;
  lockCount = enumerator.lockCount;

  EnterSafetyMode(WithReference);
}


void PSafePtrBase::Assign(PSafeCollection & safeCollection)
{
  ExitSafetyMode(WithDereference);

  collection = &safeCollection;
  lockMode = PSafeReadWrite;
  lockThread = NULL;
  lockCount = 0;

  Assign((PINDEX)0);
}


void PSafePtrBase::Assign(PSafeObject * newObj)
{
  ExitSafetyMode(WithDereference);

  currentObject = newObj;

  if (newObj == NULL)
    return;

  if (collection == NULL) {
    if (!EnterSafetyMode(WithReference))
      currentObject = NULL;
    return;
  }

  collection->collectionMutex.Wait();

  if (collection->collection->GetObjectsIndex(newObj) == P_MAX_INDEX || !newObj->SafeReference())
    currentObject = NULL;

  collection->collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::Assign(PINDEX idx)
{
  ExitSafetyMode(WithDereference);

  currentObject = NULL;

  if (collection == NULL)
    return;

  collection->collectionMutex.Wait();

  while (idx < collection->collection->GetSize()) {
    currentObject = (PSafeObject *)collection->collection->GetAt(idx);
    if (currentObject != NULL) {
      if (currentObject->SafeReference())
        break;
      currentObject = NULL;
    }
    idx++;
  }

  collection->collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::Next()
{
  if (collection == NULL || currentObject == NULL)
    return;

  ExitSafetyMode(NoDereference);

  collection->collectionMutex.Wait();

  PINDEX idx = collection->collection->GetObjectsIndex(currentObject);

  currentObject->SafeDereference();
  currentObject = NULL;

  if (idx != P_MAX_INDEX) {
    while (++idx < collection->collection->GetSize()) {
      currentObject = (PSafeObject *)collection->collection->GetAt(idx);
      if (currentObject != NULL) {
        if (currentObject->SafeReference())
          break;
        currentObject = NULL;
      }
    }
  }

  collection->collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


void PSafePtrBase::Previous()
{
  if (collection == NULL || currentObject == NULL)
    return;

  ExitSafetyMode(NoDereference);

  collection->collectionMutex.Wait();

  PINDEX idx = collection->collection->GetObjectsIndex(currentObject);

  currentObject->SafeDereference();
  currentObject = NULL;

  if (idx != P_MAX_INDEX) {
    while (idx-- > 0) {
      currentObject = (PSafeObject *)collection->collection->GetAt(idx);
      if (currentObject != NULL) {
        if (currentObject->SafeReference())
          break;
        currentObject = NULL;
      }
    }
  }

  collection->collectionMutex.Signal();

  EnterSafetyMode(AlreadyReferenced);
}


BOOL PSafePtrBase::SetSafetyMode(PSafetyMode mode)
{
  ExitSafetyMode(NoDereference);
  lockMode = mode;
  return EnterSafetyMode(AlreadyReferenced);
}


BOOL PSafePtrBase::EnterSafetyMode(EnterSafetyModeOption ref)
{
  if (currentObject == NULL)
    return FALSE;

  if (ref == WithReference && !currentObject->SafeReference()) {
    currentObject = NULL;
    return FALSE;
  }

  if (lockMode == PSafeReference)
    return TRUE;

  PThread * currentThread = PThread::Current();
  if (lockThread == currentThread) {
    lockCount++;
    return TRUE;
  }

  PAssert(lockThread == NULL, "Invalid use of PSafePtr, locking across threads");
  lockThread = currentThread;
  lockCount = 1;

  if (lockMode == PSafeReadOnly) {
    if (currentObject->LockReadOnly())
      return TRUE;
  }
  else {
    if (currentObject->LockReadWrite())
      return TRUE;
  }

  currentObject->SafeDereference();

  currentObject = NULL;
  lockThread = NULL;
  lockCount = 0;
  return FALSE;
}


void PSafePtrBase::ExitSafetyMode(ExitSafetyModeOption ref)
{
  if (currentObject == NULL)
    return;

  if (lockMode != PSafeReference) {
    if (lockThread != PThread::Current()) {
      PAssertAlways("Invalid use of PSafePtr, unlocking across threads");
      return;
    }

    lockCount--;
    if (lockCount == 0) {
      if (lockMode == PSafeReadOnly)
        currentObject->UnlockReadOnly();
      else
        currentObject->UnlockReadWrite();
      lockThread = NULL;
    }
  }

  if (ref == WithDereference)
    currentObject->SafeDereference();
}


// End of File ///////////////////////////////////////////////////////////////

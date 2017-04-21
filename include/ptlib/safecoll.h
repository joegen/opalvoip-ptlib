/*
 * safecoll.h
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
 
#ifndef PTLIB_SAFE_COLLECTION_H
#define PTLIB_SAFE_COLLECTION_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptlib/syncthrd.h>


#if P_TIMERS
class PTimer;
#endif


/** This class defines a thread-safe object in a collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  The act of adding a new object is simple and only requires locking the
  collection itself during the add.

  Locating an object is more complicated. The obvious lock on the collection
  is made for the initial search. But we wish to have the full collection lock
  for as short a period as possible (for performance reasons) so we lock the
  individual object and release the lock on the collection.

  A simple mutex on the object however is very dangerous as it can be (and
  should be able to be!) locked from other threads independently of the
  collection. If one of these threads subsequently needs to get at the
  collection (eg it wants to remove the object) then we will have a deadlock.
  Also, to avoid a race condition with the object begin deleted, the objects
  lock must be made while the collection lock is set. The performance gains
  are then lost as if something has the object locked for a long time, then
  another object wanting that object will actually lock the collection for a
  long time as well.

  So, an object has 4 states: unused, referenced, reading & writing. With the
  additional rider of "being removed". This flag prevents new locks from being
  acquired and waits for all locks to be relinquished before removing the
  object from the system. This prevents numerous race conditions and accesses
  to deleted objects.

  The "unused" state indicates the object exists in the collection but no
  threads anywhere is using it. It may be moved to any state by any thread
  while in this state. An object cannot be deleted (ie memory deallocated)
  until it is unused.

  The "referenced" state indicates that a thread has a reference (eg pointer)
  to the object and it should not be deleted. It may be locked for reading or
  writing at any time thereafter.

  The "reading" state is a form of lock that indicates that a thread is
  reading from the object but not writing. Multiple threads can obtain a read
  lock. Note the read lock has an implicit "reference" state in it.

  The "writing" state is a form of lock where the data in the object may
  be changed. It can only be obtained exclusively and if there are no read
  locks present. Again there is an implicit reference state in this lock.

  Note that threads going to the "referenced" state may do so regardless of
  the read or write locks present.

  Access to safe objects (especially when in a safe collection) is recommended
  to by the PSafePtr<> class which will manage reference counting and the
  automatic unlocking of objects ones the pointer goes out of scope. It may
  also be used to lock each object of a collection in turn.

  The enumeration of a PSafeCollection of PSafeObjects utilises the PSafePtr
  class in a classic "for loop" manner.

  <CODE>
    for (PSafePtr<MyClass> iter(collection, PSafeReadWrite); iter != NULL; ++iter)
      iter->Process();
  </CODE>

  There is one piece if important behaviour in the above. If while enumerating
  a specic object in the collection, that object is "safely deleted", you are
  guaranteed that the object is still usable and has not been phsyically
  deleted, however it will no longer be in the collection, so the enumeration
  will stop as it can no longer determine where in the collection it was.

  What to do in this case is to take a "snapshot" at a point of time that can be safely and completely
  iterated over:

  <CODE>
    PSafeList<MyClass> collCopy = collection;
    for (PSafePtr<MyClass> iter(callCopy, PSafeReadWrite); iter != NULL; ++iter)
      iter->Process();
  </CODE>

 */
class PSafeObject : public PObject
{
    PCLASSINFO(PSafeObject, PObject);
  public:
  /**@name Construction */
  //@{
    /**Create a thread safe object.
     */
    explicit PSafeObject(
      PSafeObject * indirectLock = NULL ///< Other safe object to be locked when this is locked
    );
    explicit PSafeObject(
      PReadWriteMutex & mutex ///< Mutex to be locked when this is locked
    );
  //@}

  /**@name Operations */
  //@{
    /**Increment the reference count for object.
       This will guarantee that the object is not deleted (ie memory
       deallocated) as the caller thread is using the object, but not
       necessarily at this time locking it.

       If the function returns false, then the object has been flagged for
       deletion and the calling thread should immediately cease using the
       object.

       A typical use of this would be when an entity (eg a thread) has a
       pointer to the object but is not currenty accessing the objects data.
       The LockXXX functions may be called independetly of the reference
       system and the pointer beiong used for the LockXXX call is guaranteed
       to be usable.

       It is recommended that the PSafePtr<> class is used to manage this
       rather than the application calling this function directly.
      */
    PBoolean SafeReference();

    /**Decrement the reference count for object.
       This indicates that the thread no longer has anything to do with the
       object and it may be deleted (ie memory deallocated).

       It is recommended that the PSafePtr<> class is used to manage this
       rather than the application calling this function directly.

       @return true if reference count has reached zero and is not being
               safely deleted elsewhere ie SafeRemove() not called
      */
    PBoolean SafeDereference();

    /**Lock the object for Read Only access.
       This will lock the object in read only mode. Multiple threads may lock
       the object read only, but only one thread can lock for read/write.
       Also, no read only threads can be present for the read/write lock to
       occur and no read/write lock can be present for any read only locks to
       occur.

       If the function returns false, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object, possibly executing the SafeDereference() function to remove
       any references it may have acquired.

       It is expected that the caller had already called the SafeReference()
       function (directly or implicitly) before calling this function. It is
       recommended that the PSafePtr<> class is used to automatically manage
       the reference counting and locking of objects.
      */
    PBoolean LockReadOnly(const PDebugLocation & location = PDebugLocation::None) const;

    /**Release the read only lock on an object.
       Unlock the read only mutex that a thread had obtained. Multiple threads
       may lock the object read only, but only one thread can lock for
       read/write. Also, no read only threads can be present for the
       read/write lock to occur and no read/write lock can be present for any
       read only locks to occur.

       It is recommended that the PSafePtr<> class is used to automatically
       manage the reference counting and unlocking of objects.
      */
    void UnlockReadOnly(const PDebugLocation & location = PDebugLocation::None) const;

    /**Lock the object for Read/Write access.
       This will lock the object in read/write mode. Multiple threads may lock
       the object read only, but only one thread can lock for read/write.
       Also no read only threads can be present for the read/write lock to
       occur and no read/write lock can be present for any read only locks to
       occur.

       If the function returns false, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object, possibly executing the SafeDereference() function to remove
       any references it may have acquired.

       It is expected that the caller had already called the SafeReference()
       function (directly or implicitly) before calling this function. It is
       recommended that the PSafePtr<> class is used to automatically manage
       the reference counting and locking of objects.
      */
    PBoolean LockReadWrite(const PDebugLocation & location = PDebugLocation::None) const;

    /**Release the read/write lock on an object.
       Unlock the read/write mutex that a thread had obtained. Multiple threads
       may lock the object read only, but only one thread can lock for
       read/write. Also, no read only threads can be present for the
       read/write lock to occur and no read/write lock can be present for any
       read only locks to occur.

       It is recommended that the PSafePtr<> class is used to automatically
       manage the reference counting and unlocking of objects.
      */
    void UnlockReadWrite(const PDebugLocation & location = PDebugLocation::None) const;

    /**Set the removed flag.
       This flags the object as beeing removed but does not physically delete
       the memory being used by it. The SafelyCanBeDeleted() can then be used
       to determine when all references to the object have been released so it
       may be safely deleted.

       This is typically used by the PSafeCollection class and is not expected
       to be used directly by an application.
      */
    void SafeRemove();

    /**Indicate the object is being safely removed.
       Note this returns the value outside of any mutexes, so it could change
       at any moment. Care must be exercised in its use.
      */
    unsigned IsSafelyBeingRemoved() const { return m_safelyBeingRemoved; }

    /**Determine if the object can be safely deleted.
       This determines if the object has been flagged for deletion and all
       references to it have been released.

       This is typically used by the PSafeCollection class and is not expected
       to be used directly by an application.
      */
    PBoolean SafelyCanBeDeleted() const;

    /**Do any garbage collection that may be required by the object so that it
       may be finally deleted. This is especially useful if there a references
       back to this object which this object is in charge of disposing of. This
       reference "glare" is to be resolved by this function being called every
       time the owner collection is cleaning up, causing a cascade of clean ups
       that might need to be required.

       Default implementation simply returns true.

       @return true if object may be deleted.
      */
    virtual bool GarbageCollection();

    /**Get count of references to this object.
       Note this returns the value outside of any mutexes, so it could change
       at any moment. Care must be exercised in its use.
      */
    unsigned GetSafeReferenceCount() const { return m_safeReferenceCount; }
  //@}

  private:
    mutable PCriticalSection m_safetyMutex;
    unsigned          m_safeReferenceCount;
    bool              m_safelyBeingRemoved;
    PReadWriteMutex   m_safeInUseMutex;
    PReadWriteMutex * m_safeInUse;

  friend class PSafeCollection;
  friend class PSafePtrBase;
};


class PSafeLockBase
{
  protected:
    typedef bool (PSafeObject:: * LockFn)(const PDebugLocation & location) const;
    typedef void (PSafeObject:: * UnlockFn)(const PDebugLocation & location) const;

    PSafeLockBase(const PSafeObject & object, const PDebugLocation & location, LockFn lock, UnlockFn unlock);

  public:
    ~PSafeLockBase();

    bool Lock();
    void Unlock();

    bool IsLocked() const { return m_locked; }
    bool operator!() const { return !m_locked; }

  protected:
    PSafeObject  & m_safeObject;
    PDebugLocation m_location;
    LockFn         m_lock;
    UnlockFn       m_unlock;
    bool           m_locked;
};


/**Lock a PSafeObject for read only and automatically unlock it when go out of scope.
  */
class PSafeLockReadOnly : public PSafeLockBase
{
  public:
    PSafeLockReadOnly(const PSafeObject & object)
      : PSafeLockBase(object, PDebugLocation::None, &PSafeObject::LockReadOnly, &PSafeObject::UnlockReadOnly)
    { }
};


/**Lock a PSafeObject for read/write and automatically unlock it when go out of scope.
  */
class PSafeLockReadWrite : public PSafeLockBase
{
  public:
    PSafeLockReadWrite(const PSafeObject & object)
      : PSafeLockBase(object, PDebugLocation::None, &PSafeObject::LockReadWrite, &PSafeObject::UnlockReadWrite)
    { }
};


#if PTRACING
  class PInstrumentedSafeLockReadOnly : public PSafeLockBase
  {
    public:
      PInstrumentedSafeLockReadOnly(const PSafeObject & object, const PDebugLocation & location)
        : PSafeLockBase(object, location, &PSafeObject::LockReadOnly, &PSafeObject::UnlockReadOnly)
      { }
  };

  class PInstrumentedSafeLockReadWrite : public PSafeLockBase
  {
    public:
      PInstrumentedSafeLockReadWrite(const PSafeObject & object, const PDebugLocation & location)
        : PSafeLockBase(object, location, &PSafeObject::LockReadWrite, &PSafeObject::UnlockReadWrite)
      { }
  };

  #define P_INSTRUMENTED_LOCK_READ_ONLY2(var, obj)  PInstrumentedSafeLockReadOnly  var((obj), P_DEBUG_LOCATION)
  #define P_INSTRUMENTED_LOCK_READ_WRITE2(var, obj) PInstrumentedSafeLockReadWrite var((obj), P_DEBUG_LOCATION)
#else // P_TRACING
  #define P_INSTRUMENTED_LOCK_READ_ONLY2(var, obj)  PSafeLockReadOnly  var((obj))
  #define P_INSTRUMENTED_LOCK_READ_WRITE2(var, obj) PSafeLockReadWrite var((obj))
#endif // P_TRACING
#define P_READ_WRITE_RETURN_ARG_0()
#define P_READ_WRITE_RETURN_ARG_1(arg) ; if (!lock.IsLocked()) arg
#define P_READ_WRITE_RETURN_PART1(narg, args) P_READ_WRITE_RETURN_PART2(narg, args)
#define P_READ_WRITE_RETURN_PART2(narg, args) P_READ_WRITE_RETURN_ARG_##narg args

#define P_INSTRUMENTED_LOCK_READ_ONLY(...)  P_INSTRUMENTED_LOCK_READ_ONLY2(lock,*this) P_READ_WRITE_RETURN_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))
#define P_INSTRUMENTED_LOCK_READ_WRITE(...) P_INSTRUMENTED_LOCK_READ_WRITE2(lock,*this) P_READ_WRITE_RETURN_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))


/** This class defines a thread-safe collection of objects.
  This class is a wrapper around a standard PCollection class which allows
  only safe, mutexed, access to the collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
class PSafeCollection : public PObject
{
    PCLASSINFO(PSafeCollection, PObject);
  public:
  /**@name Construction */
  //@{
    /**Create a thread safe collection of objects.
       Note the collection is automatically deleted on destruction.
     */
    PSafeCollection(
      PCollection * collection    ///< Actual collection of objects
     );

    /**Destroy the thread safe collection.
       The will delete the collection object provided in the constructor.
      */
    ~PSafeCollection();
  //@}

  /**@name Operations */
  //@{
  protected:
    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PBoolean SafeRemove(
      PSafeObject * obj   ///< Object to remove from collection
    );

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PBoolean SafeRemoveAt(
      PINDEX idx    ///< Object index to remove
    );

  public:
    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator<<</code> function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;

    /**Remove all objects in collection.
      */
    virtual void RemoveAll(
      PBoolean synchronous = false  ///< Wait till objects are deleted before returning
    );

    /**Disallow the automatic delete any objects that have been removed.
       Objects are simply removed from the collection and not marked for
       deletion using PSafeObject::SafeRemove() and DeleteObject().
      */
    void AllowDeleteObjects(
      PBoolean yes = true   ///< New value for flag for deleting objects
    ) { m_deleteObjects = yes; }

    /**Disallow the automatic delete any objects that have been removed.
       Objects are simply removed from the collection and not marked for
       deletion using PSafeObject::SafeRemove() and DeleteObject().
      */
    void DisallowDeleteObjects() { m_deleteObjects = false; }

    /**Delete any objects that have been removed.
       Returns true if all objects in the collection have been removed and
       their pending deletions carried out.
      */
    virtual PBoolean DeleteObjectsToBeRemoved();

    /**Delete an objects that has been removed.
      */
    virtual void DeleteObject(PObject * object) const;

    /**Start a timer to automatically call DeleteObjectsToBeRemoved().
      */
    virtual void SetAutoDeleteObjects();

    /**Get the current size of the collection.
       Note that usefulness of this function is limited as it is merely an
       instantaneous snapshot of the state of the collection.
      */
    PINDEX GetSize() const;

    /**Determine if the collection is empty.
       Note that usefulness of this function is limited as it is merely an
       instantaneous snapshot of the state of the collection.
      */
    PBoolean IsEmpty() const { return GetSize() == 0; }

    /**Get the mutex for the collection.
      */
    const PMutex & GetMutex() const { return m_collectionMutex; }
          PMutex & GetMutex()       { return m_collectionMutex; }
  //@}

  protected:
    void CopySafeCollection(PCollection * other);
    void CopySafeDictionary(PAbstractDictionary * other);
    bool SafeAddObject(PSafeObject * obj, PSafeObject * old);
    void SafeRemoveObject(PSafeObject * obj);

    PCollection      * m_collection;
    mutable PMutex     m_collectionMutex;
    bool               m_deleteObjects;
    PList<PSafeObject> m_toBeRemoved;
    PMutex             m_removalMutex;

#if P_TIMERS
    PDECLARE_NOTIFIER(PTimer, PSafeCollection, DeleteObjectsTimeout);
    PTimer           * m_deleteObjectsTimer;
#endif

  private:
    PSafeCollection(const PSafeCollection & other) : PObject(other) { }
    void operator=(const PSafeCollection &) { }

  friend class PSafePtrBase;
};


enum PSafetyMode {
  PSafeReference,
  PSafeReadOnly,
  PSafeReadWrite
};

/** This class defines a base class for thread-safe pointer to an object.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  NOTE: the PSafePtr will allow safe and mutexed access to objects but is not
  thread safe itself! You should not share PSafePtr instances across threads.

  See the PSafeObject class for more details.
 */
class PSafePtrBase : public PObject
{
    PCLASSINFO(PSafePtrBase, PObject);

  /**@name Construction */
  //@{
  protected:
    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       Note that this version is not associated with a collection so the ++
       and -- operators will not work.
     */
    PSafePtrBase(
      PSafeObject * obj = NULL,         ///< Physical object to point to.
      PSafetyMode mode = PSafeReference ///< Locking mode for the object
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The idx'th entry of the collection is pointed to by this object. If the
       idx is beyond the size of the collection, the pointer is NULL.
     */
    PSafePtrBase(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode,                       ///< Locking mode for the object
      PINDEX idx                              ///< Index into collection to point to
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The obj parameter is only set if it contained in the collection,
       otherwise the pointer is NULL.
     */
    PSafePtrBase(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode,                       ///< Locking mode for the object
      PSafeObject * obj                       ///< Inital object in collection to point to
    );

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtrBase(
      const PSafePtrBase & enumerator   ///< Pointer to copy
    );

  public:
    /**Unlock and dereference the PSafeObject this is pointing to.
      */
    ~PSafePtrBase();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Compare the pointers.
       Note this is not a value comparison and will only return EqualTo if the
       two PSafePtrBase instances are pointing to the same instance.
      */
    virtual Comparison Compare(
      const PObject & obj   ///< Other instance to compare against
    ) const;

    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator<<</code> function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;
  //@}

  /**@name Operations */
  //@{
    /**Set the pointer to NULL, unlocking/dereferencing existing pointer value.
      */
    virtual void SetNULL();

    /**Return true if pointer is NULL.
      */
    bool operator!() const { return m_currentObject == NULL; }

    /**Return pointer to safe object.
      */
    PSafeObject * GetObject() const { return m_currentObject; }

    /**Return pointer to safe object.
      */
    template <class T>
    T * GetObjectAs() const { return dynamic_cast<T *>(m_currentObject); }

    /**Get the locking mode used by this pointer.
      */
    PSafetyMode GetSafetyMode() const { return m_lockMode; }

    /**Change the locking mode used by this pointer.

       If the function returns false, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object. This instance pointer will be set to NULL.
      */
    virtual PBoolean SetSafetyMode(
      PSafetyMode mode  ///< New locking mode
    );
  //@}

    virtual void Assign(const PSafePtrBase & ptr);
    virtual void Assign(const PSafeCollection & safeCollection);
    virtual void Assign(PSafeObject * obj);
    virtual void Assign(PINDEX idx);

  protected:
    virtual void Next();
    virtual void Previous();
    virtual void DeleteObject(PSafeObject * obj);

    enum EnterSafetyModeOption {
      WithReference,
      AlreadyReferenced
    };
    PBoolean EnterSafetyMode(EnterSafetyModeOption ref);

    enum ExitSafetyModeOption {
      WithDereference,
      NoDereference
    };
    void ExitSafetyMode(ExitSafetyModeOption ref);

    virtual void LockPtr() { }
    virtual void UnlockPtr() { }

  protected:
    const PSafeCollection * m_collection;
    PSafeObject           * m_currentObject;
    PSafetyMode             m_lockMode;
};


/** This class defines a base class for thread-safe pointer to an object.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  NOTE: unlikel PSafePtrBase, pointers based on this class are thread safe
  themseleves, at the expense of performance on every operation.

  See the PSafeObject class for more details.
 */
class PSafePtrMultiThreaded : public PSafePtrBase
{
    PCLASSINFO(PSafePtrMultiThreaded, PSafePtrBase);

  /**@name Construction */
  //@{
  protected:
    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       Note that this version is not associated with a collection so the ++
       and -- operators will not work.
     */
    PSafePtrMultiThreaded(
      PSafeObject * obj = NULL,         ///< Physical object to point to.
      PSafetyMode mode = PSafeReference ///< Locking mode for the object
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The idx'th entry of the collection is pointed to by this object. If the
       idx is beyond the size of the collection, the pointer is NULL.
     */
    PSafePtrMultiThreaded(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode,                       ///< Locking mode for the object
      PINDEX idx                              ///< Index into collection to point to
    );

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The obj parameter is only set if it contained in the collection,
       otherwise the pointer is NULL.
     */
    PSafePtrMultiThreaded(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode,                       ///< Locking mode for the object
      PSafeObject * obj                       ///< Inital object in collection to point to
    );

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtrMultiThreaded(
      const PSafePtrMultiThreaded & enumerator   ///< Pointer to copy
    );

  public:
    /**Unlock and dereference the PSafeObject this is pointing to.
      */
    ~PSafePtrMultiThreaded();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Compare the pointers.
       Note this is not a value comparison and will only return EqualTo if the
       two PSafePtrBase instances are pointing to the same instance.
      */
    virtual Comparison Compare(
      const PObject & obj   ///< Other instance to compare against
    ) const;
  //@}

  /**@name Operations */
  //@{
    /**Set the pointer to NULL, unlocking/dereferencing existing pointer value.
      */
    virtual void SetNULL();

    /**Change the locking mode used by this pointer.

       If the function returns false, then the object has been flagged for
       deletion and the calling thread should immediately cease use of the
       object. This instance pointer will be set to NULL.
      */
    virtual PBoolean SetSafetyMode(
      PSafetyMode mode  ///< New locking mode
    );
  //@}

    virtual void Assign(const PSafePtrMultiThreaded & ptr);
    virtual void Assign(const PSafePtrBase & ptr);
    virtual void Assign(const PSafeCollection & safeCollection);
    virtual void Assign(PSafeObject * obj);
    virtual void Assign(PINDEX idx);

  protected:
    virtual void Next();
    virtual void Previous();
    virtual void DeleteObject(PSafeObject * obj);

    virtual void LockPtr() { m_mutex.Wait(); }
    virtual void UnlockPtr();

  protected:
    mutable PMutex m_mutex;
    PSafeObject  * m_objectToDelete;
};


/** This class defines a thread-safe enumeration of object in a collection.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  There are two modes of safe pointer: one that is enumerating a collection;
  and one that is independent of the collection that the safe object is in.
  There are subtle semantics that must be observed in each of these two
  modes especially when switching from one to the other.

  NOTE: the PSafePtr will allow safe and mutexed access to objects but may not
  be thread safe itself! This depends on the base class being used. If the more
  efficient PSafePtrBase class is used you should not share PSafePtr instances
  across threads.

  See the PSafeObject class for more details, especially in regards to
  enumeration of collections.
 */
template <class T, class BaseClass = PSafePtrBase> class PSafePtr : public BaseClass
{
  public:
  /**@name Construction */
  //@{
    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       Note that this version is not associated with a collection so the ++
       and -- operators will not work.
     */
    PSafePtr(
      T * obj = NULL,                   ///< Physical object to point to.
      PSafetyMode mode = PSafeReference ///< Locking mode for the object
    ) : BaseClass(obj, mode) { }

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The idx'th entry of the collection is pointed to by this object. If the
       idx is beyond the size of the collection, the pointer is NULL.
     */
    PSafePtr(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode = PSafeReadWrite,      ///< Locking mode for the object
      PINDEX idx = 0                          ///< Index into collection to point to
    ) : BaseClass(safeCollection, mode, idx) { }

    /**Create a new pointer to a PSafeObject.
       An optional locking mode may be provided to lock the object for reading
       or writing and automatically unlock it on destruction.

       The obj parameter is only set if it contained in the collection,
       otherwise the pointer is NULL.
     */
    PSafePtr(
      const PSafeCollection & safeCollection, ///< Collection pointer will enumerate
      PSafetyMode mode,                       ///< Locking mode for the object
      PSafeObject * obj                       ///< Inital object in collection to point to
    ) : BaseClass(safeCollection, mode, obj) { }

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtr(
      const PSafePtr & ptr   ///< Pointer to copy
    ) : BaseClass(ptr) { }

    /**Copy the pointer to the PSafeObject.
       This will create a copy of the pointer with the same locking mode and
       lock on the PSafeObject. It will also increment the reference count on
       the PSafeObject as well.
      */
    PSafePtr & operator=(const PSafePtr & ptr)
      {
        this->Assign(ptr);
        return *this;
      }

    /**Start an enumerated PSafeObject.
       This will create a read/write locked reference to teh first element in
       the collection.
      */
    PSafePtr & operator=(const PSafeCollection & safeCollection)
      {
        this->Assign(safeCollection);
        return *this;
      }

    /**Set the new pointer to a PSafeObject.
       This will set the pointer to the new object. The old object pointed to
       will be unlocked and dereferenced and the new object referenced.

       If the safe pointer has an associated collection and the new object is
       in that collection, then the object is set to the same locking mode as
       the previous pointer value. This, in effect, jumps the enumeration of a
       collection to the specifed object.

       If the safe pointer has no associated collection or the object is not
       in the associated collection, then the object is always only referenced
       and there is no read only or read/write lock done. In addition any
       associated collection is removed so this becomes a non enumerating
       safe pointer.
     */
    PSafePtr & operator=(T * obj)
      {
        this->Assign(obj);
        return *this;
      }

    /**Set the new pointer to a collection index.
       This will set the pointer to the new object to the index entry in the
       colelction that the pointer was created with. The old object pointed to
       will be unlocked and dereferenced and the new object referenced and set
       to the same locking mode as the previous pointer value.

       If the idx'th object is not in the collection, then the safe pointer
       is set to NULL.
     */
    PSafePtr & operator=(PINDEX idx)
      {
        this->Assign(idx);
        return *this;
      }

    /**Set the safe pointer to the specified object.
       This will return a PSafePtr for the previous value of this. It does
       this in a thread safe manner so "test and set" semantics are obeyed.
      */
    PSafePtr Set(T * obj)
      {
        this->LockPtr();
        PSafePtr oldPtr = *this;
        this->Assign(obj);
        this->UnlockPtr();
        return oldPtr;
      }
  //@}

  /**@name Operations */
  //@{
    /**Return the physical pointer to the object.
      */
    operator T*()    const { return  dynamic_cast<T *>(this->m_currentObject); }

    /**Return the physical pointer to the object.
      */
    T & operator*()  const { return *dynamic_cast<T *>(PAssertNULL(this->m_currentObject)); }

    /**Allow access to the physical object the pointer is pointing to.
      */
    T * operator->() const { return  dynamic_cast<T *>(PAssertNULL(this->m_currentObject)); }

    /**Post-increment the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator++(int)
      {
        T * previous = dynamic_cast<T *>(this->m_currentObject);
        this->Next();
        return previous;
      }

    /**Pre-increment the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator++()
      {
        this->Next();
        return dynamic_cast<T *>(this->m_currentObject);
      }

    /**Post-decrement the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator--(int)
      {
        T * previous = dynamic_cast<T *>(this->m_currentObject);
        this->Previous();
        return previous;
      }

    /**Pre-decrement the pointer.
       This requires that the pointer has been created with a PSafeCollection
       object so that it can enumerate the collection.
      */
    T * operator--()
      {
        this->Previous();
        return dynamic_cast<T *>(this->m_currentObject);
      }
  //@}
};


/**Cast the pointer to a different type. The pointer being cast to MUST
    be a derived class or NULL is returned.
  */
template <class Base, class Derived>
PSafePtr<Derived> PSafePtrCast(const PSafePtr<Base> & oldPtr)
{
    PSafePtr<Derived> newPtr;
    if (dynamic_cast<Derived *>(oldPtr.GetObject()) != NULL)
      newPtr.Assign(oldPtr);
    return newPtr;
}


/** This class defines a thread-safe collection of objects.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Coll, class Base> class PSafeColl : public PSafeCollection
{
    PCLASSINFO_WITH_CLONE(PSafeColl, PSafeCollection);
  public:
  /**@name Construction */
  //@{
    /**Create a safe list collection wrapper around the real collection.
      */
    PSafeColl()
      : PSafeCollection(new Coll)
      { }

    /**Copy constructor for safe collection.
       Note the left hand side will always have DisallowDeleteObjects() set.
      */
    PSafeColl(const PSafeColl & other)
      : PSafeCollection(new Coll)
    {
      PWaitAndSignal lock2(other.m_collectionMutex);
      this->CopySafeCollection(dynamic_cast<Coll *>(other.m_collection));
    }

    /**Assign one safe collection to another.
       Note the left hand side will always have DisallowDeleteObjects() set.
      */
    PSafeColl & operator=(const PSafeColl & other)
    {
      if (&other != this) {
        RemoveAll(true);
        PWaitAndSignal lock1(this->m_collectionMutex);
        PWaitAndSignal lock2(other.m_collectionMutex);
        CopySafeCollection(dynamic_cast<Coll *>(other.m_collection));
      }
      return *this;
    }
  //@}

  /**@name Operations */
  //@{
    /**Add an object to the collection.
       This uses the PCollection::Append() function to add the object to the
       collection, with full mutual exclusion locking on the collection.
      */
    virtual PSafePtr<Base> Append(
      Base * obj,       ///< Object to add to safe collection.
      PSafetyMode mode = PSafeReference   ///< Safety mode for returned locked PSafePtr
    ) {
        PWaitAndSignal mutex(this->m_collectionMutex);
        if (SafeAddObject(obj, NULL))
          return PSafePtr<Base>(*this, mode, this->m_collection->Append(obj));
        return NULL;
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PBoolean Remove(
      Base * obj          ///< Object to remove from safe collection
    ) {
        return SafeRemove(obj);
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PBoolean RemoveAt(
      PINDEX idx     ///< Index to remove
    ) {
        return SafeRemoveAt(idx);
      }

    /**Get the instance in the collection of the index.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> GetAt(
      PINDEX idx,
      PSafetyMode mode = PSafeReadWrite
    ) {
        return PSafePtr<Base>(*this, mode, idx);
      }

    /**Find the instance in the collection of an object with the same value.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> FindWithLock(
      const Base & value,
      PSafetyMode mode = PSafeReadWrite
    ) {
        this->m_collectionMutex.Wait();
        PSafePtr<Base> ptr(*this, PSafeReference, this->m_collection->GetValuesIndex(value));
        this->m_collectionMutex.Signal();
        ptr.SetSafetyMode(mode);
        return ptr;
      }
  //@}
};


/** This class defines a thread-safe array of objects.
  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Base> class PSafeArray : public PSafeColl<PArray<Base>, Base>
{
  public:
    typedef PSafePtr<Base> value_type;
};


/** This class defines a thread-safe list of objects.
  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Base> class PSafeList : public PSafeColl<PList<Base>, Base>
{
  public:
    typedef PSafePtr<Base> value_type;
};


/** This class defines a thread-safe sorted array of objects.
  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Base> class PSafeSortedList : public PSafeColl<PSortedList<Base>, Base>
{
  public:
    typedef PSafePtr<Base> value_type;
};


/** This class defines a thread-safe dictionary of objects.

  This is part of a set of classes to solve the general problem of a
  collection (eg a PList or PDictionary) of objects that needs to be a made
  thread safe. Any thread can add, read, write or remove an object with both
  the object and the database of objects itself kept thread safe.

  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class Coll, class Key, class Base> class PSafeDictionaryBase : public PSafeCollection
{
    PCLASSINFO_WITH_CLONE(PSafeDictionaryBase, PSafeCollection);
  public:
  /**@name Construction */
  //@{
    /**Create a safe dictionary wrapper around the real collection.
      */
    PSafeDictionaryBase()
      : PSafeCollection(new Coll) { }

    /**Copy constructor for safe collection.
       Note the left hand side will always have DisallowDeleteObjects() set.
      */
    PSafeDictionaryBase(const PSafeDictionaryBase & other)
      : PSafeCollection(new Coll)
    {
      PWaitAndSignal lock2(other.m_collectionMutex);
      CopySafeDictionary(dynamic_cast<Coll *>(other.m_collection));
    }

    /**Assign one safe collection to another.
       Note the left hand side will always have DisallowDeleteObjects() set.
      */
    PSafeDictionaryBase & operator=(const PSafeDictionaryBase & other)
    {
      if (&other != this) {
        RemoveAll(true);
        PWaitAndSignal lock1(m_collectionMutex);
        PWaitAndSignal lock2(other.m_collectionMutex);
        CopySafeDictionary(dynamic_cast<Coll *>(other.m_collection));
      }
      return *this;
    }
  //@}

  /**@name Operations */
  //@{
    /**Add an object to the collection.
       This uses the PCollection::Append() function to add the object to the
       collection, with full mutual exclusion locking on the collection.
      */
    virtual void SetAt(const Key & key, Base * obj)
      {
        this->m_collectionMutex.Wait();
        if (SafeAddObject(obj, dynamic_cast<Coll &>(*this->m_collection).GetAt(key)))
          dynamic_cast<Coll &>(*this->m_collection).SetAt(key, obj);
        this->m_collectionMutex.Signal();
      }

    /**Remove an object to the collection.
       This function removes the object from the collection itself, but does
       not actually delete the object. It simply moves the object to a list
       of objects to be garbage collected at a later time.

       As for Append() full mutual exclusion locking on the collection itself
       is maintained.
      */
    virtual PBoolean RemoveAt(
      const Key & key   ///< Key to find object to delete
    ) {
        PWaitAndSignal mutex(this->m_collectionMutex);
        return SafeRemove(dynamic_cast<Coll &>(*this->m_collection).GetAt(key));
      }

    /**Determine of the dictionary contains an entry for the key.
      */
    virtual PBoolean Contains(
      const Key & key
    ) {
        PWaitAndSignal lock(this->m_collectionMutex);
        return dynamic_cast<Coll &>(*this->m_collection).Contains(key);
      }

    /**Get the instance in the collection of the index.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> GetAt(
      PINDEX idx,
      PSafetyMode mode = PSafeReadWrite
    ) {
        return PSafePtr<Base>(*this, mode, idx);
      }

    /**Find the instance in the collection of an object with the same value.
       The returned safe pointer will increment the reference count on the
       PSafeObject and lock to the object in the mode specified. The lock
       will remain until the PSafePtr goes out of scope.
      */
    virtual PSafePtr<Base> FindWithLock(
      const Key & key,
      PSafetyMode mode = PSafeReadWrite
    ) const {
        this->m_collectionMutex.Wait();
        PSafePtr<Base> ptr(*this, PSafeReference, dynamic_cast<Coll &>(*this->m_collection).GetAt(key));
        this->m_collectionMutex.Signal();
        ptr.SetSafetyMode(mode);
        return ptr;
      }

    /** Move an object from one key location to another.
      */
    virtual bool Move(
      const Key & from,   ///< Key to find object to move
      const Key & to      ///< Key to place found object
    ) {
      PWaitAndSignal mutex(this->m_collectionMutex);
      if (dynamic_cast<Coll &>(*this->m_collection).GetAt(to) != NULL)
        return false;
      dynamic_cast<Coll &>(*this->m_collection).SetAt(to, dynamic_cast<Coll &>(*this->m_collection).GetAt(from));
      return true;
    }

    /** Move all objects from other dictionary to this one.
        This will delete all the objects from the other dictionary, but does
        not delete the objects themselves.
      */
    void MoveFrom(PSafeDictionaryBase & other)
    {
      if (this == &other)
        return;

      *this = other;
      this->AllowDeleteObjects(); // We now own the objects, need to put this back on

      // Remove from other without deleting them
      bool del = other.m_deleteObjects;
      other.DisallowDeleteObjects();
      other.RemoveAll();
      other.AllowDeleteObjects(del);
    }

    /**Get an array containing all the keys for the dictionary.
      */
    PArray<Key> GetKeys() const
    {
      PArray<Key> keys;
      this->m_collectionMutex.Wait();
      dynamic_cast<Coll &>(*this->m_collection).AbstractGetKeys(keys);
      this->m_collectionMutex.Signal();
      return keys;
    }
  //@}
};


/** This class defines a thread-safe array of objects.
  See the PSafeObject class for more details. Especially in regard to
  enumeration of collections.
 */
template <class K, class D>
 class PSafeDictionary : public PSafeDictionaryBase<PDictionary<K, D>, K, D>
{
  public:
    typedef K key_type;
    typedef D data_type;
    typedef PSafePtr<D> value_type;
    typedef PSafeDictionary<K, D> dict_type;

  /**@name Iterators */
  //@{
    class iterator;
    class const_iterator;
    class iterator_base {
      protected:
        K * m_internal_first;  // Must be first two members
        value_type m_internal_second;

        const dict_type * m_dictionary;
        PArray<K>   m_keys;
        PINDEX      m_position;

        iterator_base()
          : m_internal_first(NULL)
          , m_internal_second(NULL)
          , m_dictionary(NULL)
          , m_position(P_MAX_INDEX)
        {
        }

        iterator_base(const dict_type * dict)
          : m_dictionary(dict)
          , m_keys(dict->GetKeys())
        {
          this->SetPosition(0);
        }

        iterator_base(const dict_type * dict, const K & key)
          : m_dictionary(dict)
          , m_keys(dict->GetKeys())
        {
          this->SetPosition(m_keys.GetValuesIndex(key));
        }

        bool SetPosition(PINDEX position)
        {
          if (position >= this->m_keys.GetSize()) {
            this->m_position = P_MAX_INDEX;
            this->m_internal_first = NULL;
            this->m_internal_second.SetNULL();
            return false;
          }

          this->m_position = position;
          this->m_internal_first  = &this->m_keys[position];
          this->m_internal_second = this->m_dictionary->FindWithLock(*this->m_internal_first, PSafeReference);
          return this->m_internal_second == NULL;
        }

        void Next() { while (this->SetPosition(this->m_position+1)) { } }
        void Prev() { while (this->SetPosition(this->m_position > 0 ? this->m_position+1 : P_MAX_INDEX)) { } }

      public:
        bool operator==(const iterator_base & it) const { return this->m_position == it.m_position; }
        bool operator!=(const iterator_base & it) const { return this->m_position != it.m_position; }
    };

    class iterator_pair {
      public:
        const K & first;
        value_type second;

      private:
        iterator_pair() : first(reinterpret_cast<const K &>(0)) { }
    };

    class iterator : public iterator_base, public std::iterator<std::forward_iterator_tag, iterator_pair> {
      protected:
        iterator(dict_type * dict) : iterator_base(dict) { }
        iterator(dict_type * dict, const K & key) : iterator_base(dict, key) { }

      public:
        iterator() { }

        iterator operator++()    {                      this->Next(); return *this; }
        iterator operator--()    {                      this->Prev(); return *this; }
        iterator operator++(int) { iterator it = *this; this->Next(); return it;    }
        iterator operator--(int) { iterator it = *this; this->Prev(); return it;    }

        const iterator_pair * operator->() const { return  reinterpret_cast<const iterator_pair *>(this); }
        const iterator_pair & operator* () const { return *reinterpret_cast<const iterator_pair *>(this); }

      friend class PSafeDictionary<K, D>;
    };

    iterator begin() { return iterator(this); }
    iterator end()   { return iterator(); }
    iterator find(const K & key) { return iterator(this, key); }


    class const_iterator : public iterator_base, public std::iterator<std::forward_iterator_tag, iterator_pair> {
      protected:
        const_iterator(const dict_type * dict) : iterator_base(dict) { }
        const_iterator(const dict_type * dict, const K & key) : iterator_base(dict, key) { }

      public:
        const_iterator() { }
        const_iterator(const typename dict_type::iterator & it) : iterator_base(it) { }

        const_iterator operator++()    {                            this->Next(); return *this; }
        const_iterator operator--()    {                            this->Prev(); return *this; }
        const_iterator operator++(int) { const_iterator it = *this; this->Next(); return it;    }
        const_iterator operator--(int) { const_iterator it = *this; this->Prev(); return it;    }

        const iterator_pair * operator->() const { return  reinterpret_cast<const iterator_pair *>(this); }
        const iterator_pair & operator* () const { return *reinterpret_cast<const iterator_pair *>(this); }

      friend class PSafeDictionary<K, D>;
    };

    const_iterator begin() const { return const_iterator(this); }
    const_iterator end()   const { return const_iterator(); }
    const_iterator find(const K & key) const { return const_iterator(this, key); }

    void erase(const       iterator & it) { this->RemoveAt(it->first); }
    void erase(const const_iterator & it) { this->RemoveAt(it->first); }
  //@}
};


#endif // PTLIB_SAFE_COLLECTION_H


// End Of File ///////////////////////////////////////////////////////////////

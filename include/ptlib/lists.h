/*
 * $Id: lists.h,v 1.4 1995/02/05 00:48:05 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: lists.h,v $
 * Revision 1.4  1995/02/05 00:48:05  robertj
 * Fixed template version.
 *
 * Revision 1.3  1995/01/15  04:49:23  robertj
 * Fixed errors in template version.
 *
 * Revision 1.2  1994/12/21  11:53:12  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.1  1994/12/12  09:59:35  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PList container class

PDECLARE_CONTAINER(PAbstractList, PCollection)
/* This class is a collection of objects which are descendents of the
   $H$PObject class. It is implemeted as a doubly linked list.

   The implementation of a list allows very fast inserting and deleting of
   objects in the collection, but has severe penalties for random access. All
   object access should be done sequentially to avoid these speed penalties.

   The class remembers the last accessed element. This state information is
   used to optimise access by the "virtual array" model of collections. If
   access via ordinal index is made sequentially there is little overhead.

   The PAbstractList class would very rarely be descended from directly by
   the user. The $H$PDECLARE_LIST and $H$PLIST macros would normally be used to
   create descendent classes. They will instantiate the template based on
   $H$PList or directly declare and define the class (using inline functions)
   if templates are not being used.

   The $H$PList class or $H$PDECLARE_LIST macro will define the correctly typed
   operators for subscript access (operator[]).
 */

  public:
    PINLINE PAbstractList();
    /* Create a new, empty, list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

  // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;
    /* Get the relative rank of the two lists. The following algorithm is
       employed for the comparison:

        $I$EqualTo$I$     if the two lists are identical in length and each
                          objects values, not pointer, are equal.

        $I$LessThan$I$    if the instances object value at an ordinal position
                          is less than the corresponding objects value in the
                          $B$obj$B$ parameters list.
                          
                          This is also returned if all objects are equal and
                          the instances list length is less than the $B$obj$B$
                          parameters list length.

        $I$GreaterThan$I$ if the instances object value at an ordinal position
                          is greater than the corresponding objects value in
                          the $B$obj$B$ parameters list.
                          
                          This is also returned if all objects are equal and
                          the instances list length is greater than the
                          $B$obj$B$ parameters list length.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
     */

  // Overrides from class PContainer
    virtual BOOL SetSize(
      PINDEX newSize  // New size for the list, this is ignored.
    );
    /* This function is meaningless for lists. The size of the collection is
       determined by the addition and removal of objects. The size cannot be
       set in any other way.

       Returns: TRUE.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* Append a new object to the collection. This places a new link at the
       "tail" of the list.
    
       Returns: index of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );
    /* Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the $B$Append()$B$ function is performed.
       
       Note that the object values are compared for the search of the
       $B$before$B$ parameter, not the pointers. So the objects in the
       collection must correctly implement the $B$Compare()$B$ function.

       Returns: index of the newly inserted object.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );
    /* Insert a new object at the specified ordinal index. If the index is
       greater than the number of objects in the collection then the
       equivalent of the $B$Append()$B$ function is performed.

       Returns: index of the newly inserted object.
     */

    virtual BOOL Remove(
      const PObject * obj   // Existing object to remove from the collection.
    );
    /* Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       Returns: TRUE if the object was in the collection.
     */

    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    );
    /* Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       Note if the index is beyond the size of the collection then the
       function will assert.

       Returns: pointer to the object being removed, or NULL if it was deleted.
     */

    virtual BOOL SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );
    /* Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.

       The object accessed in this way is remembered by the class and further
       access will be fast. Access to elements one either side of that saved
       element, and the head and tail of the list, will always be fast.

       Note if the index is beyond the size of the collection then the
       function will assert.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;
    /* Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       The object accessed in this way is remembered by the class and further
       access will be fast. Access to elements one either side of that saved
       element, and the head and tail of the list, will always be fast.

       Returns: pointer to object at the specified index.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A simple linear search
       from "head" of the list is performed.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const;
    /* Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Compare()$B$ function. A
       simple linear search from "head" of the list is performed.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */


  protected:
    BOOL SetCurrent(
      PINDEX index  // Ordinal index of the list element to set as current.
    ) const;
    /* Move the internal "cursor" to the index position specified. This
       function will optimise the sequential move taking into account the
       previous current position and the position at the head and tail of the
       list. Whichever of these three points is closes is used as the starting
       point for a sequential move to the required index.

       Returns: TRUE if the index could be set as the current element.
     */

    class Element {
      public:
        Element(PObject * theData);
        Element * prev;
        Element * next;
        PObject * data;
    };

    class Info {
      public:
        Info() { head = tail = lastElement = NULL; }
        Element * head;
        Element * tail;
        Element * lastElement;
        PINDEX    lastIndex;
    } * info;
};


#ifdef PHAS_TEMPLATES

template <class T>
PDECLARE_CLASS(PList, PAbstractList)
/* This template class maps the PAbstractList to a specific object type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the $H$PDECLARE_LIST macro will simulate
   the template instantiation.
 */

  public:
    PList()
      : PAbstractList() { }
    /* Create a new, empty, list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW PList(0, this); }
    /* Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */

    T & operator[](PINDEX index) const
      { return *(T *)GetAt(index); }
    /* Retrieve a reference  to the object in the list. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       The object accessed in this way is remembered by the class and further
       access will be fast. Access to elements one either side of that saved
       element, and the head and tail of the list, will always be fast.

       Returns: reference to the object at $B$index$B$ position.
     */

  protected:
    PList(int dummy, const PList * c)
      : PAbstractList(dummy, c) { }
};


/*$MACRO PDECLARE_LIST(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PList template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PList and $H$PAbstractList classes for more information.
 */
#define PDECLARE_LIST(cls, T) \
  PDECLARE_CLASS(cls, PList<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PList<T>(dummy, c) { } \
  public: \
    cls() \
      : PList<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \

/*$MACRO PLIST(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PList template class.

   See the $H$PList class and $H$PDECLARE_LIST macro for more information.
 */
#define PLIST(cls, T) typedef PList<T> cls


template <class T>
PDECLARE_CLASS(PQueue, PAbstractList)
/* This template class maps the PAbstractList to a specific object type, and
   adds functionality that allows the list to be used as a first in first out
   queue. The functions in this class primarily do all the appropriate casting of types.

   By default, objects placed into the set will $U$not$U$ be deleted when
   removed or when all references to the set are destroyed. This is different
   from the default on most collection classes.

   Note that if templates are not used the $H$PDECLARE_QUEUE macro will
   simulate the template instantiation.
 */

  public:
    PQueue()
      : PAbstractList() { DisallowDeleteObjects(); }
    /* Create a new, empty, queue.

       Note that by default, objects placed into the queue will $U$not$U$ be
       deleted when removed or when all references to the queue are destroyed.
       This is different from the default on most collection classes.
     */

    virtual PObject * Clone() const
      { return PNEW PQueue(0, this); }
    /* Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */

    virtual void Enqueue(
      T * obj   // Object to add to the queue.
    ) { PAbstractList::Append(t); }
    /* Add a new object to the queue. This places a new link at the "tail" of
       the list, which is the "in" side of the queue.
     */

    virtual T * Dequeue()
      { return (T *)PAbstractList::RemoveAt(0);}
    /* Remove an object that was added to the queue.

       Returns: first object added to the queue or NULL if queue empty.
     */

  protected:
    PQueue(int dummy, const PQueue * c)
      : PAbstractList(dummy, c)
      { reference->deleteObjects = c->reference->deleteObjects; }
};


/*$MACRO PDECLARE_QUEUE(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$, and adds functionality
   that allows the list to be used as a first in first out queue.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PQueue template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PQueue and $H$PAbstractList classes for more information.
 */
#define PDECLARE_QUEUE(cls, T) \
  PDECLARE_CLASS(cls, PQueue<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PQueue<T>(dummy, c) { } \
  public: \
    cls() \
      : PQueue<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PQUEUE(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$, and adds functionality
   that allows the list to be used as a first in first out queue. This macro
   closes the class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PQueue template class.

   See the $H$PList class and $H$PDECLARE_QUEUE macro for more information.
 */
#define PQUEUE(cls, T) typedef PQueue<T> cls


template <class T>
PDECLARE_CLASS(PStack, PAbstractList)
/* This template class maps the PAbstractList to a specific object type, and
   adds functionality that allows the list to be used as a last in first out
   stack. The functions in this class primarily do all the appropriate casting
   of types.

   By default, objects placed into the set will $U$not$U$ be deleted when
   removed or when all references to the set are destroyed. This is different
   from the default on most collection classes.

   Note that if templates are not used the $H$PDECLARE_STACK macro will
   simulate the template instantiation.
 */

  public:
    PStack()
      : PAbstractList() { DisallowDeleteObjects(); }
    /* Create a new, empty, stack.

       Note that by default, objects placed into the stack will $U$not$U$ be
       deleted when removed or when all references to the stack are destroyed.
       This is different from the default on most collection classes.
     */

    virtual PObject * Clone() const
      { return PNEW PStack(0, this); }
    /* Make a complete duplicate of the stack. Note that all objects in the
       array are also cloned, so this will make a complete copy of the stack.
     */

    virtual void Push(
      T * obj    // Object to add to the stack.
    ) { PAbstractList::InsertAt(0, obj); }
    /* Add an object to the stack. This object will be on "top" of the stack
       and will be the object returned by the $B$Pop()$B$ function.
     */

    virtual T * Pop()
      { return (T *)PAbstractList::RemoveAt(0); }
    /* Remove the last object pushed onto the stack.

       Returns: object on top of the stack.
     */

    virtual T & Top()
      { PAssert(GetSize() > 0, PStackEmpty); return *(T *)GetAt(0); }
    /* Get the element that is currently on top of the stack without removing
       it.

       Returns: reference to object on top of the stack.
     */


  protected:
    PStack(int dummy, const PStack * c)
      : PAbstractList(dummy, c)
      { reference->deleteObjects = c->reference->deleteObjects; }
};

/*$MACRO PDECLARE_STACK(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$, and adds functionality
   that allows the list to be used as a last in first out stack.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PStack template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PStack and $H$PAbstractList classes for more information.
 */
#define PDECLARE_STACK(cls, T) \
  PDECLARE_CLASS(cls, PStack<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PStack<T>(dummy, c) { } \
  public: \
    cls() \
      : PStack<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PSTACK(cls, T)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type $B$T$B$, and adds functionality
   that allows the list to be used as a last in first out stack. This macro
   closes the class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PStack template class.

   See the $H$PStack class and $H$PDECLARE_STACK macro for more information.
 */
#define PSTACK(cls, T) typedef PStack<T> cls


#else // PHAS_TEMPLATES


#define PDECLARE_LIST(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractList() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const \
      { return *(T *)GetAt(index); } \

#define PLIST(cls, T) PDECLARE_LIST(cls, T) }

#define PDECLARE_QUEUE(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) \
      { reference->deleteObjects = c->reference->deleteObjects; } \
  public: \
    inline cls() \
      : PAbstractList() { DisallowDeleteObjects(); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    virtual inline void Enqueue(T * t) \
      { PAbstractList::Append(t); } \
    virtual inline T * Dequeue() \
      { return (T *)PAbstractList::RemoveAt(0);} \

#define PQUEUE(cls, T) PDECLARE_QUEUE(cls, T) }


#define PDECLARE_STACK(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) \
      { reference->deleteObjects = c->reference->deleteObjects; } \
  public: \
    inline cls() \
      : PAbstractList() { DisallowDeleteObjects(); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    virtual inline void Push(T * t) \
      { PAbstractList::InsertAt(0, t); } \
    virtual inline T * Pop() \
      { return (T *)PAbstractList::RemoveAt(0); } \
    virtual inline T & Top() \
      { PAssert(GetSize() > 0, PStackEmpty); return *(T *)GetAt(0); } \

#define PSTACK(cls, T) PDECLARE_STACK(cls, T) }


#endif // PHAS_TEMPLATES


///////////////////////////////////////////////////////////////////////////////
// Sorted List of PObjects

PDECLARE_CONTAINER(PAbstractSortedList, PCollection)
/* This class is a collection of objects which are descendents of the
   $H$PObject class. It is implemeted as a Red-Black binary tree to maintain
   the objects in rank order. Note that this requires that the $B$Compare()$B$
   function be fully implemented oin objects contained in the collection.

   The implementation of a sorted list allows fast inserting and deleting as
   well as random access of objects in the collection. As the objects are being
   kept sorted, "fast" is a relative term. All operations take o(lg n) unless
   a particular object is repeatedly accessed.

   The class remembers the last accessed element. This state information is
   used to optimise access by the "virtual array" model of collections. If
   repeated access via ordinal index is made there is little overhead. All
   other access incurs a minimum overhead, but not insignificant.

   The PAbstractSortedList class would very rarely be descended from directly
   by the user. The $H$PDECLARE_LIST and $H$PLIST macros would normally be used
   to create descendent classes. They will instantiate the template based on
   $H$PSortedList or directly declare and define the class (using inline
   functions) if templates are not being used.

   The $H$PSortedList class or $H$PDECLARE_SORTED_LIST macro will define the
   correctly typed operators for subscript access (operator[]).
 */

  public:
    PINLINE PAbstractSortedList();
    /* Create a new, empty, sorted list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

  // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;
    /* Get the relative rank of the two lists. The following algorithm is
       employed for the comparison:

        $I$EqualTo$I$     if the two lists are identical in length and each
                          objects values, not pointer, are equal.

        $I$LessThan$I$    if the instances object value at an ordinal position
                          is less than the corresponding objects value in the
                          $B$obj$B$ parameters list.
                          
                          This is also returned if all objects are equal and
                          the instances list length is less than the $B$obj$B$
                          parameters list length.

        $I$GreaterThan$I$ if the instances object value at an ordinal position
                          is greater than the corresponding objects value in
                          the $B$obj$B$ parameters list.
                          
                          This is also returned if all objects are equal and
                          the instances list length is greater than the
                          $B$obj$B$ parameters list length.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
     */

  // Overrides from class PContainer
    virtual BOOL SetSize(
      PINDEX newSize  // New size for the sorted list, this is ignored.
    );
    /* This function is meaningless for lists. The size of the collection is
       determined by the addition and removal of objects. The size cannot be
       set in any other way.

       Returns: TRUE.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* Add a new object to the collection. The object is always placed in the
       correct ordinal position in the list. It is not placed at the "end".

       Returns: index of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );
    /* Add a new object to the collection.
    
       The object is always placed in the correct ordinal position in the list.
       It is not placed at the specified position. The $B$before$B$ parameter
       is ignored.

       Returns: index of the newly inserted object.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );
    /* Add a new object to the collection.
    
       The object is always placed in the correct ordinal position in the list.
       It is not placed at the specified position. The $B$index$B$ parameter
       is ignored.

       Returns: index of the newly inserted object.
     */

    virtual BOOL Remove(
      const PObject * obj   // Existing object to remove from the collection.
    );
    /* Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       Note that the comparison for searching for the object in collection is
       made by pointer, not by value. Thus the parameter must point to the
       same instance of the object that is in the collection.

       Returns: TRUE if the object was in the collection.
     */

    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    );
    /* Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       Note if the index is beyond the size of the collection then the
       function will assert.

       Returns: pointer to the object being removed, or NULL if it was deleted.
     */

    virtual void RemoveAll();
    /* Remove all of the elements in the collection. This operates by
       continually calling $B$RemoveAt()$B$ until there are no objects left.

       The objects are removed from the last, at index $F$(GetSize()-1)$F$
       toward the first at index zero.
     */

    virtual BOOL SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );
    /* Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.

       Note, the object placed at $B$index$B$ will not stay at that ordinal
       position. It is actually placed at the correct position for its rank.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;
    /* Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       Returns: pointer to object at the specified index.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A binary search is
       employed to locate the entry.
       
       Note that that will require value comparisons to be made to find the
       equivalent entry and then a final check is made with the pointers to
       see if they are the same instance.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const;
    /* Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Compare()$B$ function. A
       binary search is employed to locate the entry.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */


  protected:
    class Element {
      public:
        Element(PObject * theData);
        Element * parent;
        Element * left;
        Element * right;
        PObject * data;
        PINDEX subTreeSize;
        enum { Red, Black } colour;
        void MakeBlack();
        void MakeRed();
        BOOL IsBlack();
        BOOL IsLeftBlack();
        BOOL IsRightBlack();
        void DeleteSubTrees(BOOL deleteObject);
        BOOL LeftTreeSize();
        BOOL RightTreeSize();
        Element * Successor() const;
        Element * Predecessor() const;
        Element * OrderSelect(PINDEX index);
        PINDEX ValueSelect(const PObject & obj);
    };
    friend class Element;

    class Info {
      public:
        Info() { root = lastElement = NULL; }
        Element * root;
        Element * lastElement;
        PINDEX    lastIndex;
    } * info;

    // New functions for class
    void RemoveElement(Element * node);
    void LeftRotate(Element * node);
    void RightRotate(Element * node);
};


#ifdef PHAS_TEMPLATES

template <class T>
PDECLARE_CLASS(PSortedList, PAbstractSortedList)
/* This template class maps the PAbstractSortedList to a specific object type.
   The functions in this class primarily do all the appropriate casting of
   types.

   Note that if templates are not used the $H$PDECLARE_SORTED_LIST macro will
   simulate the template instantiation.
 */

  public:
    PSortedList()
      : PAbstractSortedList() { }
    /* Create a new, empty, sorted list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW PSortedList(0, this); }
    /* Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */

    T & operator[](PINDEX index) const
      { return *(T *)GetAt(index); }
    /* Retrieve a reference  to the object in the list. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       The object accessed in this way is remembered by the class and further
       access will be fast.

       Returns: reference to the object at $B$index$B$ position.
     */

  protected:
    PSortedList(int dummy, const PSortedList * c)
      : PAbstractSortedList(dummy, c) { }
};


/*$MACRO PDECLARE_LIST(cls, T)
   This macro is used to declare a descendent of PAbstractSortedList class,
   customised for a particular object type $B$T$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PSortedList template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PSortedList and $H$PAbstractSortedList classes for more
   information.
 */
#define PDECLARE_SORTED_LIST(cls, T) \
  PDECLARE_CLASS(cls, PSortedList<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PSortedList<T>(dummy, c) { } \
  public: \
    cls() \
      : PSortedList<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \

/*$MACRO PSORTEDLIST(cls, T)
   This macro is used to declare a descendent of PAbstractSortedList class,
   customised for a particular object type $B$T$B$. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PSortedList template class.

   See the $H$PSortedList class and $H$PDECLARE_SORTED_LIST macro for more
   information.
 */
#define PSORTED_LIST(cls, T) typedef PSortedList<T> cls


#else // PHAS_TEMPLATES


#define PDECLARE_SORTED_LIST(cls, T) \
  PDECLARE_CLASS(cls, PAbstractSortedList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractSortedList(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractSortedList() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const \
      { return *(T *)GetAt(index); } \

#define PSORTED_LIST(cls, T) PDECLARE_SORTED_LIST(cls, T) }


#endif  // PHAS_TEMPLATES


// End Of File ///////////////////////////////////////////////////////////////

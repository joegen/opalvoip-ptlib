/*
 * lists.h
 *
 * List Container Classes
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

#ifndef PTLIB_LISTS_H
#define PTLIB_LISTS_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PList container class

struct PListElement
{
    PListElement(PObject * theData);
    PListElement * prev;
    PListElement * next;
    PObject * data;

    PDECLARE_POOL_ALLOCATOR(PListElement);
};

struct PListInfo
{
    PListInfo() { head = tail = NULL; }
    PListElement * head;
    PListElement * tail;

    PDECLARE_POOL_ALLOCATOR(PListInfo);
};

/**This class is a collection of objects which are descendents of the
   <code>PObject</code> class. It is implemeted as a doubly linked list.

   The implementation of a list allows very fast inserting and deleting of
   objects in the collection, but has severe penalties for random access. All
   object access should be done sequentially to avoid these speed penalties.

   The class remembers the last accessed element. This state information is
   used to optimise access by the "virtual array" model of collections. If
   access via ordinal index is made sequentially there is little overhead.

   The PAbstractList class would very rarely be descended from directly by
   the user. The <code>PDECLARE_LIST</code> and <code>PLIST</code> macros would normally
   be used to create descendent classes. They will instantiate the template
   based on <code>PList</code> or directly declare and define the class (using
   inline functions) if templates are not being used.

   The <code>PList</code> class or <code>PDECLARE_LIST</code> macro will define the
   correctly typed operators for subscript access (operator[]).
 */
class PAbstractList : public PCollection
{
  PCONTAINERINFO(PAbstractList, PCollection);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PINLINE PAbstractList();
  //@}

  // Overrides from class PObject
    /**Get the relative rank of the two lists. The following algorithm is
       employed for the comparison:

       \arg \c EqualTo if the two lists are identical in length
       and each objects values, not pointer, are equal.

       \arg \c LessThan if the instances object value at an
       ordinal position is less than the corresponding objects value in the
       <code>obj</code> parameters list.
                          
       This is also returned if all objects are equal and the instances list
       length is less than the <code>obj</code> parameters list length.

       \arg \c GreaterThan if the instances object value at an
       ordinal position is greater than the corresponding objects value in the
       <code>obj</code> parameters list.
                          
       This is also returned if all objects are equal and the instances list
       length is greater than the <code>obj</code> parameters list length.

       @return
       comparison of the two objects, <code>EqualTo</code> for same,
       <code>LessThan</code> for <code>obj</code> logically less than the
       object and <code>GreaterThan</code> for <code>obj</code> logically
       greater than the object.
     */
    virtual Comparison Compare(
      const PObject & obj   ///< Object being compared to
    ) const;

  /**@name Overrides from class PContainer */
  //@{
    /**This function is meaningless for lists. The size of the collection is
       determined by the addition and removal of objects. The size cannot be
       set in any other way.

       @return
       Always true.
     */
    virtual PBoolean SetSize(
      PINDEX newSize  ///< New size for the list, this is ignored.
    );
  //@}

  /**@name Overrides from class PCollection */
  //@{
    /**Append a new object to the collection. This places a new link at the
       "tail" of the list.
    
       @return
       index of the newly added object.
     */
    virtual PINDEX Append(
      PObject * obj   ///< New object to place into the collection.
    );

    /**Append a new object to the collection. This places a new link at the
       "tail" of the list.
    
       @return
       index of the newly added object.
     */
    virtual void Prepend(
      PObject * obj   ///< New object to place into the collection.
    );

    /**Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the <code>Append()</code> function is performed.
       
       Note that the object values are compared for the search of the
       <code>before</code> parameter, not the pointers. So the objects in the
       collection must correctly implement the <code>PObject::Compare()</code>
       function.

       @return
       index of the newly inserted object.
     */
    virtual PINDEX Insert(
      const PObject & before,   ///< Object value to insert before.
      PObject * obj             ///< New object to place into the collection.
    );

    /**Insert a new object at the specified ordinal index. If the index is
       greater than the number of objects in the collection then the
       equivalent of the <code>Append()</code> function is performed.

       @return
       index of the newly inserted object.
     */
    P_DEPRECATED virtual PINDEX InsertAt(
      PINDEX index,   ///< Index position in collection to place the object.
      PObject * obj   ///< New object to place into the collection.
    );

    /**Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       @return
       true if the object was in the collection.
     */
    virtual PBoolean Remove(
      const PObject * obj   ///< Existing object to remove from the collection.
    );

    /**Remove the head object from the list. If the AllowDeleteObjects option
       is set then the object is also deleted.

       @return
       pointer to the object being removed, or NULL if it was deleted.
     */
    virtual PObject * RemoveHead()
      { return RemoveElement(m_info->head); }

    /**Remove the tail object from the list. If the AllowDeleteObjects option
       is set then the object is also deleted.

       @return
       pointer to the object being removed, or NULL if it was deleted.
     */
    virtual PObject * RemoveTail()
      { return RemoveElement(m_info->tail); }


    /**Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       Note if the index is beyond the size of the collection then the
       function will assert.

       @return
       pointer to the object being removed, or NULL if it was deleted.
     */
    P_DEPRECATED virtual PObject * RemoveAt(
      PINDEX index   ///< Index position in collection to place the object.
    );

    /**Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. 
       This method will NOT delete the old object independently of the 
       AllowDeleteObjects option. Use <code>ReplaceAt()</code> instead.

       Note if the index is beyond the size of the collection then the
       function will assert.

       @return
       true if the object was successfully added.
     */
    P_DEPRECATED virtual PBoolean SetAt(
      PINDEX index,   ///< Index position in collection to set.
      PObject * val   ///< New value to place into the collection.
    );
    
    /**Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.
    
       Note if the index is beyond the size of the collection then the
       function will assert.
       
       @return
       true if the object was successfully replaced.
     */   
    P_DEPRECATED virtual PBoolean ReplaceAt(
      PINDEX index,   ///< Index position in collection to set.
      PObject * val   ///< New value to place into the collection.
    );

    /**Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       The object accessed in this way is remembered by the class and further
       access will be fast. Access to elements one either side of that saved
       element, and the head and tail of the list, will always be fast.

       @return
       pointer to object at the specified index.
     */
    P_DEPRECATED virtual PObject * GetAt(
      PINDEX index  ///< Index position in the collection of the object.
    ) const;

    /**Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A simple linear search
       from "head" of the list is performed.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetObjectsIndex(
      const PObject * obj  ///< Object to find.
    ) const;

    /**Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the <code>PObject::Compare()</code>
       function. A simple linear search from "head" of the list is performed.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetValuesIndex(
      const PObject & obj  ///< Object to find value of.
    ) const;
  //@}


  protected:
    PINLINE PObject & GetReferenceAt(PINDEX index) const;
    PListElement * FindElement(PINDEX index) const;
    PListElement * FindElement(const PObject & obj, PINDEX * index) const;
    void InsertElement(PListElement * element, PObject * obj);
    PObject * RemoveElement(PListElement * element);

    // The types below cannot be nested as DevStudio 2005 AUTOEXP.DAT doesn't like it
    typedef PListElement Element;
    PListInfo * m_info;
};


/**This template class maps the PAbstractList to a specific object type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the <code>PDECLARE_LIST</code> macro will
   simulate the template instantiation.
 */
template <class T> class PList : public PAbstractList
{
  PCLASSINFO(PList, PAbstractList);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PList()
      : PAbstractList() { }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */
    virtual PObject * Clone() const
      { return PNEW PList(0, this); }
  //@}

  /**@name Iterators */
  //@{
    typedef T value_type;

    class iterator_base : public std::iterator<std::bidirectional_iterator_tag, value_type> {
      protected:
        iterator_base(PListElement * e) : element(e) { }
        PListElement * element;

        void Next() { this->element = PAssertNULL(this->element)->next; }
        void Prev() { this->element = PAssertNULL(this->element)->prev; }
        value_type * Ptr() const { return  dynamic_cast<value_type *>(PAssertNULL(this->element)->data); }

      public:
        bool operator==(const iterator_base & it) const { return this->element == it.element; }
        bool operator!=(const iterator_base & it) const { return this->element != it.element; }

      friend class PList<T>;
    };

    class iterator : public iterator_base {
      public:
        iterator(PListElement * e = NULL) : iterator_base(e) { }

        iterator operator++()    {                      this->Next(); return *this; }
        iterator operator--()    {                      this->Prev(); return *this; }
        iterator operator++(int) { iterator it = *this; this->Next(); return it;    }
        iterator operator--(int) { iterator it = *this; this->Prev(); return it;    }

        value_type * operator->() const { return  this->Ptr(); }
        value_type & operator* () const { return *this->Ptr(); }
    };

    iterator begin()  { return this->m_info->head; }
    iterator end()    { return iterator(); }
    iterator rbegin() { return this->m_info->tail; }
    iterator rend()   { return iterator(); }
    iterator find(const value_type & obj) { return this->FindElement(obj, NULL); }
    void insert(const iterator & pos, value_type * obj) { this->InsertElement(pos.element, obj); }
    void insert(const iterator & pos, const value_type & obj) { this->InsertElement(pos.element, obj.Clone()); }

    class const_iterator : public iterator_base {
      public:
        const_iterator(PListElement * e = NULL) : iterator_base(e) { }

        const_iterator operator++()    {                            this->Next(); return *this; }
        const_iterator operator--()    {                            this->Prev(); return *this; }
        const_iterator operator++(int) { const_iterator it = *this; this->Next(); return it;    }
        const_iterator operator--(int) { const_iterator it = *this; this->Prev(); return it;    }

        const value_type * operator->() const { return  this->Ptr(); }
        const value_type & operator* () const { return *this->Ptr(); }
    };

    const_iterator begin()  const { return this->m_info->head; }
    const_iterator end()    const { return const_iterator(); }
    const_iterator rbegin() const { return this->m_info->tail; }
    const_iterator rend()   const { return const_iterator(); }
    const_iterator find(const value_type & obj) const { return this->FindElement(obj, NULL); }

    value_type & front() const { return dynamic_cast<value_type &>(*PAssertNULL(this->m_info->head)->data); }
    value_type & back() const { return dynamic_cast<value_type &>(*PAssertNULL(this->m_info->tail)->data); }
    __inline void erase(const iterator & it) { this->RemoveElement(it.element); }
    __inline void erase(const const_iterator & it) { this->RemoveElement(it.element); }
    __inline void push_front(const value_type & value) { this->InsertAt(0, new value_type(value)); }
    __inline void push_back(const value_type & value) { this->Append(new value_type(value)); }
    __inline void pop_front() { this->RemoveHead(); }
    __inline void pop_back() { this->RemoveTail(); }
  //@}

  /**@name New functions for class */
  //@{
    /**Retrieve a reference  to the object in the list. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       The object accessed in this way is remembered by the class and further
       access will be fast. Access to elements one either side of that saved
       element, and the head and tail of the list, will always be fast.

       @return
       reference to the object at <code>index</code> position.
     */
    T & operator[](
      PINDEX index  ///< Index for entry
    ) const { return dynamic_cast<T &>(this->GetReferenceAt(index)); }
  //@}

  protected:
    PList(int dummy, const PList * c)
      : PAbstractList(dummy, c) { }
};


/**Declare a list class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <code>PList</code> template class.

   See the <code>PList</code> class and <code>PDECLARE_LIST</code> macro for more
   information.
 */
#define PLIST(cls, T) typedef PList<T> cls

/**Begin declaration of list class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>.

   If the compilation is using templates then this macro produces a descendent
   of the <code>PList</code> template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the <code>PList</code> and <code>PAbstractList</code> classes for more information.
 */
#define PDECLARE_LIST(cls, T) \
  PLIST(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, PList<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PList<T>(dummy, c) { } \
  public: \
    cls() \
      : PList<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/**This template class maps the PAbstractList to a specific object type, and
   adds functionality that allows the list to be used as a first in first out
   queue. The functions in this class primarily do all the appropriate casting
   of types.

   By default, objects placed into the set will <b>T</b> be deleted when
   removed or when all references to the set are destroyed. This is different
   from the default on most collection classes.

   Note that if templates are not used the <code>PDECLARE_QUEUE</code> macro will
   simulate the template instantiation.
 */
template <class T> class PQueue : public PAbstractList
{
  PCLASSINFO(PQueue, PAbstractList);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, queue.

       Note that by default, objects placed into the queue will <b>not</b> be
       deleted when removed or when all references to the queue are destroyed.
       This is different from the default on most collection classes.
     */
    PQueue()
      : PAbstractList() { DisallowDeleteObjects(); }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */
    virtual PObject * Clone() const
      { return PNEW PQueue(0, this); }
  //@}

  /**@name New functions for class */
  //@{
    /**Add a new object to the queue. This places a new link at the "tail" of
       the list, which is the "in" side of the queue.
     */
    virtual void Enqueue(
      T * obj   ///< Object to add to the queue.
    ) { PAbstractList::Append(obj); }
    /**Remove an object that was added to the queue.

       @return
       first object added to the queue or NULL if queue empty.
     */
    virtual T * Dequeue()
      { return dynamic_cast<T *>(PAbstractList::RemoveHead()); }
  //@}

  protected:
    PQueue(int dummy, const PQueue * c)
      : PAbstractList(dummy, c)
      { reference->deleteObjects = c->reference->deleteObjects; }
};


/**Declare a queue class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>, and adds functionality
   that allows the list to be used as a first in first out queue. This macro
   closes the class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <code>PQueue</code> template class.

   See the <code>PList</code> class and <code>PDECLARE_QUEUE</code> macro for more
   information.
 */
#define PQUEUE(cls, T) typedef PQueue<T> cls


/**Begin declataion of a queue class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>, and adds functionality
   that allows the list to be used as a first in first out queue.

   If the compilation is using templates then this macro produces a descendent
   of the <code>PQueue</code> template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the <code>PQueue</code> and <code>PAbstractList</code> classes for more information.
 */
#define PDECLARE_QUEUE(cls, T) \
  PQUEUE(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, cls##_PTemplate) \
  protected: \
    cls(int dummy, const cls * c) \
      : cls##_PTemplate(dummy, c) { } \
  public: \
    cls() \
      : cls##_PTemplate() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/**This template class maps the PAbstractList to a specific object type, and
   adds functionality that allows the list to be used as a last in first out
   stack. The functions in this class primarily do all the appropriate casting
   of types.

   By default, objects placed into the set will <b>not</b> be deleted when
   removed or when all references to the set are destroyed. This is different
   from the default on most collection classes.

   Note that if templates are not used the <code>PDECLARE_STACK</code> macro will
   simulate the template instantiation.
 */
template <class T> class PStack : public PAbstractList
{
  PCLASSINFO(PStack, PAbstractList);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, stack.

       Note that by default, objects placed into the stack will <b>not</b> be
       deleted when removed or when all references to the stack are destroyed.
       This is different from the default on most collection classes.
     */
    PStack()
      : PAbstractList() { DisallowDeleteObjects(); }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Make a complete duplicate of the stack. Note that all objects in the
       array are also cloned, so this will make a complete copy of the stack.
     */
    virtual PObject * Clone() const
      { return PNEW PStack(0, this); }
  //@}

  /**@name New functions for class */
  //@{
    /**Add an object to the stack. This object will be on "top" of the stack
       and will be the object returned by the <code>Pop()</code>
       function.
     */
    virtual void Push(
      T * obj    ///< Object to add to the stack.
    ) { PAbstractList::Prepend(obj); }
    __inline void push(T * obj) { Push(obj); }

    /**Remove the last object pushed onto the stack.

       @return
       object on top of the stack.
     */
    virtual T * Pop()
      { return dynamic_cast<T *>(PAbstractList::RemoveHead()); }
    __inline void pop() { Pop(); }

    /**Get the element that is currently on top of the stack without removing
       it.

       @return
       reference to object on top of the stack.
     */
    virtual T & Top()
      { PAssert(this->GetSize() > 0, PStackEmpty); return dynamic_cast<T &>(*this->m_info->head->data); }
    __inline T & front() { Top(); }
  //@}

  protected:
    PStack(int dummy, const PStack * c)
      : PAbstractList(dummy, c)
      { this->reference->deleteObjects = c->reference->deleteObjects; }
};


/**Declare a stack class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>, and adds functionality
   that allows the list to be used as a last in first out stack. This macro
   closes the class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <code>PStack</code> template class.

   See the <code>PStack</code> class and <code>PDECLARE_STACK</code> macro for more
   information.
 */
#define PSTACK(cls, T) typedef PStack<T> cls


/**Begin declaration of a stack class.
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular object type <b>T</b>, and adds functionality
   that allows the list to be used as a last in first out stack.

   If the compilation is using templates then this macro produces a descendent
   of the <code>PStack</code> template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the <code>PStack</code> and <code>PAbstractList</code> classes for more information.
 */
#define PDECLARE_STACK(cls, T) \
  PSTACK(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, cls##_PTemplate) \
  protected: \
    cls(int dummy, const cls * c) \
      : cls##_PTemplate(dummy, c) { } \
  public: \
    cls() \
      : cls##_PTemplate() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


///////////////////////////////////////////////////////////////////////////////
// Sorted List of PObjects

struct PSortedListElement
{
  PSortedListElement(PSortedListElement * nil = NULL, PObject * obj = NULL);

  PSortedListElement * m_parent;
  PSortedListElement * m_left;
  PSortedListElement * m_right;
  PObject            * m_data;
  PINDEX               m_subTreeSize;
  enum { Red, Black }  m_colour;

  PDECLARE_POOL_ALLOCATOR(PSortedListElement);
};

struct PSortedListInfo
{
  PSortedListInfo() : m_root(&nil) { }

  PSortedListElement   nil;
  PSortedListElement * m_root;

  PSortedListElement * Successor(PSortedListElement * node) const;
  PSortedListElement * Predecessor(PSortedListElement * node) const;
  PSortedListElement * OrderSelect(PSortedListElement * node, PINDEX index) const;
  PSortedListElement * OrderSelect(PINDEX index) const { return OrderSelect(m_root, index); }
  PINDEX ValueSelect(PSortedListElement * node, const PObject & obj, PSortedListElement * & element) const;
  PINDEX ValueSelect(const PObject & obj, PSortedListElement * & element) const { return ValueSelect(m_root, obj, element); }

  PDECLARE_POOL_ALLOCATOR(PSortedListInfo);
};

/**This class is a collection of objects which are descendents of the
   <code>PObject</code> class. It is implemeted as a Red-Black binary tree to
   maintain the objects in rank order. Note that this requires that the
   <code>PObject::Compare()</code> function be fully implemented oin objects
   contained in the collection.

   The implementation of a sorted list allows fast inserting and deleting as
   well as random access of objects in the collection. As the objects are being
   kept sorted, "fast" is a relative term. All operations take o(lg n) unless
   a particular object is repeatedly accessed.

   The class remembers the last accessed element. This state information is
   used to optimise access by the "virtual array" model of collections. If
   repeated access via ordinal index is made there is little overhead. All
   other access incurs a minimum overhead, but not insignificant.

   The PAbstractSortedList class would very rarely be descended from directly
   by the user. The <code>PDECLARE_LIST</code> and <code>PLIST</code> macros would normally
   be used to create descendent classes. They will instantiate the template
   based on <code>PSortedList</code> or directly declare and define the class (using
   inline functions) if templates are not being used.

   The <code>PSortedList</code> class or <code>PDECLARE_SORTED_LIST</code> macro will
   define the correctly typed operators for subscript access
   (operator[]).
 */
class PAbstractSortedList : public PCollection
{
  PCONTAINERINFO(PAbstractSortedList, PCollection);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, sorted list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PAbstractSortedList();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Get the relative rank of the two lists. The following algorithm is
       employed for the comparison:

       \arg \c EqualTo if the two lists are identical in length
       and each objects values, not pointer, are equal.

       \arg \c LessThan if the instances object value at an
       ordinal position is less than the corresponding objects value in the
       <code>obj</code> parameters list.
                          
       This is also returned if all objects are equal and the instances list
       length is less than the <code>obj</code> parameters list length.

       \arg \c GreaterThan if the instances object value at an
       ordinal position is greater than the corresponding objects value in the
       <code>obj</code> parameters list.
                          
       This is also returned if all objects are equal and the instances list
       length is greater than the <code>obj</code> parameters list length.

       @return
       comparison of the two objects, <code>EqualTo</code> for same,
       <code>LessThan</code> for <code>obj</code> logically less than the
       object and <code>GreaterThan</code> for <code>obj</code> logically
       greater than the object.
     */
    virtual Comparison Compare(const PObject & obj) const;
  //@}

  /**@name Overrides from class PContainer */
  //@{
    /**This function is meaningless for lists. The size of the collection is
       determined by the addition and removal of objects. The size cannot be
       set in any other way.

       @return
       Always true.
     */
    virtual PBoolean SetSize(
      PINDEX newSize  // New size for the sorted list, this is ignored.
    );
  //@}

  /**@name Overrides from class PCollection */
  //@{
    /**Add a new object to the collection. The object is always placed in the
       correct ordinal position in the list. It is not placed at the "end".

       @return
       index of the newly added object.
     */
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );

    /**Add a new object to the collection.
    
       The object is always placed in the correct ordinal position in the list.
       It is not placed at the specified position. The <code>before</code>
       parameter is ignored.

       @return
       index of the newly inserted object.
     */
    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );

    /**Add a new object to the collection.
    
       The object is always placed in the correct ordinal position in the list.
       It is not placed at the specified position. The <code>index</code>
       parameter is ignored.

       @return
       index of the newly inserted object.
     */
    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );

    /**Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       Note that the comparison for searching for the object in collection is
       made by pointer, not by value. Thus the parameter must point to the
       same instance of the object that is in the collection.

       @return
       true if the object was in the collection.
     */
    virtual PBoolean Remove(
      const PObject * obj   // Existing object to remove from the collection.
    );

    /**Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       Note if the index is beyond the size of the collection then the
       function will assert.

       @return
       pointer to the object being removed, or NULL if it was deleted.
     */
    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    );

    /**Remove all of the elements in the collection. This operates by
       continually calling <code>RemoveAt()</code> until there are no objects left.

       The objects are removed from the last, at index
       <code>(GetSize()-1)</code> toward the first at index zero.
     */
    virtual void RemoveAll();

    /**This method simply returns false as the list order is mantained by the 
       class. Kept to mimic <code>PAbstractList</code> interface.
       
       @return
       false allways
     */
    virtual PBoolean SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );

    /**Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       @return
       pointer to object at the specified index.
     */
    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;

    /**Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A binary search is
       employed to locate the entry.
       
       Note that that will require value comparisons to be made to find the
       equivalent entry and then a final check is made with the pointers to
       see if they are the same instance.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;

    /**Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the <code>PObject::Compare()</code>
       function. A binary search is employed to locate the entry.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const;
  //@}

  protected:

    // New functions for class
    void RemoveElement(PSortedListElement * node);
    void LeftRotate(PSortedListElement * node);
    void RightRotate(PSortedListElement * node);
    void DeleteSubTrees(PSortedListElement * node, bool deleteObject);
    PSortedListElement * FindElement(const PObject & obj, PINDEX * index) const;
    PSortedListElement * FindElement(const PObject * obj, PINDEX * index) const;

    // The type below cannot be nested as DevStudio 2005 AUTOEXP.DAT doesn't like it
    PSortedListInfo * m_info;
};


/**This template class maps the PAbstractSortedList to a specific object type.
   The functions in this class primarily do all the appropriate casting of
   types.

   Note that if templates are not used the <code>PDECLARE_SORTED_LIST</code> macro
   will simulate the template instantiation.
 */
template <class T> class PSortedList : public PAbstractSortedList
{
  PCLASSINFO(PSortedList, PAbstractSortedList);

  public:
  /**@name Construction */
  //@{
    /**Create a new, empty, sorted list.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PSortedList()
      : PAbstractSortedList() { }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Make a complete duplicate of the list. Note that all objects in the
       array are also cloned, so this will make a complete copy of the list.
     */
    virtual PObject * Clone() const
      { return PNEW PSortedList(0, this); }
  //@}

  /**@name New functions for class */
  //@{
    /**Retrieve a reference  to the object in the list. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       The object accessed in this way is remembered by the class and further
       access will be fast.

       @return
       reference to the object at <code>index</code> position.
     */
    T & operator[](
      PINDEX index  ///< Index for entry
    ) const { return dynamic_cast<T &>(*this->GetAt(index)); }
  //@}

  /**@name Iterators */
  //@{
    typedef T value_type;
    friend class iterator_base;

  private:
    void NextElement(PSortedListElement * & element) const
    {
      element = this->m_info->Successor(element);
      if (element == &this->m_info->nil)
        element = NULL;
    }

    void PrevElement(PSortedListElement * & element) const
    {
      element = this->m_info->Predecessor(element);
      if (element == &this->m_info->nil)
        element = NULL;
    }

    class iterator_base : public std::iterator<std::bidirectional_iterator_tag, value_type> {
      protected:
        const PSortedList<T> * m_list;
        PSortedListElement   * m_element;

        iterator_base(const PSortedList<T> * l, PSortedListElement * e) : m_list(l), m_element(e) { }

        bool Valid() const { return PAssert(this->m_list != NULL && this->m_element != NULL && this->m_element != &m_list->m_info->nil, PInvalidArrayIndex); }
        void Next() { if (Valid()) this->m_list->NextElement(this->m_element); }
        void Prev() { if (Valid()) this->m_list->PrevElement(this->m_element); }
        value_type * Ptr() const { return dynamic_cast<value_type *>(Valid() ? this->m_element->m_data : NULL); }

      public:
        bool operator==(const iterator_base & it) const { return this->m_element == it.m_element; }
        bool operator!=(const iterator_base & it) const { return this->m_element != it.m_element; }

      friend class PSortedList<T>;
    };

  public:
    class iterator : public iterator_base {
      public:
        iterator() : iterator_base(NULL, NULL) { }
        iterator(PSortedList<T> * l, PSortedListElement * e) : iterator_base(l, e) { }

        iterator operator++()    {                      this->Next(); return *this; }
        iterator operator--()    {                      this->Prev(); return *this; }
        iterator operator++(int) { iterator it = *this; this->Next(); return it;    }
        iterator operator--(int) { iterator it = *this; this->Prev(); return it;    }

        value_type * operator->() const { return  this->Ptr(); }
        value_type & operator* () const { return *this->Ptr(); }
    };

    iterator begin()  { return IsEmpty() ? iterator() : iterator(this, this->m_info->OrderSelect(1));                 }
    iterator end()    { return             iterator();                                                                }
    iterator rbegin() { return IsEmpty() ? iterator() : iterator(this, this->m_info->OrderSelect(this->GetSize())); }
    iterator rend()   { return             iterator();                                                                }

    class const_iterator : public iterator_base {
      public:
        const_iterator() : iterator_base(NULL, NULL) { }
        const_iterator(const PSortedList<T> * l, PSortedListElement * e) : iterator_base(l, e) { }

        const_iterator operator++()    {                            this->Next(); return *this; }
        const_iterator operator--()    {                            this->Prev(); return *this; }
        const_iterator operator++(int) { const_iterator it = *this; this->Next(); return it;    }
        const_iterator operator--(int) { const_iterator it = *this; this->Prev(); return it;    }

        const value_type * operator->() const { return  this->Ptr(); }
        const value_type & operator* () const { return *this->Ptr(); }
    };

    const_iterator begin()  const { return IsEmpty() ? const_iterator() : const_iterator(this, this->m_info->OrderSelect(1));                 }
    const_iterator end()    const { return             const_iterator();                                                                            }
    const_iterator rbegin() const { return IsEmpty() ? const_iterator() : const_iterator(this, this->m_info->OrderSelect(this->GetSize())); }
    const_iterator rend()   const { return             const_iterator();                                                                            }

    value_type & front() { return *this->begin(); }
    value_type & back()  { return *this->rbegin(); }
    const value_type & front() const { return *this->begin(); }
    const value_type & back()  const { return *this->rbegin(); }

          iterator find(const value_type & obj)       { return       iterator(this, this->FindElement(obj, NULL));                      }
    const_iterator find(const value_type & obj) const { return const_iterator(this, this->FindElement(obj, NULL)); }

    void erase(const iterator & it)       { PAssert(this == it.m_list, PLogicError); this->RemoveElement(it.m_element); }
    void erase(const const_iterator & it) { PAssert(this == it.m_list, PLogicError); this->RemoveElement(it.m_element); }
    __inline void insert(const value_type & value) { this->Append(new value_type(value)); }
    __inline void pop_front() { this->erase(this->begin()); }
    __inline void pop_back() { this->erase(this->rbegin()); }
  //@}

  protected:
    PSortedList(int dummy, const PSortedList * c)
      : PAbstractSortedList(dummy, c) { }
};


/**Declare a sorted list class.
   This macro is used to declare a descendent of PAbstractSortedList class,
   customised for a particular object type <b>T</b>. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <code>PSortedList</code> template class.

   See the <code>PSortedList</code> class and <code>PDECLARE_SORTED_LIST</code> macro for
   more information.
 */
#define PSORTED_LIST(cls, T) typedef PSortedList<T> cls


/**Begin declaration of a sorted list class.
   This macro is used to declare a descendent of PAbstractSortedList class,
   customised for a particular object type <b>T</b>.

   If the compilation is using templates then this macro produces a descendent
   of the <code>PSortedList</code> template class. If templates are not being used
   then the macro defines a set of inline functions to do all casting of types.
   The resultant classes have an identical set of functions in either case.

   See the <code>PSortedList</code> and <code>PAbstractSortedList</code> classes for more
   information.
 */
#define PDECLARE_SORTED_LIST(cls, T) \
  PSORTED_LIST(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, PSortedList<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PSortedList<T>(dummy, c) { } \
  public: \
    cls() \
      : PSortedList<T>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


#endif // PTLIB_LISTS_H


// End Of File ///////////////////////////////////////////////////////////////

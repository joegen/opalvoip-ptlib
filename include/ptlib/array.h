/*
 * array.h
 *
 * Linear Array Container classes.
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
 * $Log: array.h,v $
 * Revision 1.12  1998/09/23 06:20:16  robertj
 * Added open source copyright license.
 *
 * Revision 1.11  1998/08/21 05:23:57  robertj
 * Added hex dump capability to base array types.
 * Added ability to have base arrays of static memory blocks.
 *
 * Revision 1.10  1997/06/08 04:49:10  robertj
 * Fixed non-template class descendent order.
 *
 * Revision 1.9  1996/08/17 09:54:34  robertj
 * Optimised RemoveAll() for object arrays.
 *
 * Revision 1.8  1996/01/02 11:48:46  robertj
 * Removed requirement that PArray elements have parameterless constructor..
 *
 * Revision 1.7  1995/10/14 14:52:33  robertj
 * Changed arrays to not break references.
 *
 * Revision 1.6  1995/06/17 11:12:18  robertj
 * Documentation update.
 *
 * Revision 1.5  1995/03/14 12:40:58  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.4  1995/02/22  10:50:26  robertj
 * Changes required for compiling release (optimised) version.
 *
 * Revision 1.3  1995/01/15  04:49:09  robertj
 * Fixed errors in template version.
 *
 * Revision 1.2  1994/12/21  11:52:46  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.1  1994/12/12  09:59:29  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif

///////////////////////////////////////////////////////////////////////////////
// The abstract array class

class PArrayObjects;

PDECLARE_CONTAINER(PAbstractArray, PContainer)
/* This class contains a variable length array of arbitrary memory blocks.
   These can be anything from individual bytes to large structures. Note that
   that does <EM>not</EM> include class objects that require construction or
   destruction. Elements in this array will not execute the contructors or
   destructors of objects.

   An abstract array consists of a linear block of memory sufficient to hold
   <A>PContainer::GetSize()</A> elements of <CODE>elementSize</CODE> bytes
   each. The memory block itself will atuomatically be resized when required
   and freed when no more references to it are present.

   The PAbstractArray class would very rarely be descended from directly by
   the user. The <A>PDECLARE_BASEARRAY</A> and <A>PBASEARRAY</A> macros would
   normally be used to create descendent classes. They will instantiate the
   template based on <A>PBaseArray</A> or directly declare and define the class
   (using inline functions) if templates are not being used.

   The <A>PBaseArray</A> class or <A>PDECLARE_BASEARRAY</A> macro will define
   the correctly typed operators for pointer access
   (<CODE>operator const T *</CODE>) and subscript access
   (<CODE>operator[]</CODE>).
 */

  friend class PArrayObjects;

  public:
    PAbstractArray(
      PINDEX elementSizeInBytes,
      /* Size of each element in the array. This must be > 0 or the
         constructor will assert.
       */
      PINDEX initialSize = 0      // Number of elements to allocate initially.
    );
    /* Create a new dynamic array of <CODE>initalSize</CODE> elements of
       <CODE>elementSizeInBytes</CODE> bytes each. The array memory is
       initialised to zeros.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the <A>PDECLARE_BASEARRAY</A> macro.
     */

    PAbstractArray(
      PINDEX elementSizeInBytes,
      /* Size of each element in the array. This must be > 0 or the
         constructor will assert.
       */
      const void *buffer,          // Pointer to an array of elements.
      PINDEX bufferSizeInElements, // Number of elements pointed to by buffer.
      BOOL dynamicAllocation       // Buffer is copied and dynamically allocated.
    );
    /* Create a new dynamic array of <CODE>bufferSizeInElements</CODE>
       elements of <CODE>elementSizeInBytes</CODE> bytes each. The contents of
       the memory pointed to by buffer is then used to initialise the newly
       allocated array.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the <A>PDECLARE_BASEARRAY</A> macro.

       If the <CODE>dynamicAllocation</CODE> parameter is FALSE then the
       pointer is used directly by the container. It will not be copied to a
       dynamically allocated buffer. If the SetSize() function is used to
       change the size of the buffer, the object will be converted to a
       dynamic form with the contents of the static buffer copied to the
       allocated buffer.
     */

  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Other PAbstractArray to compare against.
    ) const;
    /* Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:
          
          <DL>
          <DT>EqualTo<DD>     if the two array memory blocks are identical in
                              length and contents.
          <DT>LessThan<DD>    if the array length is less than the
                              <CODE>obj</CODE> parameters array length.
          <DT>GreaterThan<DD> if the array length is greater than the
                              <CODE>obj</CODE> parameters array length.
          </DL>

        If the array sizes are identical then the <CODE>memcmp()</CODE>
        function is used to rank the two arrays.

       <H2>Returns:</H2>
       comparison of the two objects, <CODE>EqualTo</CODE> for same,
       <CODE>LessThan</CODE> for <CODE>obj</CODE> logically less than the
       object and <CODE>GreaterThan</CODE> for <CODE>obj</CODE> logically
       greater than the object.
     */

  // Overrides from class PContainer
    virtual BOOL SetSize(
      PINDEX newSize  // New size of the array in elements.
    );
    /* Set the size of the array in elements. A new array may be allocated to
       accomodate the new number of elements. If the array increases in size
       then the new bytes are initialised to zero. If the array is made smaller
       then the data beyond the new size is lost.

       <H2>Returns:</H2>
       TRUE if the memory for the array was allocated successfully.
     */

  // New functions for class
    void Attach(
      const void *buffer, // Pointer to an array of elements.
      PINDEX bufferSize   // Number of elements pointed to by buffer.
    );
    /* Attach a pointer to a static block to the base array type. The pointer
       is used directly and will not be copied to a dynamically allocated
       buffer. If the SetSize() function is used to change the size of the
       buffer, the object will be converted to a dynamic form with the
       contents of the static buffer copied to the allocated buffer.
       
       Any dynamically allocated buffer will be freed.
     */

    void * GetPointer(
      PINDEX minSize = 1  // Minimum size the array must be.
    );
    /* Get a pointer to the internal array and assure that it is of at least
       the specified size. This is useful when the array contents are being
       set by some external or system function eg file read.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       <H2>Returns:</H2>
       pointer to the array memory.
     */

  protected:
    void PrintNumbersOn(ostream & strm, PINDEX size, BOOL is_signed) const;
    virtual long GetNumberValueAt(PINDEX idx) const;

    PINDEX elementSize;
    // Size of an element in bytes

    char * theArray;
    // Pointer to the allocated block of memory.

    BOOL allocatedDynamically;
};



///////////////////////////////////////////////////////////////////////////////
// An array of some base type

#ifdef PHAS_TEMPLATES

template <class T>
PDECLARE_CLASS(PBaseArray, PAbstractArray)
/* This template class maps the PAbstractArray to a specific element type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the <A>PDECLARE_BASEARRAY</A> macro will
   simulate the template instantiation.

   The following classes are instantiated automatically for the basic scalar
   types:
        <UL>
        <LI><CODE><A>PCharArray</A></CODE>
        <LI><CODE><A>PBYTEArray</A></CODE>
        <LI><CODE><A>PShortArray</A></CODE>
        <LI><CODE><A>PWORDArray</A></CODE>
        <LI><CODE><A>PIntArray</A></CODE>
        <LI><CODE><A>PUnsignedArray</A></CODE>
        <LI><CODE><A>PLongArray</A></CODE>
        <LI><CODE><A>PDWORDArray</A></CODE>
        </UL>
 */

  public:
    PBaseArray(
      PINDEX initialSize = 0  // Initial number of elements in the array.
    ) : PAbstractArray(sizeof(T), initialSize) { }
    /* Construct a new dynamic array of elements of the specified type. The
       array is initialised to all zero bytes. Note that this may not be
       logically equivalent to the zero value for the type, though this would
       be very rare.
     */
    
    PBaseArray(
      T const * buffer,   // Pointer to an array of the elements of type <B>T</B>.
      PINDEX length,      // Number of elements pointed to by <CODE>buffer</CODE>.
      BOOL dynamic = TRUE // Buffer is copied and dynamically allocated.
    ) : PAbstractArray(sizeof(T), buffer, length, dynamic) { }
    /* Construct a new dynamic array of elements of the specified type.
     */

    virtual PObject * Clone() const
      { return PNEW PBaseArray<T>(*this, GetSize()); }


    BOOL SetAt(
      PINDEX index,   // Position in the array to set the new value.
      T val           // Value to set in the array.
    ) { return SetMinSize(index+1) && val==(((T *)theArray)[index] = val); }
    /* Set the specific element in the array. The array will automatically
       expand, if necessary, to fit the new element in.

       <H2>Returns:</H2>
       TRUE if new memory for the array was successfully allocated.
     */

    T GetAt(
      PINDEX index  // Position on the array to get value from.
    ) const { PASSERTINDEX(index);
                    return index < GetSize() ? ((T *)theArray)[index] : (T)0; }
    /* Get a value from the array. If the <CODE>index</CODE> is beyond the end
       of the allocated array then a zero value is returned.

       <H2>Returns:</H2>
       value at the array position.
     */

    T operator[](
      PINDEX index  // Position on the array to get value from.
    ) const { return GetAt(index); }
    /* Get a value from the array. If the <CODE>index</CODE> is beyond the end
       of the allocated array then a zero value is returned.

       This is functionally identical to the <A>PContainer::GetAt()</A>
       function.

       <H2>Returns:</H2>
       value at the array position.
     */

    T & operator[](
      PINDEX index  // Position on the array to get value from.
    ) { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory);
        return ((T *)theArray)[index]; }
    /* Get a reference to value from the array. If the <CODE>index</CODE> is
       beyond the end of the allocated array then the array is expanded. If a
       memory allocation failure occurs the function asserts.

       This is functionally similar to the <A>SetAt()</A> function and allows
       the array subscript to be an lvalue.

       <H2>Returns:</H2>
       reference to value at the array position.
     */

    void Attach(
      const T * buffer,   // Pointer to an array of elements.
      PINDEX bufferSize   // Number of elements pointed to by buffer.
    );
    /* Attach a pointer to a static block to the base array type. The pointer
       is used directly and will not be copied to a dynamically allocated
       buffer. If the SetSize() function is used to change the size of the
       buffer, the object will be converted to a dynamic form with the
       contents of the static buffer copied to the allocated buffer.
       
       Any dynamically allocated buffer will be freed.
     */
    T * GetPointer(
      PINDEX minSize = 0
    ) { return (T *)PAbstractArray::GetPointer(minSize); }
    /* Get a pointer to the internal array and assure that it is of at least
       the specified size. This is useful when the array contents are being
       set by some external or system function eg file read.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       <H2>Returns:</H2>
       pointer to the array memory.
     */

    operator T const *() const { return (T const *)theArray; }
    /* Get a pointer to the internal array. The user may not modify the
       contents of this pointer/ This is useful when the array contents are
       required by some external or system function eg file write.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       <H2>Returns:</H2>
       constant pointer to the array memory.
     */
};

/*$MACRO PDECLARE_BASEARRAY(cls, T)
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type <B>T</B>.

   If the compilation is using templates then this macro produces a descendent
   of the <A>PBaseArray</A> template class. If templates are not being used
   then the macro defines a set of inline functions to do all casting of types.
   The resultant classes have an identical set of functions in either case.

   See the <A>PBaseArray</A> and <A>PAbstractArray</A> classes for more
   information.
 */
#define PDECLARE_BASEARRAY(cls, T) \
  typedef PBaseArray<T> PBaseArray_##cls; \
  PDECLARE_CLASS(cls, PBaseArray_##cls) \
    cls(PINDEX initialSize = 0) \
      : PBaseArray_##cls(initialSize) { } \
    cls(T const * buffer, PINDEX length, BOOL dynamic = TRUE) \
      : PBaseArray_##cls(buffer, length, dynamic) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \

/*$MACRO PBASEARRAY(cls, T)
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type <B>T</B>. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <A>PBaseArray</A> template class.

   See the <A>PDECLARE_BASEARRAY</A> for more information.
 */
#define PBASEARRAY(cls, T) typedef PBaseArray<T> cls

#else // PHAS_TEMPLATES

#define PBASEARRAY(cls, T) \
  typedef T P_##cls##_Base_Type; \
  PDECLARE_CLASS(cls, PAbstractArray) \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), initialSize) { } \
    inline cls(P_##cls##_Base_Type const * buffer, PINDEX length, BOOL dynamic = TRUE) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), buffer, length, dynamic) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \
    inline BOOL SetAt(PINDEX index, P_##cls##_Base_Type val) \
      { return SetMinSize(index+1) && \
                     val==(((P_##cls##_Base_Type *)theArray)[index] = val); } \
    inline P_##cls##_Base_Type GetAt(PINDEX index) const \
      { PASSERTINDEX(index); return index < GetSize() ? \
          ((P_##cls##_Base_Type*)theArray)[index] : (P_##cls##_Base_Type)0; } \
    inline P_##cls##_Base_Type operator[](PINDEX index) const \
      { PASSERTINDEX(index); return GetAt(index); } \
    inline P_##cls##_Base_Type & operator[](PINDEX index) \
      { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory); \
        return ((P_##cls##_Base_Type *)theArray)[index]; } \
    inline void Attach(const P_##cls##_Base_Type * buffer, PINDEX bufferSize) \
      { PAbstractArray::Attach(buffer, bufferSize); } \
    inline P_##cls##_Base_Type * GetPointer(PINDEX minSize = 0) \
      { return (P_##cls##_Base_Type *)PAbstractArray::GetPointer(minSize); } \
    inline operator P_##cls##_Base_Type const *() const \
      { return (P_##cls##_Base_Type const *)theArray; } \
  }

#define PDECLARE_BASEARRAY(cls, T) \
  PBASEARRAY(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, cls##_PTemplate) \
    cls(PINDEX initialSize = 0) \
      : cls##_PTemplate(initialSize) { } \
    cls(T const * buffer, PINDEX length, BOOL dynamic = TRUE) \
      : cls##_PTemplate(buffer, length, dynamic) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \

#endif // PHAS_TEMPLATES


PDECLARE_BASEARRAY(PCharArray, char)
  public:
    virtual void PrintOn(ostream & strm) const;
};

PDECLARE_BASEARRAY(PShortArray, short)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PIntArray, int)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PLongArray, long)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PBYTEArray, BYTE)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PWORDArray, WORD)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PUnsignedArray, unsigned)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};

PDECLARE_BASEARRAY(PDWORDArray, DWORD)
  public:
    virtual void PrintOn(ostream & strm) const;
    virtual long GetNumberValueAt(PINDEX idx) const;
};


///////////////////////////////////////////////////////////////////////////////
// Linear array of objects

PDECLARE_CONTAINER(PArrayObjects, PCollection)
/* This class is a collection of objects which are descendents of the
   <A>PObject</A> class. It is implemeted as a dynamic, linear array of
   pointers to the objects.

   The implementation of an array allows very fast random access to items in
   the collection, but has severe penalties for inserting and deleting objects
   as all other objects must be moved to accommodate the change.

   An array of objects may have "gaps" in it. These are array entries that
   contain NULL as the object pointer.

   The PArrayObjects class would very rarely be descended from directly by
   the user. The <A>PDECLARE_ARRAY</A> and <A>PARRAY</A> macros would normally
   be used to create descendent classes. They will instantiate the template
   based on <A>PArray</A> or directly declare and define the class (using
   inline functions) if templates are not being used.

   The <A>PArray</A> class or <A>PDECLARE_ARRAY</A> macro will define the
   correctly typed operators for pointer access (operator const T *) and
   subscript access (operator[]).
 */

  public:
    PArrayObjects(
      PINDEX initialSize = 0  // Initial number of objects in the array.
    );
    /* Create a new array of objects. The array is initially set to the
       specified size with each entry having NULL as is pointer value.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Other <A>PAbstractArray</A> to compare against.
    ) const;
    /* Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:

        <DL>
        <DT>EqualTo<DD>     if the two array memory blocks are identical in
                            length and each objects values, not pointer, are
                            equal.

        <DT>LessThan<DD>    if the instances object value at an ordinal
                            position is less than the corresponding objects
                            value in the <CODE>obj</CODE> parameters array.
                          
                            This is also returned if all objects are equal and
                            the instances array length is less than the
                            <CODE>obj</CODE> parameters array length.

        <DT>GreaterThan<DD> if the instances object value at an ordinal
                            position is greater than the corresponding objects
                            value in the <CODE>obj</CODE> parameters array.
                          
                            This is also returned if all objects are equal and
                            the instances array length is greater than the
                            <CODE>obj</CODE> parameters array length.
          </DL>

       <H2>Returns:</H2>
       comparison of the two objects, <CODE>EqualTo</CODE> for same,
       <CODE>LessThan</CODE> for <CODE>obj</CODE> logically less than the
       object and <CODE>GreaterThan</CODE> for <CODE>obj</CODE> logically
       greater than the object.
     */

  // Overrides from class PContainer
    virtual PINDEX GetSize() const;
    //$ANCESTOR

    virtual BOOL SetSize(
      PINDEX newSize  // New size of the array in objects.
    );
    /* Set the size of the array in objects. A new array may be allocated to
       accomodate the new number of objects. If the array increases in size
       then the new object pointers are initialised to NULL. If the array is
       made smaller then the data beyond the new size is lost.

       <H2>Returns:</H2>
       TRUE if the memory for the array was allocated successfully.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* Append a new object to the collection. This will increase the size of
       the array by one and place the new object at that position.
    
       <H2>Returns:</H2>
       index of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );
    /* Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the <A>Append()</A> function is performed.

       All objects, including the <CODE>before</CODE> object are shifted up
       one in the array.

       Note that the object values are compared for the search of the
       <CODE>before</CODE> parameter, not the pointers. So the objects in the
       collection must correctly implement the <A>PObject::Compare()</A>
       function.

       <H2>Returns:</H2>
       index of the newly inserted object.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );
    /* Insert a new object at the specified ordinal index. If the index is
       greater than the number of objects in the collection then the
       equivalent of the <A>Append()</A> function is performed.

       All objects, including the <CODE>index</CODE> position object are
       shifted up one in the array.

       <H2>Returns:</H2>
       index of the newly inserted object.
     */

    virtual BOOL Remove(
      const PObject * obj   // Existing object to remove from the collection.
    );
    /* Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

       <H2>Returns:</H2>
       TRUE if the object was in the collection.
     */

    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    );
    /* Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

       Note if the index is beyond the size of the collection then the
       function will assert.

       <H2>Returns:</H2>
       pointer to the object being removed, or NULL if it was deleted.
     */

    virtual BOOL SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );
    /* Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.

       <H2>Returns:</H2>
       TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;
    /* Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       <H2>Returns:</H2>
       pointer to object at the specified index.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A simple linear search
       from ordinal position zero is performed.

       <H2>Returns:</H2>
       ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const;
    /* Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the <A>PObject::Compare()</A>
       function. A simple linear search from ordinal position zero is
       performed.

       <H2>Returns:</H2>
       ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual void RemoveAll();
    /* Remove all of the elements in the collection. This operates by
       continually calling <A>RemoveAt()</A> until there are no objects left.

       The objects are removed from the last, at index
       <CODE>(GetSize()-1)</CODE> toward the first at index zero.
     */


  protected:
    PBASEARRAY(ObjPtrArray, PObject *);
    ObjPtrArray * theArray;
};


#ifdef PHAS_TEMPLATES

template <class T>
PDECLARE_CLASS(PArray, PArrayObjects)
/* This template class maps the PArrayObjects to a specific object type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the <A>PDECLARE_ARRAY</A> macro will
   simulate the template instantiation.
 */

  public:
    PArray( 
      PINDEX initialSize = 0  // Initial number of objects in the array.
    ) : PArrayObjects(initialSize) { }
    /* Create a new array of objects. The array is initially set to the
       specified size with each entry having NULL as is pointer value.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW PArray(0, this); }
    /* Make a complete duplicate of the array. Note that all objects in the
       array are also cloned, so this will make a complete copy of the array.
     */

    T & operator[](
      PINDEX index  // Index position in the collection of the object.
    ) const { PAssert(GetAt(index) != NULL, PInvalidArrayElement);
                                                   return *(T *)GetAt(index); }
    /* Retrieve a reference  to the object in the array. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       <H2>Returns:</H2>
       reference to the object at <CODE>index</CODE> position.
     */


  protected:
    PArray(int dummy, const PArray * c) : PArrayObjects(dummy, c) { }
};


/*$MACRO PDECLARE_ARRAY(cls, T)
   This macro is used to declare a descendent of PArrayObjects class,
   customised for a particular object type <B>T</B>.

   If the compilation is using templates then this macro produces a descendent
   of the <A>PArray</A> template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the <A>PBaseArray</A> and <A>PAbstractArray</A> classes for more
   information.
 */
#define PDECLARE_ARRAY(cls, T) \
  PDECLARE_CLASS(cls, PArray<T>) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PArray<T>(dummy, c) { } \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PArray<T>(initialSize) { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PARRAY(cls, T)
   This macro is used to declare a descendent of PArrayObjects class,
   customised for a particular object type <B>T</B>. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the <A>PArray</A> template class.

   See the <A>PBaseArray</A> class and <A>PDECLARE_ARRAY</A> macro for more
   information.
 */
#define PARRAY(cls, T) typedef PArray<T> cls


#else // PHAS_TEMPLATES


#define PARRAY(cls, T) \
  PDECLARE_CLASS(cls, PArrayObjects) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PArrayObjects(dummy, c) { } \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PArrayObjects(initialSize) { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const\
      { PAssert((*theArray)[index] != NULL, PInvalidArrayElement); \
                                           return *(T *)(*theArray)[index]; } \
  }

#define PDECLARE_ARRAY(cls, T) \
  PARRAY(cls##_PTemplate, T); \
  PDECLARE_CLASS(cls, cls##_PTemplate) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : cls##_PTemplate(dummy, c) { } \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : cls##_PTemplate(initialSize) { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \

#endif // PHAS_TEMPLATES


// End Of File ///////////////////////////////////////////////////////////////

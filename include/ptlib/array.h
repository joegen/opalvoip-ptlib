/*
 * $Id: array.h,v 1.1 1994/12/12 09:59:29 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: array.h,v $
 * Revision 1.1  1994/12/12 09:59:29  robertj
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
   that does $U$not$U$ include class objects that require construction or
   destruction. Elements in this array will not execute the contructors or
   destructors of objects.

   An abstract array consists of a linear block of memory sufficient to hold
   $B$GetSize()$B$ elements of $B$elementSize$B$ bytes each. The memory block
   itself will atuomatically be resized when required and freed when no more
   references to it are present.

   An important feature of abstract array classes, which is not normally
   present in container classes, is that when the array contents is changed, 
   that is resized or elemnts set, the array is derefrerenced, and a duplicate
   made of its contents. That is this instance of the array is disconnected
   from all other references, if any, and a new array contents created. For
   example consider the following:$F$

          PString s1 = "String"; // New array allocated and set to "String"
          PString s2 = s1;       // s2 has pointer to same array as s1
                                 // and reference count is 2 for both
          s1[0] = 's';           // Breaks references into different strings

   at the end s1 is "string" and s2 is "String" both with reference count of 1.

   The PAbstractArray class would very rarely be descended from directly by
   the user. The $H$PDECLARE_BASEARRAY and $H$PBASEARRAY macros would normally be
   used to create descendent classes. They will instantiate the template based
   on $H$PBaseArray or directly declare and define the class (using inline
   functions) if templates are not being used.

   The $H$PBaseArray class or $H$PDECLARE_BASEARRAY macro will define the
   correctly typed operators for pointer access (operator const T *) and
   subscript access (operator[]).
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
    /* Create a new dynamic array of $B$initalSize$B$ elements of
       $B$elementSizeInBytes$B$ bytes each. The array memory is initialised to
       zeros.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the $H$PDECLARE_BASEARRAY macro.
     */

    PAbstractArray(
      PINDEX elementSizeInBytes,
      /* Size of each element in the array. This must be > 0 or the
         constructor will assert.
       */
      const void *buffer,         // Pointer to an array of elements.
      PINDEX bufferSizeInElements // Number of elements pointed to by buffer.
    );
    /* Create a new dynamic array of $B$bufferSizeInElements$B$ elements of
       $B$elementSizeInBytes$B$ bytes each. The contents of the memory pointed
       to by buffer is then used to initialise the newly allocated array.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the $H$PDECLARE_BASEARRAY macro.
     */

  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Other PAbstractArray to compare against.
    ) const;
    /* Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:
        
        $I$EqualTo$I$     if the two array memory blocks are identical in
                          length and contents.
        $I$LessThan$I$    if the array length is less than the $B$obj$B$
                          parameters array length.
        $I$GreaterThan$I$ if the array length is greater than the $B$obj$B$
                          parameters array length.

        If the array sizes are identical then the memcmp() function is used
        to rank the two arrays.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
     */

  // Overrides from class PContainer
    virtual BOOL SetSize(
      PINDEX newSize  // New size of the array in elements.
    );
    /* Set the size of the array in elements. A new array may be allocated to
       accomodate the new number of elements. If the array increases in size
       then the new bytes are initialised to zero. If the array is made smaller
       then the data beyond the new size is lost.

       Note that this function will break the current instance from multiple
       references to an array. A new array is allocated and the data from the
       old array copied to it.

       Returns: TRUE if the memory for the array was allocated successfully.
     */

  // New functions for class
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

       Note that this function will break the current instance from multiple
       references to an array. A new array is allocated and the data from the
       old array copied to it.

       Returns: pointer to the array memory.
     */

  protected:
    PINDEX elementSize;
    // Size of an element in bytes

    char * theArray;
    // Pointer to the allocated block of memory.
};



///////////////////////////////////////////////////////////////////////////////
// An array of some base type

#ifdef PHAS_TEMPLATES

template <class T> PDECLARE_CLASS(PBaseArray, PAbstractArray)
/* This template class maps the PAbstractArray to a specific element type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the $H$PDECLARE_BASEARRAY macro will
   simulate the template instantiation.

   The following classes are instantiated automatically for the basic scalar
   types:
        PCharArray
        PBYTEArray
        PShortArray
        PWORDArray
        PIntArray
        PUnsignedArray
        PLongArray
        PDWORDArray
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
      T const * buffer, // Pointer to an array of the elements of type $B$T$B$.
      PINDEX length     // Number of elements pointed to by $B$buffer$B$.
    ) : PAbstractArray(sizeof(T), buffer, length) { }
    /* Construct a new dynamic array of elements of the specified type.
     */

    BOOL SetAt(
      PINDEX index,   // Position in the array to set the new value.
      T val           // Value to set in the array.
    ) { return SetMinSize(index+1) && val==(((T *)theArray)[index] = val); }
    /* Set the specific element in the array. The array will automatically
       expand, if necessary, to fit the new element in.

       Note that this function will break the current instance from multiple
       references to an array. A new array is allocated and the data from the
       old array copied to it.

       Returns: TRUE if new memory for the array was successfully allocated.
     */

    T GetAt(
      PINDEX index  // Position on the array to get value from.
    ) const { PASSERTINDEX(index);
                    return index < GetSize() ? ((T *)theArray)[index] : (T)0; }
    /* Get a value from the array. If the $B$index$B$ is beyond the end of the
       allocated array then a zero value is returned.

       Returns: value at the array position.
     */

    T operator[](
      PINDEX index  // Position on the array to get value from.
    ) const { return GetAt(index); }
    /* Get a value from the array. If the $B$index$B$ is beyond the end of the
       allocated array then a zero value is returned.

       This is functionally identical to the $B$GetAt()$B$ function.

       Returns: value at the array position.
     */

    T & operator[](
      PINDEX index  // Position on the array to get value from.
    ) { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory);
        return ((T *)theArray)[index]; }
    /* Get a reference to value from the array. If the $B$index$B$ is beyond
       the end of the allocated array then the array is expanded. If a memory
       allocation failure occurs the function asserts.

       This is functionally similar to the $B$SetAt()$B$ function and allows
       the array subscript to be an lvalue.

       Returns: reference to value at the array position.
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

       Note that this function will break the current instance from multiple
       references to an array. A new array is allocated and the data from the
       old array copied to it.

       Returns: pointer to the array memory.
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

       Returns: constant pointer to the array memory.
     */
};

/*$MACRO PDECLARE_BASEARRAY(cls, T)
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type $B$T$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PBaseArray template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PBaseArray and $H$PAbstractArray classes for more information.
 */
#define PDECLARE_BASEARRAY(cls, T) \
  PDECLARE_CLASS(cls, PBaseArray<T>) \
    cls(PINDEX initialSize = 0) \
      : PBaseArray<T>(initialSize) { } \
    cls(T const * buffer, PINDEX length) \
      : PBaseArray<T>(buffer, length) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \

/*$MACRO PBASEARRAY(cls, T)
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type $B$T$B$. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PBaseArray template class.

   See the $H$PDECLARE_BASEARRAY for more information.
 */
#define PBASEARRAY(cls, T) typedef PBaseArray<T> cls;

#else

#define PDECLARE_BASEARRAY(cls, T) \
  typedef T P_##cls##_Base_Type; \
  PDECLARE_CLASS(cls, PAbstractArray) \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), initialSize) { } \
    inline cls(P_##cls##_Base_Type const * buffer, PINDEX length) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), buffer, length) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \
    inline BOOL SetAt(PINDEX index, P_##cls##_Base_Type val) \
      { return SetMinSize(index+1) && val==(((P_##cls##_Base_Type *)theArray)[index] = val); } \
    inline P_##cls##_Base_Type GetAt(PINDEX index) const \
      { PASSERTINDEX(index); return index < GetSize() ? ((P_##cls##_Base_Type *)theArray)[index] : (P_##cls##_Base_Type)0; } \
    inline P_##cls##_Base_Type * GetPointer(PINDEX minSize = 0) \
      { return (P_##cls##_Base_Type *)PAbstractArray::GetPointer(minSize); } \
    inline P_##cls##_Base_Type operator[](PINDEX index) const \
      { PASSERTINDEX(index); return GetAt(index); } \
    inline P_##cls##_Base_Type & operator[](PINDEX index) \
      { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory); \
        return ((P_##cls##_Base_Type *)theArray)[index]; } \
    inline operator P_##cls##_Base_Type const *() const \
      { return (P_##cls##_Base_Type const *)theArray; } \

#define PBASEARRAY(cls, T) PDECLARE_BASEARRAY(cls, T) }

#endif


PBASEARRAY(PCharArray, char);
PBASEARRAY(PShortArray, short);
PBASEARRAY(PIntArray, int);
PBASEARRAY(PLongArray, long);
PBASEARRAY(PBYTEArray, BYTE);
PBASEARRAY(PWORDArray, WORD);
PBASEARRAY(PUnsignedArray, unsigned);
PBASEARRAY(PDWORDArray, DWORD);



///////////////////////////////////////////////////////////////////////////////
// Linear array of objects

PDECLARE_CONTAINER(PArrayObjects, PCollection)
/* This class is a collection of objects which are descendents of the
   $H$PObject class. It is implemeted as a dynamic, linear array of pointers
   to the objects.

   The implementation of an array allows very fast random access to items in
   the collection, but has severe penalties for inserting and deleting objects
   as all other objects must be moved to accommodate the change.

   An array of objects may have "gaps" in it. These are array entries that
   contain NULL as the object pointer.

   The PArrayObjects class would very rarely be descended from directly by
   the user. The $H$PDECLARE_ARRAY and $H$PARRAY macros would normally be used
   to create descendent classes. They will instantiate the template based on
   $H$PArray or directly declare and define the class (using inline functions)
   if templates are not being used.

   The $H$PArray class or $H$PDECLARE_ARRAY macro will define the correctly
   typed operators for pointer access (operator const T *) and subscript access
   (operator[]).
 */

  public:
    PINLINE PArrayObjects(
      PINDEX initialSize = 0  // Initial number of objects in the array.
    );
    /* Create a new array of objects. The array is initially set to the
       specified size with each entry having NULL as is pointer value.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Other $H$PAbstractArray to compare against.
    ) const;
    /* Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:
        
        $I$EqualTo$I$     if the two array memory blocks are identical in
                          length and each objects values, not pointer, are
                          equal.

        $I$LessThan$I$    if the instances object value at an ordinal position
                          is less than the corresponding objects value in the
                          $B$obj$B$ parameters array.
                          
                          This is also returned if all objects are equal and
                          the instances array length is less than the $B$obj$B$
                          parameters array length.

        $I$GreaterThan$I$ if the instances object value at an ordinal position
                          is greater than the corresponding objects value in
                          the $B$obj$B$ parameters array.
                          
                          This is also returned if all objects are equal and
                          the instances array length is greater than the
                          $B$obj$B$ parameters array length.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
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

       Returns: TRUE if the memory for the array was allocated successfully.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* Append a new object to the collection. This will increase the size of
       the array by one and place the new object at that position.
    
       Returns: index of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );
    /* Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the $B$Append()$B$ function is performed.

       All objects, including the $B$before$B$ object are shifted up one in
       the array.

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

       All objects, including the $B$index$B$ position object are shifted up
       one in the array.

       Returns: index of the newly inserted object.
     */

    virtual BOOL Remove(
      const PObject * obj   // Existing object to remove from the collection.
    );
    /* Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

       Returns: TRUE if the object was in the collection.
     */

    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    );
    /* Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

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
       object pointers are compared, not the values. A simple linear search
       from ordinal position zero is performed.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const;
    /* Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Compare()$B$ function. A
       simple linear search from ordinal position zero is performed.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */


  protected:
    PBASEARRAY(ObjPtrArray, PObject *);
    ObjPtrArray * theArray;
};


#ifdef PHAS_TEMPLATES

template <class T> PDECLARE_CLASS(PArray, PObjectArray)
/* This template class maps the PObjectArray to a specific object type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the $H$PDECLARE_ARRAY macro will
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
    ) const { PAssert(GetAt(index) != NULL, PInvalidArrayElement); return *(T *)GetAt(index); }
    /* Retrieve a reference  to the object in the array. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       Returns: reference to the object at $B$index$B$ position.
     */

    T & operator[](
      PINDEX index  // Index position in the collection of the object.
    ) { return *(T *)(GetAt(index) != NULL ?  GetAt(index) : ((*theArray)[index] = PNEW T)); }
    /* Retrieve a reference  to the object in the array. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function will create a new object.

       Returns: reference to the object at $B$index$B$ position.
     */


  protected:
    PArray(int dummy, const PArray * c) : PArrayObjects(dummy, c) { }
};


/*$MACRO PDECLARE_ARRAY(cls, T)
   This macro is used to declare a descendent of PObjectArray class,
   customised for a particular object type $B$T$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PArray template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PBaseArray and $H$PAbstractArray classes for more information.
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
   This macro is used to declare a descendent of PObjectArray class,
   customised for a particular object type $B$T$B$. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PArray template class.

   See the $H$PBaseArray class and $H$PDECLARE_ARRAY macro for more
   information.
 */
#define PARRAY(cls, T) typedef PArray<T> cls;


#else // PHAS_TEMPLATES


#define PDECLARE_ARRAY(cls, T) \
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
      { PAssert((*theArray)[index] != NULL, PInvalidArrayElement); return *(T *)(*theArray)[index]; } \
    inline T & operator[](PINDEX index) \
      { return *(T *)((*theArray)[index] != NULL ?  (*theArray)[index] \
                                            : ((*theArray)[index] = PNEW T)); } \

#define PARRAY(cls, T) PDECLARE_ARRAY(cls, T) }

#endif // PHAS_TEMPLATES


// End Of File ///////////////////////////////////////////////////////////////

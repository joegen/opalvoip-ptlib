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
 * Revision 1.17  1999/08/22 12:13:42  robertj
 * Fixed warning when using inlines on older GNU compiler
 *
 * Revision 1.16  1999/08/20 03:07:44  robertj
 * Fixed addded Concatenate function for non-template version.
 *
 * Revision 1.15  1999/08/18 01:45:12  robertj
 * Added concatenation function to "base type" arrays.
 *
 * Revision 1.14  1999/03/09 02:59:49  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.13  1999/02/16 08:07:11  robertj
 * MSVC 6.0 compatibility changes.
 *
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

/**This class contains a variable length array of arbitrary memory blocks.
   These can be anything from individual bytes to large structures. Note that
   that does {\bf not} include class objects that require construction or
   destruction. Elements in this array will not execute the contructors or
   destructors of objects.

   An abstract array consists of a linear block of memory sufficient to hold
   #PContainer::GetSize()# elements of #elementSize# bytes
   each. The memory block itself will atuomatically be resized when required
   and freed when no more references to it are present.

   The PAbstractArray class would very rarely be descended from directly by
   the user. The #PBASEARRAY# macro would normally be used to create
   a class and any new classes descended from that. That will instantiate the
   template based on #PBaseArray# or directly declare and define a class
   (using inline functions) if templates are not being used.

   The #PBaseArray# class or #PBASEARRAY# macro will define the correctly
   typed operators for pointer access (#operator const T *#) and subscript
   access (#operator[]#).
 */
class PAbstractArray : public PContainer
{
  PCONTAINERINFO(PAbstractArray, PContainer);

  friend class PArrayObjects;

  public:
  /**@name Construction */
  //@{
    /**Create a new dynamic array of #initalSize# elements of
       #elementSizeInBytes# bytes each. The array memory is
       initialised to zeros.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the #PBASEARRAY# macro.
     */
    PAbstractArray(
      PINDEX elementSizeInBytes,
      /**Size of each element in the array. This must be > 0 or the
         constructor will assert.
       */
      PINDEX initialSize = 0      /// Number of elements to allocate initially.
    );

    /**Create a new dynamic array of #bufferSizeInElements#
       elements of #elementSizeInBytes# bytes each. The contents of
       the memory pointed to by buffer is then used to initialise the newly
       allocated array.

       If the initial size is zero then no memory is allocated. Note that the
       internal pointer is set to NULL, not to a pointer to zero bytes of
       memory. This can be an important distinction when the pointer is
       obtained via an operator created in the #PBASEARRAY# macro.

       If the #dynamicAllocation# parameter is FALSE then the
       pointer is used directly by the container. It will not be copied to a
       dynamically allocated buffer. If the #SetSize()# function is used to
       change the size of the buffer, the object will be converted to a
       dynamic form with the contents of the static buffer copied to the
       allocated buffer.
     */
    PAbstractArray(
      PINDEX elementSizeInBytes,
      /**Size of each element in the array. This must be > 0 or the
         constructor will assert.
       */
      const void *buffer,          /// Pointer to an array of elements.
      PINDEX bufferSizeInElements, /// Number of elements pointed to by buffer.
      BOOL dynamicAllocation       /// Buffer is copied and dynamically allocated.
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:
\begin{description}
          \item[EqualTo]     if the two array memory blocks are identical in
                              length and contents.
          \item[LessThan]    if the array length is less than the
                              #obj# parameters array length.
          \item[GreaterThan] if the array length is greater than the
                              #obj# parameters array length.
\end{description}

        If the array sizes are identical then the #memcmp()#
        function is used to rank the two arrays.

       @return
       comparison of the two objects, #EqualTo# for same,
       #LessThan# for #obj# logically less than the
       object and #GreaterThan# for #obj# logically
       greater than the object.
     */
    virtual Comparison Compare(
      const PObject & obj   /// Other PAbstractArray to compare against.
    ) const;
  //@}

  /**@name Overrides from class PContainer */
  //@{
    /**Set the size of the array in elements. A new array may be allocated to
       accomodate the new number of elements. If the array increases in size
       then the new bytes are initialised to zero. If the array is made smaller
       then the data beyond the new size is lost.

       @return
       TRUE if the memory for the array was allocated successfully.
     */
    virtual BOOL SetSize(
      PINDEX newSize  /// New size of the array in elements.
    );
  //@}

  /**@name New functions for class */
  //@{
    /**Attach a pointer to a static block to the base array type. The pointer
       is used directly and will not be copied to a dynamically allocated
       buffer. If the SetSize() function is used to change the size of the
       buffer, the object will be converted to a dynamic form with the
       contents of the static buffer copied to the allocated buffer.
       
       Any dynamically allocated buffer will be freed.
     */
    void Attach(
      const void *buffer, /// Pointer to an array of elements.
      PINDEX bufferSize   /// Number of elements pointed to by buffer.
    );

    /**Get a pointer to the internal array and assure that it is of at least
       the specified size. This is useful when the array contents are being
       set by some external or system function eg file read.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       @return
       pointer to the array memory.
     */
    void * GetPointer(
      PINDEX minSize = 1  /// Minimum size the array must be.
    );

    /**Concatenate one array to the end of this array.
       This function will allocate a new array large enough for the existing 
       contents and the contents of the parameter. The paramters contents is then
       copied to the end of the existing array.
       
       Note this does nothing and returns FALSE if the target array is not
       dynamically allocated, or if the two arrays are of base elements of
       different sizes.

       @return
       TRUE if the memory allocation succeeded.
     */
    BOOL Concatenate(
      const PAbstractArray & array  /// Array to concatenate.
    );
  //@}

  protected:
    void PrintNumbersOn(ostream & strm, PINDEX size, BOOL is_signed) const;
    virtual long GetNumberValueAt(PINDEX idx) const;

    /// Size of an element in bytes
    PINDEX elementSize;

    /// Pointer to the allocated block of memory.
    char * theArray;

    /// Flag indicating the array was allocated on the heap.
    BOOL allocatedDynamically;
};



///////////////////////////////////////////////////////////////////////////////
// An array of some base type

#ifdef PHAS_TEMPLATES

/**This template class maps the #PAbstractArray# to a specific element type. The
   functions in this class primarily do all the appropriate casting of types.

   Note that if templates are not used the #PBASEARRAY# macro will
   simulate the template instantiation.

   The following classes are instantiated automatically for the basic scalar
   types:
\begin{itemize}
        \item #PCharArray#
        \item #PBYTEArray#
        \item #PShortArray#
        \item #PWORDArray#
        \item #PIntArray#
        \item #PUnsignedArray#
        \item #PLongArray#
        \item #PDWORDArray#
\end{itemize}
 */
template <class T> class PBaseArray : public PAbstractArray
{
  PCLASSINFO(PBaseArray, PAbstractArray);

  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of elements of the specified type. The
       array is initialised to all zero bytes. Note that this may not be
       logically equivalent to the zero value for the type, though this would
       be very rare.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    ) : PAbstractArray(sizeof(T), initialSize) { }
    
    /**Construct a new dynamic array of elements of the specified type.
     */
    PBaseArray(
      T const * buffer,   /// Pointer to an array of the elements of type {\bf T}.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    ) : PAbstractArray(sizeof(T), buffer, length, dynamic) { }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Clone the object.
     */
    virtual PObject * Clone() const
      { return PNEW PBaseArray<T>(*this, GetSize()); }
  //@}

  /**@name Overrides from class PContainer */
  //@{
    /**Set the specific element in the array. The array will automatically
       expand, if necessary, to fit the new element in.

       @return
       TRUE if new memory for the array was successfully allocated.
     */
    BOOL SetAt(
      PINDEX index,   /// Position in the array to set the new value.
      T val           /// Value to set in the array.
    ) { return SetMinSize(index+1) && val==(((T *)theArray)[index] = val); }

    /**Get a value from the array. If the #index# is beyond the end
       of the allocated array then a zero value is returned.

       @return
       value at the array position.
     */
    T GetAt(
      PINDEX index  /// Position on the array to get value from.
    ) const { PASSERTINDEX(index);
                    return index < GetSize() ? ((T *)theArray)[index] : (T)0; }

    /**Attach a pointer to a static block to the base array type. The pointer
       is used directly and will not be copied to a dynamically allocated
       buffer. If the SetSize() function is used to change the size of the
       buffer, the object will be converted to a dynamic form with the
       contents of the static buffer copied to the allocated buffer.
       
       Any dynamically allocated buffer will be freed.
     */
    void Attach(
      const T * buffer,   /// Pointer to an array of elements.
      PINDEX bufferSize   /// Number of elements pointed to by buffer.
    ) { PAbstraceArray::Attach(buffer, bufferSize); }

    /**Get a pointer to the internal array and assure that it is of at least
       the specified size. This is useful when the array contents are being
       set by some external or system function eg file read.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       @return
       pointer to the array memory.
     */
    T * GetPointer(
      PINDEX minSize = 0    /// Minimum size for returned buffer pointer.
    ) { return (T *)PAbstractArray::GetPointer(minSize); }
  //@}

  /**@name New functions for class */
  //@{
    /**Get a value from the array. If the #index# is beyond the end
       of the allocated array then a zero value is returned.

       This is functionally identical to the #PContainer::GetAt()#
       function.

       @return
       value at the array position.
     */
    T operator[](
      PINDEX index  /// Position on the array to get value from.
    ) const { return GetAt(index); }

    /**Get a reference to value from the array. If the #index# is
       beyond the end of the allocated array then the array is expanded. If a
       memory allocation failure occurs the function asserts.

       This is functionally similar to the #SetAt()# function and allows
       the array subscript to be an lvalue.

       @return
       reference to value at the array position.
     */
    T & operator[](
      PINDEX index  /// Position on the array to get value from.
    ) { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory);
        return ((T *)theArray)[index]; }

    /**Get a pointer to the internal array. The user may not modify the
       contents of this pointer/ This is useful when the array contents are
       required by some external or system function eg file write.

       It is unsafe to assume that the pointer is valid for very long after
       return from this function. The array may be resized or otherwise
       changed and the pointer returned invalidated. It should be used for
       simple calls to atomic functions, or very careful examination of the
       program logic must be performed.

       @return
       constant pointer to the array memory.
     */
    operator T const *() const { return (T const *)theArray; }

    /**Concatenate one array to the end of this array.
       This function will allocate a new array large enough for the existing 
       contents and the contents of the parameter. The paramters contents is then
       copied to the end of the existing array.
       
       Note this does nothing and returns FALSE if the target array is not
       dynamically allocated.

       @return
       TRUE if the memory allocation succeeded.
     */
    BOOL Concatenate(
      const PBaseArray & array  /// Other array to concatenate
    ) { return PAbstractArray::Concatenate(array); }
};

/*Declare a dynamic array base type.
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type {\bf T}. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the #PBaseArray# template class.
 */
#define PBASEARRAY(cls, T) typedef PBaseArray<T> cls

/**Begin a declaration of an array of base types.
   This macro is used to declare a descendent of PAbstractArray class,
   customised for a particular element type {\bf T}.

   If the compilation is using templates then this macro produces a descendent
   of the #PBaseArray# template class. If templates are not being used
   then the macro defines a set of inline functions to do all casting of types.
   The resultant classes have an identical set of functions in either case.

   See the #PBaseArray# and #PAbstractArray# classes for more
   information.
 */
#define PDECLARE_BASEARRAY(cls, T) \
  PBASEARRAY(PBaseArray_##cls, T); \
  PDECLARE_CLASS(cls, PBaseArray_##cls) \
    cls(PINDEX initialSize = 0) \
      : PBaseArray_##cls(initialSize) { } \
    cls(T const * buffer, PINDEX length, BOOL dynamic = TRUE) \
      : PBaseArray_##cls(buffer, length, dynamic) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(*this, GetSize()); } \


#else // PHAS_TEMPLATES

#define PBASEARRAY(cls, T) \
  typedef T P_##cls##_Base_Type; \
  class cls : public PAbstractArray { \
    PCLASSINFO(cls, PAbstractArray) \
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
    inline BOOL Concatenate(cls const & array) \
      { return PAbstractArray::Concatenate(array); } \
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


/// Array of characters.
#ifdef DOC_PLUS_PLUS
class PCharArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of char.
       The array is initialised to all zero bytes.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of char.
     */
    PBaseArray(
      char const * buffer,   /// Pointer to an array of chars.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PCharArray, char);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}
};


/// Array of short integers.
#ifdef DOC_PLUS_PLUS
class PShortArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of shorts.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of shorts.
     */
    PBaseArray(
      short const * buffer,   /// Pointer to an array of shorts.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PShortArray, short);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of integers.
#ifdef DOC_PLUS_PLUS
class PIntArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of ints.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of ints.
     */
    PBaseArray(
      int const * buffer,   /// Pointer to an array of ints.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PIntArray, int);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of long integers.
#ifdef DOC_PLUS_PLUS
class PLongArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of longs.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of longs.
     */
    PBaseArray(
      long const * buffer,   /// Pointer to an array of longs.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PLongArray, long);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of unsigned characters.
#ifdef DOC_PLUS_PLUS
class PBYTEArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of unsigned chars.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of unsigned chars.
     */
    PBaseArray(
      BYTE const * buffer,   /// Pointer to an array of BYTEs.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PBYTEArray, BYTE);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of unsigned short integers.
#ifdef DOC_PLUS_PLUS
class PWORDArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of unsigned shorts.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of unsigned shorts.
     */
    PBaseArray(
      WORD const * buffer,   /// Pointer to an array of WORDs.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PWORDArray, WORD);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of unsigned integers.
#ifdef DOC_PLUS_PLUS
class PUnsignedArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of unsigned ints.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of unsigned ints.
     */
    PBaseArray(
      unsigned const * buffer,   /// Pointer to an array of unsigned ints.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PUnsignedArray, unsigned);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


/// Array of unsigned long integers.
#ifdef DOC_PLUS_PLUS
class PDWORDArray : public PBaseArray {
  public:
  /**@name Construction */
  //@{
    /**Construct a new dynamic array of unsigned longs.
       The array is initialised to all zeros.
     */
    PBaseArray(
      PINDEX initialSize = 0  /// Initial number of elements in the array.
    );

    /**Construct a new dynamic array of DWORDs.
     */
    PBaseArray(
      DWORD const * buffer,   /// Pointer to an array of DWORDs.
      PINDEX length,      /// Number of elements pointed to by #buffer#.
      BOOL dynamic = TRUE /// Buffer is copied and dynamically allocated.
    );
  //@}
#endif
PDECLARE_BASEARRAY(PDWORDArray, DWORD);
  public:
  /**@name Overrides from class PObject */
  //@{
    /// Print the array
    virtual void PrintOn(
      ostream & strm /// Stream to output to.
    ) const;
  //@}

    virtual long GetNumberValueAt(PINDEX idx) const;
};


///////////////////////////////////////////////////////////////////////////////
// Linear array of objects

/** An array of objects.
This class is a collection of objects which are descendents of the
#PObject# class. It is implemeted as a dynamic, linear array of
pointers to the objects.

The implementation of an array allows very fast random access to items in
the collection, but has severe penalties for inserting and deleting objects
as all other objects must be moved to accommodate the change.

An array of objects may have "gaps" in it. These are array entries that
contain NULL as the object pointer.

The PArrayObjects class would very rarely be descended from directly by
the user. The #PARRAY# macro would normally be used to create a class.
That will instantiate the template based on #PArray# or directly declare
and define the class (using inline functions) if templates are not being used.

The #PArray# class or #PARRAY# macro will define the
correctly typed operators for pointer access (#operator const T *#) and
subscript access (#operator[]#).
*/
class PArrayObjects : public PCollection
{
  PCONTAINERINFO(PArrayObjects, PCollection);

  public:
  /**@name Construction */
  //@{
    /**Create a new array of objects. The array is initially set to the
       specified size with each entry having NULL as is pointer value.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PINLINE PArrayObjects(
      PINDEX initialSize = 0  /// Initial number of objects in the array.
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Get the relative rank of the two arrays. The following algorithm is
       employed for the comparison:
\begin{description}
        \item[EqualTo]      if the two array memory blocks are identical in
                            length and each objects values, not pointer, are
                            equal.

        \item[LessThan]     if the instances object value at an ordinal
                            position is less than the corresponding objects
                            value in the #obj# parameters array.
                          
                            This is also returned if all objects are equal and
                            the instances array length is less than the
                            #obj# parameters array length.

        \item[GreaterThan]  if the instances object value at an ordinal
                            position is greater than the corresponding objects
                            value in the #obj# parameters array.
                          
                            This is also returned if all objects are equal and
                            the instances array length is greater than the
                            #obj# parameters array length.
\end{description}

       @return
       comparison of the two objects, #EqualTo# for same,
       #LessThan# for #obj# logically less than the
       object and #GreaterThan# for #obj# logically
       greater than the object.
     */
    virtual Comparison Compare(
      const PObject & obj   /// Other #PAbstractArray# to compare against.
    ) const;
  //@}

  /**@name Overrides from class PContainer */
  //@{
    /// Get size of array
    virtual PINDEX GetSize() const;

    /**Set the size of the array in objects. A new array may be allocated to
       accomodate the new number of objects. If the array increases in size
       then the new object pointers are initialised to NULL. If the array is
       made smaller then the data beyond the new size is lost.

       @return
       TRUE if the memory for the array was allocated successfully.
     */
    virtual BOOL SetSize(
      PINDEX newSize  /// New size of the array in objects.
    );
  //@}

  /**@name Overrides from class PCollection */
  //@{
    /**Append a new object to the collection. This will increase the size of
       the array by one and place the new object at that position.
    
       @return
       index of the newly added object.
     */
    virtual PINDEX Append(
      PObject * obj   /// New object to place into the collection.
    );

    /**Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the #Append()# function is performed.

       All objects, including the #before# object are shifted up
       one in the array.

       Note that the object values are compared for the search of the
       #before# parameter, not the pointers. So the objects in the
       collection must correctly implement the #PObject::Compare()#
       function.

       @return
       index of the newly inserted object.
     */
    virtual PINDEX Insert(
      const PObject & before,   /// Object value to insert before.
      PObject * obj             /// New object to place into the collection.
    );

    /** Insert a new object at the specified ordinal index. If the index is
       greater than the number of objects in the collection then the
       equivalent of the #Append()# function is performed.

       All objects, including the #index# position object are
       shifted up one in the array.

       @return
       index of the newly inserted object.
     */
    virtual PINDEX InsertAt(
      PINDEX index,   /// Index position in collection to place the object.
      PObject * obj   /// New object to place into the collection.
    );

    /**Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

       @return
       TRUE if the object was in the collection.
     */
    virtual BOOL Remove(
      const PObject * obj   /// Existing object to remove from the collection.
    );

    /**Remove the object at the specified ordinal index from the collection.
       If the AllowDeleteObjects option is set then the object is also deleted.

       All objects are shifted down to fill the vacated position.

       Note if the index is beyond the size of the collection then the
       function will assert.

       @return
       pointer to the object being removed, or NULL if it was deleted.
     */
    virtual PObject * RemoveAt(
      PINDEX index   /// Index position in collection to place the object.
    );

    /**Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.

       @return
       TRUE if the object was successfully added.
     */
    virtual BOOL SetAt(
      PINDEX index,   /// Index position in collection to set.
      PObject * val   /// New value to place into the collection.
    );

    /**Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       @return
       pointer to object at the specified index.
     */
    virtual PObject * GetAt(
      PINDEX index  /// Index position in the collection of the object.
    ) const;

    /**Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. A simple linear search
       from ordinal position zero is performed.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetObjectsIndex(
      const PObject * obj  /// Object to find.
    ) const;

    /**Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the #PObject::Compare()#
       function. A simple linear search from ordinal position zero is
       performed.

       @return
       ordinal index position of the object, or P_MAX_INDEX.
     */
    virtual PINDEX GetValuesIndex(
      const PObject & obj   // Object to find equal of.
    ) const;

    /**Remove all of the elements in the collection. This operates by
       continually calling #RemoveAt()# until there are no objects left.

       The objects are removed from the last, at index
       #(GetSize()-1)# toward the first at index zero.
     */
    virtual void RemoveAll();
  //@}

  protected:
    PBASEARRAY(ObjPtrArray, PObject *);
    ObjPtrArray * theArray;
};


#ifdef PHAS_TEMPLATES

/**This template class maps the PArrayObjects to a specific object type.
The functions in this class primarily do all the appropriate casting of types.

Note that if templates are not used the #PARRAY# macro will
simulate the template instantiation.
*/
template <class T> class PArray : public PArrayObjects
{
  PCLASSINFO(PArray, PArrayObjects);

  public:
  /**@name Construction */
  //@{
    /**Create a new array of objects. The array is initially set to the
       specified size with each entry having NULL as is pointer value.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */
    PArray( 
      PINDEX initialSize = 0  /// Initial number of objects in the array.
    ) : PArrayObjects(initialSize) { }
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Make a complete duplicate of the array. Note that all objects in the
       array are also cloned, so this will make a complete copy of the array.
     */
    virtual PObject * Clone() const
      { return PNEW PArray(0, this); }
  //@}

  /**@name New functions for class */
  //@{
    /**Retrieve a reference  to the object in the array. If there was not an
       object at that ordinal position or the index was beyond the size of the
       array then the function asserts.

       @return
       reference to the object at #index# position.
     */
    T & operator[](
      PINDEX index  /// Index position in the collection of the object.
    ) const { PAssert(GetAt(index) != NULL, PInvalidArrayElement);
                                                   return *(T *)GetAt(index); }
  //@}

  protected:
    PArray(int dummy, const PArray * c) : PArrayObjects(dummy, c) { }
};


/** Declare an array to a specific type of object.
This macro is used to declare a descendent of PArrayObjects class,
customised for a particular object type {\bf T}. This macro closes the
class declaration off so no additional members can be added.

If the compilation is using templates then this macro produces a typedef
of the #PArray# template class.

See the #PBaseArray# class and #PDECLARE_ARRAY# macro for more
information.
*/
#define PARRAY(cls, T) typedef PArray<T> cls


/** Begin declaration an array to a specific type of object.
This macro is used to declare a descendent of PArrayObjects class,
customised for a particular object type {\bf T}.

If the compilation is using templates then this macro produces a descendent
of the #PArray# template class. If templates are not being used then
the macro defines a set of inline functions to do all casting of types. The
resultant classes have an identical set of functions in either case.

See the #PBaseArray# and #PAbstractArray# classes for more
information.
*/
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

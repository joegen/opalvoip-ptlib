/*
 * $Id: dict.h,v 1.7 1995/02/22 10:50:29 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: dict.h,v $
 * Revision 1.7  1995/02/22 10:50:29  robertj
 * Changes required for compiling release (optimised) version.
 *
 * Revision 1.6  1995/02/11  04:10:35  robertj
 * Fixed dictionary MACRO for templates.
 *
 * Revision 1.5  1995/02/05  00:48:03  robertj
 * Fixed template version.
 *
 * Revision 1.4  1995/01/09  12:35:31  robertj
 * Removed unnecesary return value from I/O functions.
 * Changes due to Mac port.
 *
 * Revision 1.3  1994/12/21  11:52:51  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.2  1994/12/17  01:36:57  robertj
 * Fixed memory leak in PStringSet
 *
 * Revision 1.1  1994/12/12  09:59:32  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PDictionary classes

PDECLARE_CLASS(POrdinalKey, PObject)
/* This class is used when an ordinal index value is the key for $H$PSet and
   $H$PDictionary classes.
 */

  public:
    POrdinalKey(
      PINDEX newKey   // Ordinal index value to use as a key.
    );
    // Create a new key for ordinal index values.

  // Overrides from class PObject
    virtual PObject * Clone() const;
    // Create a duplicate of the POrdinalKey.

    virtual Comparison Compare(const PObject & obj) const;
    /* Get the relative rank of the ordinal index. This is a simpel comparison
       of the objects PINDEX values.

       Returns: comparison of the two objects, $B$EqualTo$B$ for same,
                $B$LessThan$B$ for $B$obj$B$ logically less than the object
                and $B$GreaterThan$B$ for $B$obj$B$ logically greater than
                the object.
     */

    virtual PINDEX HashFunction() const;
    /* This function calculates a hash table index value for the implementation
       of $H$PSet and $H$PDictionary classes.

       Returns: hash table bucket number.
     */

    virtual void PrintOn(ostream & strm) const;
    /* Output the ordinal index to the specified stream. This is identical to
       outputting the PINDEX, ie integer, value.

       Returns: stream that the index was output to.
     */

    operator PINDEX() const;
    // Operator so that a POrdinalKey can be used as a PINDEX value.

  private:
    PINDEX theKey;
};


//////////////////////////////////////////////////////////////////////////////

PDECLARE_CONTAINER(PHashTable, PCollection)
/* The hash table class is the basis for implementing the $H$PSet and
   $H$PDictionary classes.

   The hash table allows for very fast searches for an object based on a "hash
   function". This function yields an index into an array which is directly
   looked up to locate the object. When two key values have the same hash
   function value, then a linear search of a linked list is made to locate
   the object. Thus the efficiency of the hash table is highly dependent on the
   quality of the hash function for the data being used as keys.
 */

  public:
    PHashTable();
    // Create a new, empty, hash table.


  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Other PHashTable to compare against.
    ) const;
    /* Get the relative rank of the two hash tables. Actally ranking hash
       tables is really meaningless, so only equality is returned by the
       comparison. Equality is only achieved if the two instances reference the
       same hash table.

       Returns: comparison of the two objects, $B$EqualTo$B$ if the same
                reference and $B$GreaterThan$B$ if not.
     */


  protected:
  // Overrides from class PContainer
    virtual BOOL SetSize(
      PINDEX newSize  // New size for the hash table, this is ignored.
    );
    /* This function is meaningless for hash table. The size of the collection
       is determined by the addition and removal of objects. The size cannot be
       set in any other way.

       Returns: TRUE.
     */


    // New functions for class
    virtual const PObject & AbstractGetKeyAt(
      PINDEX index  // Ordinal position in the hash table.
    ) const;
    /* Get the key in the hash table at the ordinal index position.
    
       The ordinal position in the hash table is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       This function is primarily used by the descendent template classes, or
       macro, with the appropriate type conversion.

       Returns: reference to key at the index position.
     */

    virtual PObject & AbstractGetDataAt(
      PINDEX index  // Ordinal position in the hash table.
    ) const;
    /* Get the data in the hash table at the ordinal index position.
    
       The ordinal position in the hash table is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       This function is primarily used by the descendent template classes, or
       macro, with the appropriate type conversion.

       Returns: reference to key at the index position.
     */


    // Member variables
    PCLASS Element {
      public:
        PObject * key;
        PObject * data;
        Element * next;
        Element * prev;
    };

    PDECLARE_BASEARRAY(Table, Element *)
      public:
        virtual ~Table() { Destruct(); }
        virtual void DestroyContents();

        PINDEX AppendElement(PObject * key, PObject * data);
        PObject * RemoveElement(const PObject & key);
        BOOL SetLastElementAt(PINDEX index);
        Element * GetElementAt(const PObject & key);
        PINDEX GetElementsIndex(const PObject*obj,BOOL byVal,BOOL keys) const;

        PINDEX    lastIndex;
        PINDEX    lastBucket;
        Element * lastElement;

        BOOL deleteKeys;

      friend class PHashTable;
      friend class PAbstractSet;
    };
    friend class Table;


    Table * hashTable;
};


//////////////////////////////////////////////////////////////////////////////

PDECLARE_CONTAINER(PAbstractSet, PHashTable)
  public:
    PAbstractSet();
    /* Create a new, empty, set.

       Note that by default, objects placed into the list will be deleted when
       removed or when all references to the list are destroyed.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* Add a new object to the collection. If the objects value is already in
       the set then the object is $U$not$U$ included. If the AllowDeleteObjects
       option is set then the $B$obj$B$ parameter is also deleted.

       Returns: hash function value of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    );
    /* Add a new object to the collection. If the objects value is already in
       the set then the object is $U$not$U$ included. If the AllowDeleteObjects
       option is set then the $B$obj$B$ parameter is also deleted.
       
       The object is always placed in the an ordinal position dependent on its
       hash function. It is not placed at the specified position. The
       $B$before$B$ parameter is ignored.

       Returns: hash function value of the newly added object.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );
    /* Add a new object to the collection. If the objects value is already in
       the set then the object is $U$not$U$ included. If the AllowDeleteObjects
       option is set then the $B$obj$B$ parameter is also deleted.
       
       The object is always placed in the an ordinal position dependent on its
       hash function. It is not placed at the specified position. The
       $B$index$B$ parameter is ignored.

       Returns: hash function value of the newly added object.
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
    /* This function is meaningless and will assert if executed.

       Returns: NULL.
     */

    virtual BOOL SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );
    /* Add a new object to the collection. If the objects value is already in
       the set then the object is $U$not$U$ included. If the AllowDeleteObjects
       option is set then the $B$obj$B$ parameter is also deleted.
       
       The object is always placed in the an ordinal position dependent on its
       hash function. It is not placed at the specified position. The
       $B$index$B$ parameter is ignored.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;
    /* This function is meaningless.

       Returns: NULL.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. The hash table is used
       to locate the entry.

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
       collection must correctly implement the $B$Compare()$B$ function. The
       hash table is used to locate the entry.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */


  // New functions for class
    BOOL Contains(
      const PObject & key   // Key to look for in the set.
    );
    /* Determine if the value of the object is contained in the set. The
       object values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Compare()$B$ function. The
       hash table is used to locate the entry.

       Returns: TRUE if the object value is in the set.
     */
};


#ifdef PHAS_TEMPLATES

template <class T>
PDECLARE_CLASS(PSet, PAbstractSet)
/* This template class maps the PAbstractSet to a specific object type. The
   functions in this class primarily do all the appropriate casting of types.

   By default, objects placed into the set will $U$not$U$ be deleted when
   removed or when all references to the set are destroyed. This is different
   from the default on most collection classes.

   Note that if templates are not used the $H$PDECLARE_SET macro will simulate
   the template instantiation.
 */

  public:
    inline PSet(BOOL initialDeleteObjects = FALSE)
      : PAbstractSet() { AllowDeleteObjects(initialDeleteObjects); }
    /* Create a new, empty, dictionary. The parameter indicates whether to
       delete objects that are removed from the set.

       Note that by default, objects placed into the set will $U$not$U$ be
       deleted when removed or when all references to the set are destroyed.
       This is different from the default on most collection classes.
     */

    virtual PObject * Clone() const
      { return PNEW PSet(0, this); }
    /* Make a complete duplicate of the set. Note that all objects in the
       array are also cloned, so this will make a complete copy of the set.
     */

    void Include(
      const T * obj   // New object to include in the set.
    ) { Append((PObject *)obj); }
    /* Include the spcified object into the set. If the objects value is
       already in the set then the object is $U$not$U$ included. If the
       AllowDeleteObjects option is set then the $B$obj$B$ parameter is also
       deleted.

       The object values are compared, not the pointers.  So the objects in
       the collection must correctly implement the $B$Compare()$B$ function.
       The hash table is used to locate the entry.
     */

    void Exclude(
      const T * obj   // New object to exclude in the set.
    ) { Remove(obj); }
    /* Remove the object from the set. If the AllowDeleteObjects option is set
       then the object is also deleted.

       The object values are compared, not the pointers.  So the objects in
       the collection must correctly implement the $B$Compare()$B$ function.
       The hash table is used to locate the entry.
     */

    BOOL operator[](
      const T & key  // Key to look for in the set.
    ) { return Contains(key); }
    /* Determine if the value of the object is contained in the set. The
       object values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Compare()$B$ function. The
       hash table is used to locate the entry.

       Returns: TRUE if the object value is in the set.
     */

    virtual const T & GetKeyAt(PINDEX index) const
      { return (const T &)AbstractGetKeyAt(index); }
    /* Get the key in the set at the ordinal index position.
    
       The ordinal position in the set is determined by the hash values of the
       keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to key at the index position.
     */

  protected:
    PSet(int dummy, const PSet * c)
      : PAbstractSet(dummy, c)
      { reference->deleteObjects = c->reference->deleteObjects; }
};


/*$MACRO PDECLARE_SET(cls, T, initDelObj)
   This macro is used to declare a descendent of PAbstractSet class,
   customised for a particular object type $B$T$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PSet template class. If templates are not being used then the
   macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PSet and $H$PAbstractSet classes for more information.
 */
#define PDECLARE_SET(cls, T, initDelObj) \
  PDECLARE_CLASS(cls, PSet<T>) \
  protected: \
    cls(int dummy, const cls * c) \
      : PSet<T>(dummy, c) { } \
  public: \
    cls(BOOL initialDeleteObjects = initDelObj) \
      : PSet<T>(initialDeleteObjects) { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PSET(cls, T)
   This macro is used to declare a descendent of PAbstractSet class,
   customised for a particular object type $B$T$B$. This macro closes the
   class declaration off so no additional members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PSet template class.

   See the $H$PSet class and $H$PDECLARE_SET macro for more information.
 */
#define PSET(cls, T) typedef PSet<T> cls


#else // PHAS_TEMPLATES


#define PDECLARE_SET(cls, K, initDelObj) \
  PDECLARE_CLASS(cls, PAbstractSet) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractSet(dummy, c) \
      { reference->deleteObjects = c->reference->deleteObjects; } \
  public: \
    inline cls(BOOL initialDeleteObjects = initDelObj) \
      : PAbstractSet() { AllowDeleteObjects(initialDeleteObjects); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline void Include(const PObject * key) \
      { Append((PObject *)key); } \
    inline void Exclude(const PObject * key) \
      { Remove(key); } \
    inline BOOL operator[](const K & key) \
        { return Contains(key); } \
    virtual const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \

#define PSET(cls, K) PDECLARE_SET(cls, K, FALSE) }


#endif  // PHAS_TEMPLATES


//////////////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PAbstractDictionary, PHashTable)
  public:
    PAbstractDictionary();
    /* Create a new, empty, dictionary.

       Note that by default, objects placed into the dictionary will be deleted
       when removed or when all references to the dictionary are destroyed.
     */

  // Overrides from class PCollection
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    );
    /* This function is meaningless and will assert.

       Returns: 0.
     */

    virtual PINDEX Insert(
      const PObject & key,   // Object value to use as the key.
      PObject * obj          // New object to place into the collection.
    );
    /* Insert a new object into the dictionary. The semantics of this function
       is different from that of the $H$PCollection class. This function is
       exactly equivalent to the SetAt() function that sets a data value at
       the key value location.

       Returns: 0.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    );
    /* Insert a new object at the specified index. The index is converted to
       a $H$POrdinalKey type before being used in the $B$SetAt()$B$ function.

       Returns: $B$index$B$ parameter.
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
    /* Remove an object at the specified index. The index is converted to
       a $H$POrdinalKey type before being used in the GetAt() function. The
       returned pointer is then removed using the SetAt() function to set that
       key value to NULL. If the AllowDeleteObjects option is set then the
       object is also deleted.

       Returns: pointer to the object being removed, or NULL if it was deleted.
     */

    virtual BOOL SetAt(
      PINDEX index,   // Index position in collection to set.
      PObject * val   // New value to place into the collection.
    );
    /* Set the object at the specified index to the new value. The index is
       converted to a $H$POrdinalKey type before being used.This will overwrite
       the existing entry. If the AllowDeleteObjects option is set then the
       old object is also deleted.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const;
    /* Get the object at the specified index position. The index is converted
       to a $H$POrdinalKey type before being used. If the index was not in the
       collection then NULL is returned.

       Returns: pointer to object at the specified index.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. The hash table is used
       to locate the entry.

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
       collection must correctly implement the $B$Compare()$B$ function. The
       hash table is used to locate the entry.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */


  // New functions for class
    virtual BOOL SetDataAt(
      PINDEX index,   // Ordinal index in the dictionary.
      PObject * obj   // New object to put into the dictionary.
    );
    /* Set the data at the specified ordinal index position in the dictionary.

       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       Returns: TRUE if the new object could be placed into the dictionary.
     */

    virtual BOOL SetAt(
      const PObject & key,  // Key for position in dictionary to add object.
      PObject * obj         // New object to put into the dictionary.
    );
    /* Add a new object to the collection. If the objects value is already in
       the dictionary then the object is overrides the previous value. If the
       AllowDeleteObjects option is set then the old object is also deleted.

       The object is placed in the an ordinal position dependent on the keys
       hash function. Subsequent searches use the has function to speed access
       to the data item.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject & GetRefAt(
      const PObject & key   // Key for position in dictionary to get object.
    ) const;
    /* Get the object at the specified key position. If the key was not in the
       collection then this function asserts.

       This function is primarily for use by the oeprator[] function is
       descendent template classes.

       Returns: reference to object at the specified key.
     */

    virtual PObject * GetAt(
      const PObject & key   // Key for position in dictionary to get object.
    ) const;
    /* Get the object at the specified key position. If the key was not in the
       collection then NULL is returned.

       Returns: pointer to object at the specified key.
     */

  protected:
    PAbstractDictionary(int dummy, const PAbstractDictionary * c);
};


#ifdef PHAS_TEMPLATES

template <class K, class D>
PDECLARE_CLASS(PDictionary, PAbstractDictionary)
/* This template class maps the PAbstractDictionary to a specific key and data
   types. The functions in this class primarily do all the appropriate casting
   of types.

   Note that if templates are not used the $H$PDECLARE_DICTIONARY macro will
   simulate the template instantiation.
 */

  public:
    PDictionary()
      : PAbstractDictionary() { }
    /* Create a new, empty, dictionary.

       Note that by default, objects placed into the dictionary will be
       deleted when removed or when all references to the dictionary are
       destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW PDictionary(0, this); }
    /* Make a complete duplicate of the dictionary. Note that all objects in
       the array are also cloned, so this will make a complete copy of the
       dictionary.
     */

    D & operator[](const K & key) const
      { return (D &)GetRefAt(key); }
    /* Get the object contained in the dictionary at the $B$key$B$ position.
       The hash table is used to locate the data quickly via the hash function
       privided by the key.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to the object indexed by the key.
     */

    virtual D * GetAt(
      const K & key   // Key for position in dictionary to get object.
    ) const { return (D *)PAbstractDictionary::GetAt(key); }
    /* Get the object at the specified key position. If the key was not in the
       collection then NULL is returned.

       Returns: pointer to object at the specified key.
     */

    const K & GetKeyAt(PINDEX index) const
      { return (const K &)AbstractGetKeyAt(index); }
    /* Get the key in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to key at the index position.
     */

    D & GetDataAt(PINDEX index) const
      { return (D &)AbstractGetDataAt(index); }
    /* Get the data in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to data at the index position.
     */


  protected:
    PDictionary(int dummy, const PDictionary * c)
      : PAbstractDictionary(dummy, c) { }
};


/*$MACRO PDECLARE_DICTIONARY(cls, K, D)
   This macro is used to declare a descendent of PAbstractDictionary class,
   customised for a particular key type $B$K$B$ and data object type $B$D$B$.

   If the compilation is using templates then this macro produces a descendent
   of the $H$PDictionary template class. If templates are not being used then
   the macro defines a set of inline functions to do all casting of types. The
   resultant classes have an identical set of functions in either case.

   See the $H$PDictionary and $H$PAbstractDictionary classes for more
   information.
 */
#define PDECLARE_DICTIONARY_TEMPLATE_CLASS_INSTANCE(K,D) PDictionary<K,D>
#define PDECLARE_DICTIONARY(cls, K, D) \
  PDECLARE_CLASS(cls, PDECLARE_DICTIONARY_TEMPLATE_CLASS_INSTANCE(K, D)) \
  protected: \
    cls(int dummy, const cls * c) \
      : PDictionary<K, D>(dummy, c) { } \
  public: \
    cls() \
      : PDictionary<K, D>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PDICTIONARY(cls, K, D)
   This macro is used to declare a descendent of PAbstractDictionary class,
   customised for a particular key type $B$K$B$ and data object type $B$D$B$.
   This macro closes the class declaration off so no additional members can
   be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$PDictionary template class.

   See the $H$PDictionary class and $H$PDECLARE_DICTIONARY macro for more
   information.
 */
#define PDICTIONARY(cls, K, D) typedef PDictionary<K, D> cls


template <class K>
PDECLARE_CLASS(POrdinalDictionary, PAbstractDictionary)
/* This template class maps the $H$PAbstractDictionary to a specific key type
   and a $H$POrdinalKey data type. The functions in this class primarily do
   all the appropriate casting of types.

   Note that if templates are not used the $H$PDECLARE_ORDINAL_DICTIONARY
   macro will simulate the template instantiation.
 */

  public:
    POrdinalDictionary()
      : PAbstractDictionary() { }
    /* Create a new, empty, dictionary.

       Note that by default, objects placed into the dictionary will be
       deleted when removed or when all references to the dictionary are
       destroyed.
     */

    virtual PObject * Clone() const
      { return PNEW POrdinalDictionary(0, this); }
    /* Make a complete duplicate of the dictionary. Note that all objects in
       the array are also cloned, so this will make a complete copy of the
       dictionary.
     */

    PINDEX operator[](const K & key) const
      { return (POrdinalKey &)GetRefAt(key); }
    /* Get the object contained in the dictionary at the $B$key$B$ position.
       The hash table is used to locate the data quickly via the hash function
       privided by the key.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to the object indexed by the key.
     */

    virtual POrdinalKey * GetAt(
      const K & key   // Key for position in dictionary to get object.
    ) const { return (POrdinalKey *)PAbstractDictionary::GetAt(key); }
    /* Get the object at the specified key position. If the key was not in the
       collection then NULL is returned.

       Returns: pointer to object at the specified key.
     */

    virtual BOOL SetDataAt(
      PINDEX index,   // Ordinal index in the dictionary.
      PINDEX ordinal  // New ordinal value to put into the dictionary.
    ) { return PAbstractDictionary::SetDataAt(index, PNEW POrdinalKey(ordinal)); }
    /* Set the data at the specified ordinal index position in the dictionary.

       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       Returns: TRUE if the new object could be placed into the dictionary.
     */

    virtual BOOL SetAt(
      const K & key,  // Key for position in dictionary to add object.
      PINDEX ordinal  // New ordinal value to put into the dictionary.
    ) { return PAbstractDictionary::SetAt(key, PNEW POrdinalKey(ordinal)); }
    /* Add a new object to the collection. If the objects value is already in
       the dictionary then the object is overrides the previous value. If the
       AllowDeleteObjects option is set then the old object is also deleted.

       The object is placed in the an ordinal position dependent on the keys
       hash function. Subsequent searches use the has function to speed access
       to the data item.

       Returns: TRUE if the object was successfully added.
     */

    const K & GetKeyAt(PINDEX index) const
      { return (const K &)AbstractGetKeyAt(index); }
    /* Get the key in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to key at the index position.
     */

    PINDEX GetDataAt(PINDEX index) const
      { return (POrdinalKey &)AbstractGetDataAt(index); }
    /* Get the data in the dictionary at the ordinal index position.
    
       The ordinal position in the dictionary is determined by the hash values
       of the keys and the order of insertion.

       The last key/data pair is remembered by the class so that subseqent
       access is very fast.

       Returns: reference to data at the index position.
     */


  protected:
    POrdinalDictionary(int dummy, const POrdinalDictionary * c)
      : PAbstractDictionary(dummy, c) { }
};


/*$MACRO PDECLARE_ORDINAL_DICTIONARY(cls, K)
   This macro is used to declare a descendent of PAbstractList class,
   customised for a particular key type $B$K$B$ and data object type of
   $H$POrdinalKey.

   If the compilation is using templates then this macro produces a descendent
   of the $H$POrdinalDictionary template class. If templates are not being used
   then the macro defines a set of inline functions to do all casting of types.
   The resultant classes have an identical set of functions in either case.

   See the $H$POrdinalDictionary and $H$PAbstractDictionary classes for more
   information.
 */
#define PDECLARE_ORDINAL_DICTIONARY(cls, K) \
  PDECLARE_CLASS(cls, POrdinalDictionary<K>) \
  protected: \
    cls(int dummy, const cls * c) \
      : POrdinalDictionary<K>(dummy, c) { } \
  public: \
    cls() \
      : PDictionary<K>() { } \
    virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \


/*$MACRO PORDINAL_DICTIONARY(cls, K)
   This macro is used to declare a descendent of PAbstractDictionary class,
   customised for a particular key type $B$K$B$ and data object type of
   $H$POrdinalKey. This macro closes the class declaration off so no additional
   members can be added.

   If the compilation is using templates then this macro produces a typedef
   of the $H$POrdinalDictionary template class.

   See the $H$POrdinalDictionary class and $H$PDECLARE_ORDINAL_DICTIONARY macro
   for more information.
 */
#define PORDINAL_DICTIONARY(cls, K) typedef POrdinalDictionary<K> cls


#else // PHAS_TEMPLATES


#define PDECLARE_DICTIONARY(cls, K, D) \
  PDECLARE_CLASS(cls, PAbstractDictionary) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractDictionary(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractDictionary() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline D & operator[](const K & key) const \
      { return (D &)GetRefAt(key); } \
    inline virtual D * GetAt(const K & key) const \
      { return (D *)PAbstractDictionary::GetAt(key); } \
    inline const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \
    inline D & GetDataAt(PINDEX index) const \
      { return (D &)AbstractGetDataAt(index); } \

#define PDICTIONARY(cls, K, D) PDECLARE_DICTIONARY(cls, K, D) }

#define PDECLARE_ORDINAL_DICTIONARY(cls, K) \
  PDECLARE_CLASS(cls, PAbstractDictionary) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractDictionary(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractDictionary() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline PINDEX operator[](const K & key) const \
      { return (POrdinalKey &)GetRefAt(key); } \
    inline virtual POrdinalKey * GetAt(const K & key) const \
      { return (POrdinalKey *)PAbstractDictionary::GetAt(key); } \
    inline virtual BOOL SetDataAt(PINDEX index, PINDEX ordinal) \
     {return PAbstractDictionary::SetDataAt(index,PNEW POrdinalKey(ordinal));}\
    inline virtual BOOL SetAt(PINDEX idx, PObject * obj) \
      { return PAbstractDictionary::SetAt(idx, obj); } \
    inline virtual BOOL SetAt(const PObject & key, PObject * obj) \
      { return PAbstractDictionary::SetAt(key, obj); } \
    inline virtual BOOL SetAt(const K & key, PINDEX ordinal) \
      { return PAbstractDictionary::SetAt(key, PNEW POrdinalKey(ordinal)); } \
    inline const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \
    inline PINDEX GetDataAt(PINDEX index) const \
      { return (POrdinalKey &)AbstractGetDataAt(index); } \

#define PORDINAL_DICTIONARY(cls, K) PDECLARE_ORDINAL_DICTIONARY(cls, K) }


#endif // PHAS_TEMPLATES



// End Of File ///////////////////////////////////////////////////////////////

/*
 * $Id: contain.h,v 1.40 1994/12/13 11:50:45 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: contain.h,v $
 * Revision 1.40  1994/12/13 11:50:45  robertj
 * Added MakeUnique() function to all container classes.
 *
 * Revision 1.39  1994/12/12  10:16:18  robertj
 * Restructuring and documentation of container classes.
 * Renaming of some macros for declaring container classes.
 * Added some extra functionality to PString.
 * Added start to 2 byte characters in PString.
 * Fixed incorrect overrides in PCaselessString.
 *
 * Revision 1.38  1994/12/05  11:18:58  robertj
 * Moved SetMinSize from PAbstractArray to PContainer.
 *
 * Revision 1.37  1994/11/28  12:33:44  robertj
 * Added dummy parameter for cls* constructor in containers. This prevents some very
 * strange an undesirable default construction of clones.
 *
 * Revision 1.36  1994/10/30  11:50:09  robertj
 * Split into Object classes and Container classes.
 * Changed mechanism for doing notification callback functions.
 *
 * Revision 1.35  1994/10/23  04:40:50  robertj
 * Made container * constractor protected.
 * Shorted OS Error assert.
 * Added printf constructor to PString.
 *
 * Revision 1.34  1994/09/25  10:36:41  robertj
 * Improved const behavious of container class macros.
 *
 * Revision 1.33  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.32  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.31  1994/08/21  23:43:02  robertj
 * Changed parameter before variable argument list to NOT be a reference.
 * Added object serialisation classes.
 *
 * Revision 1.30  1994/08/04  11:51:39  robertj
 * Rewrite of memory check functions.
 *
 * Revision 1.29  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.28  1994/07/25  03:33:50  robertj
 * Extra memory tests.
 *
 * Revision 1.27  1994/07/17  10:46:06  robertj
 * Added functions to strings in containers.
 *
 * Revision 1.26  1994/07/02  03:03:49  robertj
 * Addition of container searching facilities.
 *
 * Revision 1.25  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.24  1994/04/20  12:17:44  robertj
 * Added code to assert
 *
 * Revision 1.23  1994/04/11  14:17:27  robertj
 * Made standard operators new and delete only declared for GNU C++
 *
 * Revision 1.22  1994/04/01  14:09:46  robertj
 * Removed PDECLARE_ABSTRACT_CONTAINER.
 * Added string stream class.
 * Added string containers.
 *
 * Revision 1.21  1994/03/07  07:38:19  robertj
 * Major enhancementsacross the board.
 *
 * Revision 1.20  1994/01/13  08:42:29  robertj
 * Fixed missing copy constuctor and assignment operator for PString.
 *
 * Revision 1.19  1994/01/13  05:33:41  robertj
 * Added contructor to get caseless string from ordinary string.
 *
 * Revision 1.18  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.17  1993/12/31  06:40:34  robertj
 * Made inlines optional for debugging purposes.
 * Added default to DeleteObjects() function.
 *
 * Revision 1.16  1993/12/24  04:20:52  robertj
 * Mac CFront port.
 *
 * Revision 1.15  1993/12/16  00:51:46  robertj
 * Made some container functions const.
 *
 * Revision 1.14  1993/12/15  21:10:10  robertj
 * Changes to fix inadequate reference system for containers.
 *
 * Revision 1.13  1993/12/14  18:44:56  robertj
 * Added RemoveAll() to collection classes.
 * Fixed incorrect destruction of objects in containers.
 *
 * Revision 1.12  1993/12/04  05:23:58  robertj
 * Added more string functions
 *
 * Revision 1.11  1993/09/27  16:35:25  robertj
 * Fixed bug in sorted lists.
 * Changed simple function for array of strings to a constructor.
 * Capitalised all macros.
 *
 * Revision 1.10  1993/08/27  18:17:47  robertj
 * Fixed bug with default number of elements in a collection.
 * Added missing Compare function to PAbstractSortedList
 * Added inline keywords for CFront compatibility.
 *
 * Revision 1.9  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.8  1993/08/19  18:00:32  robertj
 * Added two more standard base array classes
 *
 * Revision 1.7  1993/08/01  14:05:27  robertj
 * Added const to ToLower() and ToUpper() in the PString class.
 *
 * Revision 1.6  1993/07/16  14:40:55  robertj
 * Added PString constructor for individual characters.
 * Added string to C style literal format.
 *
 * Revision 1.5  1993/07/15  05:02:57  robertj
 * Removed redundant word in PString enum for string types.
 *
 * Revision 1.4  1993/07/15  04:23:39  robertj
 * Added constructor to PString to allow conversion from other string formats.
 * Fixed problem with variable parameter lists in sprintf() functions.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#ifndef _CONTAIN_H
#define _CONTAIN_H

#ifdef __GNUC__
#pragma interface
#endif

#include <object.h>


///////////////////////////////////////////////////////////////////////////////
// Abstract container class

PDECLARE_CLASS(PContainer, PObject)
/* Abstract class to embody the base functionality of a $I$container$I$.

   Fundamentally, a container is an object that contains other objects. There
   are two main areas of support for tha that are provided by this class. The
   first is simply to keep a count of the number of things that the container
   contains. The current size is stored and accessed by members of this class.
   The setting of size is determined by the semantics of the descendent class
   and so is a pure function.

   The second area of support is for reference integrity. When an instance of
   a container is copied to another instance, the two instance contain the
   same thing. There can therefore be multiple references to the same things.
   When one reference is destroyed this must $U$not$U$ destroy the contained
   object as it may be referenced by another instance of a container class.
   To this end a reference count is provided by the PContainer class. This
   assures that the container only destroys the objects it contains when there
   are no more references to them.

   In support of this, descendent classes must provide a DestroyContents()
   function. As the normal destructor cannot be used, this function will free
   the memory or unlock the resource the container is wrapping.
 */

  public:
    PContainer(
      PINDEX initialSize = 0  // Initial number of things in the container.
    );
    // Create a new unique container.

    PContainer(
      const PContainer & cont  // Container to create a new reference from.
    );
    /* Create a new container referencing the same contents as the container
       specified in the parameter.
     */

    PContainer & operator=(const PContainer & cont);
    /* Set the current container to reference the same thing as the container
       specified in the parameter.

       Note that the old contents of the container is dereferenced and if
       it was unique, destroyed using the DestroyContents() function.
     */

    virtual ~PContainer();
    /* Destroy the container class. This will decrement the reference count
       on the contents and if unique, will destroy it using the
       DestroyContents() function.
     */


  // New functions for class
    virtual PINDEX GetSize() const;
    /* Get the current size of the container. This represents the number of
       things the container contains. For some types of containers this will
       always return 1.

       Returns: number of objects in container.
     */

    virtual BOOL SetSize(
      PINDEX newSize  // New size for the container.
    ) = 0;
    /* Set the new current size of the container. The exact behavious of this
       is determined by the descendent class. For instance an array class would
       reallocate memory to make space for the new number of elements.

       Note for some types of containers this does not do anything as they
       inherently only contain one item. The function returns TRUE always and
       the new value is ignored.

       Returns: TRUE if the size was successfully changed. The value FALSE
                usually indicates failure due to insufficient memory.
     */

    BOOL SetMinSize(
      PINDEX minSize  // Possible, new size for the container.
    );
    /* This function will set the size of the object to be at least the size
       specified. The SetSize() function is always called, either with the
       new value or the previous size, whichever is the larger.
     */

    virtual BOOL IsEmpty() const;
    /* Determine if the container is empty, that is there are no objects in its
       contents.

       Returns: TRUE if GetSize() returns zero.
     */

    BOOL IsUnique() const;
    /* Determine if this instance is the one and only reference to the
       container contents.

       Returns: TRUE if the reference count is one.
     */

    virtual BOOL MakeUnique();
    /* Make this instance to be the one and only reference to the container
       contents. This implicitly does a clone of the contents of the container
       to make a unique reference. If the instance was already unique then
       the function does nothing.

       Returns: TRUE if the instance was already unique.
     */


  protected:
    PContainer(
      int dummy,        // Dummy to prevent accidental use of the constructor.
      const PContainer * cont  // Container class to clone.
    );
    /* Constructor used in support of the Clone() function. This creates a
       new unique reference of a copy of the contents. It does $U$not$U$ create
       another reference.

       The dummy parameter is there to prevent the contructor from being
       invoked automatically by the compiler when a pointer is used by accident
       when a normal instance or reference was expected. The container would
       be silently cloned and the copy used instead of the container expected
       leading to unpredictable results.
     */

    virtual void DestroyContents() = 0;
    /* Destroy the container contents. This function must be defined by the
       descendent class to do the actual destruction of the contents. It is
       automatically declared when the PDECLARE_CONTAINER() macro is used.

       For all descendent classes not immediately inheriting off the PContainer
       itself, the implementation of DestroyContents() should always call its
       ancestors function. This is especially relevent if many of the standard
       container classes, such as arrays, are descended from as memory leaks
       will occur.
     */

    virtual void CopyContents(const PContainer & c);
    /* Copy the container contents. This copies the contents from one reference
       to another. It is automatically declared when the PDECLARE_CONTAINER()
       macro is used.
       
       No duplication of contents occurs, for instance if the container is an
       array, the pointer to the array memory is copied, not the array memory
       block itself.

       This function will get called once for every class in the heirarchy, so
       the ancestor function should $U$not$U$ be called.
     */

    virtual void CloneContents(const PContainer * src);
    /* Create a duplicate of the container contents. This copies the contents
       from one container to another, unique container. It is automatically
       declared when the PDECLARE_CONTAINER() macro is used.
       
       This class will duplicate the contents completely, for instance if the
       container is an array, the actual array memory is copied, not just the
       pointer. If the container contains objects that descend from PObject,
       they too should also be cloned and not simply copied.

       This function will get called once for every class in the heirarchy, so
       the ancestor function should $U$not$U$ be called.
       
       $B$$U$Note well$U$$B$, the logic of the function must be able to accept
       the passed in parameter to clone being the same instance as the
       destination object, ie during execution $F$this == src$F$.
     */

    void Destruct();
    /* Internal function called from container destructors. This will
       conditionally call DestroyContents() to destroy the container contents.
     */


    class Reference {
      public:
        inline Reference(PINDEX initialSize)
          : size(initialSize), count(1), deleteObjects(TRUE) { }
        PINDEX   size;      // Size of what the container contains
        unsigned count;     // reference count to the container content
        BOOL deleteObjects; // Used by PCollection but put here for efficieny
    } * reference;
};



/*$MACRO PCONTAINERINFO(cls, par)
   This macro is used to declare all the functions that should be implemented
   for a working container class. It will also define some inline code for
   some standard function behaviour.

   This may be used when multiple inheritance requires a special class
   declaration. Normally, the $H$PDECLARE_CONTAINER macro would be used, which
   includes this macro in it.

   The default implementation for contructors, destructor, the assignment
   operator and the MakeUnique() function is as follows:
   $F$
        cls(const cls & c)
          : par(c)
        {
          CopyContents(c);
        }

        cls & operator=(const cls & c)
        {
          par::operator=(c);
          cls::CopyContents(c);
          return *this;
        }

        cls(int dummy, const cls * c)
          : par(dummy, c)
        {
          CloneContents(c);
        }

        virtual ~cls()
        {
          Destruct();
        }

        BOOL MakeUnique()
        {
          if (par::MakeUnique())
            return TRUE;
          CloneContents(c);
          return FALSE;
        }
    $F$
    Then the DestroyContents(), CloneContents() and CopyContents() functions
    are declared and must be implemted by the programmer. See the $H$PContainer
    class for more information on these functions.
 */
#define PCONTAINERINFO(cls, par) \
    PCLASSINFO(cls, par) \
  public: \
    cls(const cls & c) : par(c) { CopyContents(c); } \
    cls & operator=(const cls & c) \
      { par::operator=(c); cls::CopyContents(c); return *this; } \
    virtual ~cls() { Destruct(); } \
    virtual BOOL MakeUnique() \
      { if(par::MakeUnique())return TRUE; CloneContents(this);return FALSE; } \
  protected: \
    cls(int dummy, const cls * c) : par(dummy, c) { CloneContents(c); } \
    virtual void DestroyContents(); \
    virtual void CloneContents(const cls * c); \
    virtual void CopyContents(const cls & c); \


/*$MACRO PDECLARE_CONTAINER(cls, par)
   This macro is used to declare a descendent of $H$PContainer class. It
   declares all the functions that should be implemented for a working
   container class.

   See the $H$PCONTAINERINFO macro for more information.
 */
#define PDECLARE_CONTAINER(cls, par) \
                             PCLASS cls : public par { PCONTAINERINFO(cls, par)


///////////////////////////////////////////////////////////////////////////////
// Abstract collection of objects class

PDECLARE_CLASS(PCollection, PContainer)
/* A collection is a container that collects together descendents of the
   $H$PObject class. The objects contained in the collection are always
   pointers to objects, not the objects themselves. The life of an object in
   the collection should be carefully considered. Typically, it is allocated
   by the user of the collection when it is added. The collection then
   automatically deletes it when it is removed or the collection is destroyed,
   ie when the container class has no more references to the collection. Other
   models may be accommodated but it is up to the programmer to determine the
   scope and life of the objects.

   The exact form of the collection depends on the descendent of PCollection
   and determines the access modes for the objects in it. Thus a collection
   can be an array which allows fast random access at the expense of slow
   insertion and deletion. Or the collection may be a list which has fast
   insertion and deletion but very slow random access.

   The basic paradigm of all collections is the "virtual array". Regardless of
   the internal implementation of the collection; array, list, sorted list etc,
   the user may access elements via an ordinal index. The implementation then
   optimises the access as best it can. For instance, in a list ordinal zero
   will go directly to the head of the list. Stepping along sequential indexes
   then will return the next element of the list, remembering the new position
   at each step, thus allowing sequential access with little overhead as is
   expected for lists. If a random location is specified, then the list
   implementation must sequentially search for that ordinal from either the
   last location or an end of the list, incurring an overhead.

   All collection classes implement a base set of functions, though they may
   be meaningless or degenerative in some collection types eg $B$Insert()$B$$
   for $H$PSortedList will degenerate to be the same as $B$Append()$B$.
 */

  public:
    PCollection(
      PINDEX initialSize = 0  // Initial number of things in the collection.
    );
    /* Create a new collection
     */

    // Overrides from class PObject
    virtual ostream & PrintOn(
      ostream &strm   // Output stream to print the collection.
    ) const;
    /* Print the collection on the stream. This simply executes the
       $B$PrintOn()$B$ function on each element in the collection.

       Returns: the stream printed to.
     */

    // New functions for class
    virtual PINDEX Append(
      PObject * obj   // New object to place into the collection.
    ) = 0;
    /* Append a new object to the collection.
    
       The exact semantics depends on the specific type of the collection. So
       the function may not place the object at the "end" of the collection at
       all. For example, in a $H$PSortedList the object is placed in the
       correct ordinal position in the list.

       Returns: index of the newly added object.
     */

    virtual PINDEX Insert(
      const PObject & before,   // Object value to insert before.
      PObject * obj             // New object to place into the collection.
    ) = 0;
    /* Insert a new object immediately before the specified object. If the
       object to insert before is not in the collection then the equivalent of
       the $B$Append()$B$ function is performed.
       
       The exact semantics depends on the specific type of the collection. So
       the function may not place the object before the specified object at
       all. For example, in a $H$PSortedList the object is placed in the
       correct ordinal position in the list.

       Note that the object values are compared for the search of the
       $B$before$B$ parameter, not the pointers. So the objects in the
       collection must correctly implement the $B$Comapre()$B$ function.

       Returns: index of the newly inserted object.
     */

    virtual PINDEX InsertAt(
      PINDEX index,   // Index position in collection to place the object.
      PObject * obj   // New object to place into the collection.
    ) = 0;
    /* Insert a new object at the specified ordinal index. If the index is
       greater than the number of objects in the collection then the
       equivalent of the $B$Append()$B$ function is performed.

       The exact semantics depends on the specific type of the collection. So
       the function may not place the object at the specified index at all.
       For example, in a $H$PSortedList the object is placed in the correct
       ordinal position in the list.

       Returns: index of the newly inserted object.
     */

    virtual BOOL Remove(
      const PObject * obj   // Existing object to remove from the collection.
    ) = 0;
    /* Remove the object from the collection. If the AllowDeleteObjects option
       is set then the object is also deleted.

       Note that the comparison for searching for the object in collection is
       made by pointer, not by value. Thus the parameter must point to the
       same instance of the object that is in the collection.

       Returns: TRUE if the object was in the collection.
     */

    virtual PObject * RemoveAt(
      PINDEX index   // Index position in collection to place the object.
    ) = 0;
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
    ) = 0;
    /* Set the object at the specified ordinal position to the new value. This
       will overwrite the existing entry. If the AllowDeleteObjects option is
       set then the old object is also deleted.

       The exact semantics depends on the specific type of the collection. For
       some, eg $H$PSortedList, the object inserted will not stay at the
       ordinal position. Also the exact behaviour when the index is greater
       than the size of the collection depends on the collection type, eg in
       an array collection the array is expanded to accommodate the new index,
       whereas in a list it will return FALSE.

       Returns: TRUE if the object was successfully added.
     */

    virtual PObject * GetAt(
      PINDEX index  // Index position in the collection of the object.
    ) const = 0;
    /* Get the object at the specified ordinal position. If the index was
       greater than the size of the collection then NULL is returned.

       Returns: pointer to object at the specified index.
     */

    virtual PINDEX GetObjectsIndex(
      const PObject * obj
    ) const = 0;
    /* Search the collection for the specific instance of the object. The
       object pointers are compared, not the values. The fastest search
       algorithm is employed depending on the collection type.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */

    virtual PINDEX GetValuesIndex(
      const PObject & obj
    ) const = 0;
    /* Search the collection for the specified value of the object. The object
       values are compared, not the pointers.  So the objects in the
       collection must correctly implement the $B$Comapre()$B$ function. The
       fastest search algorithm is employed depending on the collection type.

       Returns: ordinal index position of the object, or P_MAX_INDEX.
     */

    void AllowDeleteObjects(
      BOOL yes = TRUE   // New value for flag for deleting objects
    );
    /* Allow or disallow the deletion of the objects contained in the
       collection. If TRUE then whenever an object is removed, overwritten or
       the colelction is deleted due to all references being destroyed, the
       object is deleted.

       For example: $F$
              coll.SetAt(2, new PString("one"));
              coll.SetAt(2, new PString("Two"));
              $F$
       would automatically delete the string containing "one" on the second
       call to SetAt().
     */

    void DisallowDeleteObjects();
    /* Disallow the deletion of the objects contained in the collection. See
       the $H$AllowDeleteObjects() function for more details.
     */

  protected:
    PCollection(
      int dummy,        // Dummy to prevent accidental use of the constructor.
      const PCollection * coll  // Collection class to clone.
    );
    /* Constructor used in support of the Clone() function. This creates a
       new unique reference of a copy of the contents. It does $U$not$U$ create
       another reference.

       The dummy parameter is there to prevent the contructor from being
       invoked automatically by the compiler when a pointer is used by accident
       when a normal instance or reference was expected. The container would
       be silently cloned and the copy used instead of the container expected
       leading to unpredictable results.
     */
};



///////////////////////////////////////////////////////////////////////////////
// The abstract array class

#include <array.h>

///////////////////////////////////////////////////////////////////////////////
// The abstract array class

#include <lists.h>

///////////////////////////////////////////////////////////////////////////////
// PString class (specialised version of PBASEARRAY(char))

#include <dict.h>


///////////////////////////////////////////////////////////////////////////////
// PString class

#include <pstring.h>



///////////////////////////////////////////////////////////////////////////////
// Fill in all the inline functions

#if defined(P_USE_INLINES)
#include "contain.inl"
#endif

#endif // _CONTAIN_H


// End Of File ///////////////////////////////////////////////////////////////

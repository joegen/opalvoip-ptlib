/*
 * $Id: object.h,v 1.11 1995/03/12 04:40:55 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: object.h,v $
 * Revision 1.11  1995/03/12 04:40:55  robertj
 * Changed standard error code for not open from file to channel.
 *
 * Revision 1.10  1995/02/19  04:19:14  robertj
 * Added dynamically linked command processing.
 *
 * Revision 1.9  1995/02/05  00:48:07  robertj
 * Fixed template version.
 *
 * Revision 1.8  1995/01/15  04:51:31  robertj
 * Mac compatibility.
 * Added levels of memory checking.
 *
 * Revision 1.7  1995/01/09  12:38:31  robertj
 * Changed variable names around during documentation run.
 * Fixed smart pointer comparison.
 * Fixed serialisation stuff.
 * Documentation.
 *
 * Revision 1.6  1995/01/03  09:39:06  robertj
 * Put standard malloc style memory allocation etc into memory check system.
 *
 * Revision 1.5  1994/12/12  10:08:30  robertj
 * Renamed PWrapper to PSmartPointer..
 *
 * Revision 1.4  1994/12/05  11:23:28  robertj
 * Fixed PWrapper macros.
 *
 * Revision 1.3  1994/11/19  00:22:55  robertj
 * Changed PInteger to be INT, ie standard type like BOOL/WORD etc.
 * Moved null object check in notifier to construction rather than use.
 * Added virtual to the callback function in notifier destination class.
 *
 * Revision 1.2  1994/11/03  09:25:30  robertj
 * Made notifier destination object not to be descendent of PObject.
 *
 * Revision 1.1  1994/10/30  12:01:37  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#ifndef __MWERKS__
#include <iomanip.h>
#endif



///////////////////////////////////////////////////////////////////////////////
// Disable inlines when debugging for faster compiles (the compiler doesn't
// actually inline the function with debug on any way).

#ifdef P_USE_INLINES
#define PINLINE inline
#else
#define PINLINE
#endif


///////////////////////////////////////////////////////////////////////////////
// Declare the debugging support

enum PStandardAssertMessage {
  PLogicError,              // A logic error occurred.
  POutOfMemory,             // A new or malloc failed.
  PNullPointerReference,    // A reference was made through a NULL pointer.
  PInvalidArrayIndex,       // An index into an array was negative.
  PInvalidArrayElement,     // A NULL array element object was accessed.
  PStackEmpty,              // A Pop() was made of a stack with no elements.
  PUnimplementedFunction,   // Funtion is not implemented.
  PInvalidParameter,        // Invalid parameter was passed to a function.
  POperatingSystemError,    // Error was returned by Operating System.
  PChannelNotOpen,          // Operation attempted when channel not open.
  PUnsupportedFeature,      // Feature is not supported.
  PInvalidWindow,           // Access through invalid window.
  PMaxStandardAssertMessage
};
// Standard assert messages for the PAssert macro.

/*$MACRO PAssert(b, m)
   This macro is used to assert that a condition must be TRUE. If the condition
   is FALSE then an assert function is called with the source file and line
   number the macro was instantiated on, plus the message described by the
   $B$m$B$ parameter. This parameter may be either a standard value from the
   $H$PStandardAssertMessage enum or a literal string.
 */
#define PAssert(b, m) if(b);else PAssertFunc(__FILE__, __LINE__, (m))

/*$MACRO PAssertOS(b)
   This macro is used to assert that a condition must be TRUE. If the condition
   is FALSE then an assert function is called with the source file and line
   number the macro was instantiated on, plus the message described by the
   $B$POperatingSystemError$B$ value in the $H$PStandardAssertMessage enum.
 */
#define PAssertOS(b) \
              if(b);else PAssertFunc(__FILE__, __LINE__, POperatingSystemError)

/*$MACRO PAssertNULL(p)
   This macro is used to assert that a pointer must be non-null. If the
   pointer is NULL then an assert function is called with the source file and
   line number the macro was instantiated on, plus the message described by the
   $B$PNullPointerReference$B$ value in the $H$PStandardAssertMessage enum.
   
   Note that this evaluates the expression defined by $B$p$B$ twice. To prevent
   incorrect behaviour with this, the macro will assume that the $B$p$B$
   parameter is an L-Value.
 */
#define PAssertNULL(p) ((&(p)&&(p)!=NULL)?(p):(PAssertFunc(__FILE__, \
                                        __LINE__, PNullPointerReference), (p)))

/*$MACRO PAssertAlways(m)
   This macro is used to assert immediately. The assert function is called with
   the source file and line number the macro was instantiated on, plus the
   message described by the $B$m$B$ parameter. This parameter may be either a
   standard value from the $H$PStandardAssertMessage enum or a literal string.
 */
#define PAssertAlways(m) PAssertFunc(__FILE__, __LINE__, (m))


void PAssertFunc(const char * file, int line, PStandardAssertMessage msg);
void PAssertFunc(const char * file, int line, const char * msg);


// Declaration for standard error output
extern ostream * PSTATIC PErrorStream;

/*$MACRO PError
   This macro is used to access the platform specific error output stream. This
   is to be used in preference to assuming $B$cerr$B$ is always available. On
   Unix platforms this $U$is$U$ $B$cerr$B$ but for MS-Windows this is another
   stream that uses the OutputDebugString() Windows API function. Note that a
   MS-DOS or Windows NT console application would still use $B$cerr$B$.

   The PError stream would normally only be used for debugging information as
   a suitable display is not always available in windowed environments.
   
   The macro is a wrapper for a global variable $B$PErrorStream$B$ which is
   a pointer to an $B$ostream$B$. The variable is initialised to $B$cerr$B$
   for all but MS-Windows and NT GUI applications. An application could change
   this pointer to a $B$ofstream$B$ variable of PError output is wished to be
   redirected to a file.
 */
#define PError (*PErrorStream)


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

/*$MACRO PARRAYSIZE(array)
   This macro is used to calculate the number of array elements in a static
   array.
 */
#define PARRAYSIZE(array) ((PINDEX)(sizeof(array)/sizeof(array[0])))

/*$MACRO PMIN(v1, v2)
   This macro is used to calculate the minimum of two values. As this is a
   macro the expression in $B$v1$B$ or $B$v2$B$ is executed twice so extreme
   care should be made in its use.
 */
#define PMIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))

/*$MACRO PMAX(v1, v2)
   This macro is used to calculate the maximum of two values. As this is a
   macro the expression in $B$v1$B$ or $B$v2$B$ is executed twice so extreme
   care should be made in its use.
 */
#define PMAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))

/*$MACRO PABS(v)
   This macro is used to calculate an absolute value. As this is a macro the
   expression in $B$v$B$ is executed twice so extreme care should be made in
   its use.
 */
#define PABS(v) ((v) < 0 ? -(v) : (v))



///////////////////////////////////////////////////////////////////////////////
// The low level object support, memory checks, run time typing etc.

#if defined(PMEMORY_CHECK) && PMEMORY_CHECK>2

/*$MACRO PMALLOC(s)
   This macro is used to allocate memory via the memory check system selected
   with the PMEMORY_CHECK compile time option.
   
   This macro should be used instead of the system $B$malloc()$B$ function.
 */
#define PMALLOC(s) PObject::MemoryCheckAllocate(s, __FILE__, __LINE__, NULL)

/*$MACRO PCALLOC(n,s)
   This macro is used to allocate memory via the memory check system selected
   with the PMEMORY_CHECK compile time option.
   
   This macro should be used instead of the system $B$calloc()$B$ function.
 */
#define PCALLOC(n,s) PObject::MemoryCheckAllocate(n, s, __FILE__, __LINE__)

/*$MACRO PREALLOC(p,s)
   This macro is used to allocate memory via the memory check system selected
   with the PMEMORY_CHECK compile time option.
   
   This macro should be used instead of the system $B$realloc()$B$ function.
 */
#define PREALLOC(p,s) PObject::MemoryCheckReallocate(p, s, __FILE__, __LINE__)

/*$MACRO PFREE(p)
   This macro is used to deallocate memory via the memory check system selected
   with the PMEMORY_CHECK compile time option.
   
   This macro should be used instead of the system $B$free()$B$ function.
 */
#define PFREE(p) PObject::MemoryCheckDeallocate(p, NULL)

#else // defined(PMEMORY_CHECK) && PMEMORY_CHECK>1


#define PMALLOC(s)    malloc(s)
#define PCALLOC(n,s)  calloc(n, s)
#ifdef __GNUC__
inline void * p_realloc(void * p, size_t s) // Bug in Linux GNU realloc()
  { return realloc(p, s <= 4 ? 4 : s); }
#define PREALLOC(p,s) p_realloc(p, s)
#else
#define PREALLOC(p,s) realloc(p, s)
#endif
#define PFREE(p)      free(p)


#endif // defined(PMEMORY_CHECK) && PMEMORY_CHECK>1


#if defined(PMEMORY_CHECK)

/*$MACRO PNEW
   This macro is used to allocate memory via the memory check system selected
   with the PMEMORY_CHECK compile time option.

   This macro should be used instead of the system $B$new$B$ operator.
 */
#define PNEW new(__FILE__, __LINE__)


#define PNEW_AND_DELETE_FUNCTIONS \
    void * operator new(size_t nSize, const char * file, int line) \
      { return MemoryCheckAllocate(nSize, file, line, Class()); } \
    void * operator new(size_t nSize) \
      { return MemoryCheckAllocate(nSize, NULL, 0, Class()); } \
    void operator delete(void * ptr) \
      { MemoryCheckDeallocate(ptr, Class()); }


#else // defined(PMEMORY_CHECK)

#define PNEW new
#define PNEW_AND_DELETE_FUNCTIONS


#endif // defined(PMEMORY_CHECK)


/*$MACRO PCLASSINFO(cls, par)
   This macro is used to provide the basic run-time typing capability needed
   by the library. All descendent classes from the $H$PObject class require
   these functions for correct operation. Either use this macro or the
   $H$PDECLARE_CLASS macro.
 */
#define PCLASSINFO(cls, par) \
  public: \
    static const char * Class() \
      { return #cls; } \
    virtual const char * GetClass(unsigned ancestor = 0) const \
      { return ancestor > 0 ? par::GetClass(ancestor-1) : cls::Class(); } \
    virtual BOOL IsClass(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0; } \
    virtual BOOL IsDescendant(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0 || par::IsDescendant(clsName); } \
    virtual Comparison CompareObjectMemoryDirect(const PObject & obj) const \
      { return (Comparison)memcmp(this, &obj, sizeof(cls)); } \
    PNEW_AND_DELETE_FUNCTIONS

/*$MACRO PDECLARE_CLASS(cls, par)
   This macro is used to declare a new class with a single public ancestor. It
   starts the class declaration and then uses the $H$PCLASSINFO macro to get
   all the run-time type functions.
 */
#define PDECLARE_CLASS(cls, par) PCLASS cls : public par { PCLASSINFO(cls, par)


class PSerialiser;
class PUnSerialiser;


///////////////////////////////////////////////////////////////////////////////
// The root of all evil ... umm classes

PCLASS PObject {
/* Ultimate parent class for all objects in the class library. This provides
   functionality provided to all classes, eg run-time types, default comparison
   operations, simple stream I/O and serialisation support.
 */

  public:
    virtual ~PObject() { }
    /* Destructor required to get the "virtual". A PObject really has nothing
       to destroy.
     */

    static const char * Class() { return "PObject"; }
    /* Get the name of the class as a C string. This is a static function which
       returns the type of a specific class. It is primarily used as an
       argument to the $B$IsClass()$B$ or $B$IsDescendant()$B$ functions.
       
       When comparing class names, always use the $B$strcmp()$B$ function
       rather than comparing pointers. The pointers are not necessarily the
       same over compilation units depending on the compiler, platform etc.

       The $H$PCLASSINFO macro declares a version of this function for the
       particular class.

       Returns: pointer to C string literal.
     */      

    virtual const char * GetClass(
      unsigned ancestor = 0
      /* Level of ancestor to get the class name for. A value of zero is the
         instances class name, one is its ancestor, two for the ancestors
         ancestor etc.
       */
    ) const;
    /* Get the current dynamic type of the object instance.

       When comparing class names, always use the $B$strcmp()$B$ function
       rather than comparing pointers. The pointers are not necessarily the
       same over compilation units depending on the compiler, platform etc.

       The $H$PCLASSINFO macro declares an override of this function for the
       particular class. The user need not implement it.

       Returns: pointer to C string literal.
     */

    virtual BOOL IsClass(
      const char * clsName    // Class name to compare against.
    ) const;
    /* Determine if the dynamic type of the current instance is of the
       specified class. The class name is usually provided by the $B$Class()$B$
       static function of the desired class.
    
       The $H$PCLASSINFO macro declares an override of this function for the
       particular class. The user need not implement it.

       Returns: TRUE if object is of the class.
     */

    virtual BOOL IsDescendant(
      const char * clsName    // Ancestor class name to compare against.
    ) const;
    /* Determine if the dynamic type of the current instance is a descendent of
       the specified class. The class name is usually provided by the
       $B$Class()$B$ static function of the desired class.
    
       The $H$PCLASSINFO macro declares an override of this function for the
       particular class. The user need not implement it.

       Returns: TRUE if object is descended from the class.
     */


    enum Comparison {
      LessThan = -1,    // Object is less than parameter.
      EqualTo = 0,      // Object is equal to parameter.
      GreaterThan = 1   // Object is greater than parameter.
    };
    /* Result of the comparison operation performed by the $B$Compare()$B$
       function.
      */

    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    /* Compare the two objects and return their relative rank. This function is
       usually overridden by descendent classes to yield the ranking according
       to the semantics of the object.
       
       The default function is to use the $B$CompareObjectMemoryDirect()$B$
       function to do a byte wise memory comparison of the two objects.

       Returns: $B$LessThan$B$, $B$EqualTo$B$ or $B$GreaterThan$B$ according to
                the relative rank of the objects.
     */
    
    virtual Comparison CompareObjectMemoryDirect(
      const PObject & obj   // Object to compare against.
    ) const;
    /* Determine the byte wise comparison of two objects. This is the default
       comparison operation for objects that do not explicitly override the
       $B$Compare()$B$ function.
    
       The $H$PCLASSINFO macro declares an override of this function for the
       particular class. The user need not implement it.

       Returns: $B$LessThan$B$, $B$EqualTo$B$ or $B$GreaterThan$B$ according to
                the result $B$memcpy()$B$ function.
     */

    BOOL operator==(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == EqualTo; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are equal.
     */

    BOOL operator!=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != EqualTo; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are not equal.
     */

    BOOL operator<(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == LessThan; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are less than.
     */

    BOOL operator>(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == GreaterThan; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are greater than.
     */

    BOOL operator<=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != GreaterThan; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are less than or equal.
     */

    BOOL operator>=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != LessThan; }
    /* Compare the two objects.
    
       Returns: TRUE if objects are greater than or equal.
     */


    virtual PObject * Clone() const;
    /* Create a copy of the class on the heap. The exact semantics of the
       descendent class determine what is required to make a duplicate of the
       instance. Not all classes can even $U$do$U$ a clone operation.
       
       The main user of the clone function is the $H$PDictionary class as it
       requires copies of the dictionary keys.

       The default behaviour is for this function to assert.

       Returns: pointer to new copy of the class instance.
     */

    virtual PINDEX HashFunction() const;
    /* This function yields a hash value required by the $H$PDictionary class.
       A descendent class that is required to be the key of a dictionary should
       override this function. The precise values returned is dependent on the
       semantics of the class. For example, the $H$PString class overrides it
       to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       Returns: hash function value for class instance.
     */

    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;
    /* Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard $B$operator<<$B$ function.

       The default behaviour is to print the class name.
     */

    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );
    /* Input the contents of the object from the stream. The exact input is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard $B$operator<<$B$ function.

       The default behaviour is to do nothing.
     */


    inline friend ostream & operator<<(
      ostream &strm,       // Stream to print the object into.
      const PObject & obj  // Object to print to the stream.
    ) { obj.PrintOn(strm); return strm; }
    /* Global function for using the standard << operator on objects descended
       from PObject. This simply calls the objects $B$PrintOn()$B$ function.
       
       Returns: the $B$strm$B$ parameter.
     */

    inline friend istream & operator>>(
      istream &strm,   // Stream to read the objects contents from.
      PObject & obj    // Object to read inormation into.
    ) { obj.ReadFrom(strm); return strm; }
    /* Global function for using the standard >> operator on objects descended
       from PObject. This simply calls the objects $B$ReadFrom()$B$ function.

       Returns: the $B$strm$B$ parameter.
     */


    virtual PINDEX PreSerialise(
      PSerialiser & strm   // Serialiser stream to serialise object into.
    );
    /* This function is used to determine the size of the object and all other
       objects it contains. The actual size is dependent on the exact semantics
       of the descendent object. For example the $H$PString class would return
       the length of the string plus one, while the $H$PList class would return
       the sum of the sizes of all of the objects in the list plus the size of
       an integer for the number of objects.

       This in only required by the $H$PBinarySerialiser class which serialises
       the objects into a binary file. The $H$PTextSerialiser class which
       serialises into a text stream does not use this function.

       Note serialisation requires the use of the $H$PDECLARE_SERIAL and
       $H$PIMPLEMENT_SERIAL macros.

       Returns: size in bytes of object.
     */

    virtual void Serialise(
      PSerialiser & strm   // Serialiser stream to serialise object into.
    );
    /* Serialise the object into the specified stream. This is similar to the
       $B$PrintOn()$B$ function that outputs the contents of the object to a
       stream, but where $B$PrintOn()$B$ usually produces a human readable form
       of the object, this function outputs enough data so that it can be
       reconstructed by the $H$PUnSerialiser class.
       
       When the user implements this function they will usually be doing it for
       one of either the text of binary output versions. In some circumstances,
       eg libraries, both need be supported so the $B$IsDscendent()$B$
       function should be used on the $B$strm$B$ parameter to determine whether
       it is a $H$PBinarySerialiser class or a $H$PTextSerialiser class and do
       the appropriate output.

       To a large extent, if only the << operator is used on the $H$PSerialiser
       instance, the text and binary version can be made identical.
     */

    virtual void UnSerialise(
      PUnSerialiser & strm   // Serialiser stream to serialise object into.
    );
    /* Un-serialise the object from the specified stream. This is similar to
       the $B$ReadFrom()$B$ function that inputs the contents of the object
       from a stream, but where $B$ReadFrom()$B$ usually intrerprets a human
       readable form of the object, this function inputs enough data so that
       it can be reconstructed from the data provided by the $B$Serialise()$B$
       function.

       When the user implements this function they will usually be doing it for
       one of either the text of binary input versions. In some circumstances,
       eg libraries, both need be supported so the $B$IsDscendent()$B$ function
       should be used on the $B$strm$B$ parameter to determine whether it is a
       $H$PBinarySerialiser class or a $H$PTextSerialiser class and do the
       appropriate input.

       To a large extent, if only the >> operator is used on the
       $H$PUnSerialiser instance, the text and binary version can be made
       identical.
     */

#if defined(PMEMORY_CHECK)

    static void * MemoryCheckAllocate(
      size_t nSize,           // Number of bytes to allocate.
      const char * file,      // Source file name for allocating function.
      int line,               // Source file line for allocating function.
      const char * className  // Class name for allocating function.
    );
    static void * MemoryCheckAllocate(
      size_t count,       // Number of items to allocate.
      size_t iSize,       // Size in bytes of each item.
      const char * file,  // Source file name for allocating function.
      int line            // Source file line for allocating function.
    );
    /* Allocate a memory block.
    
       This funtion will only be present if the PMEMORY_CHECK compile time
       option is specified.
       
       Returns: pointer to newly allocated memory block.
     */

    static void * MemoryCheckReallocate(
      void * ptr,         // Pointer to memory block to reallocate.
      size_t nSize,       // New number of bytes to allocate.
      const char * file,  // Source file name for allocating function.
      int line            // Source file line for allocating function.
    );
    /* Change the size of an allocated memory block.
    
       This funtion will only be present if the PMEMORY_CHECK compile time
       option is specified.
       
       Returns: pointer to reallocated memory block. Note this may $U$not$U$ be
                the same as the pointer passed into the function.
     */

    static void MemoryCheckDeallocate(
      void * ptr,             // Pointer to memory block to deallocate.
      const char * className  // Class name for deallocating function.
    );
    /* Free a memory block.
    
       This funtion will only be present if the PMEMORY_CHECK compile time
       option is specified.
     */

    static void MemoryCheckStatistics(
      long * currentMemory,   // Current memory usage in bytes.
      long * peakMemory,      // Peak memory usage in bytes.
      long * currentObjects,  // Current number of memory object.
      long * peakObjects,     // Peak number of memory objects.
      long * totalObjects     // Total number of memory objects created.
    );
    /* Get memory check system statistics.

       If any parameter is NULL then it is not returned.
     */

    PNEW_AND_DELETE_FUNCTIONS

#else

    void * operator new(
      size_t nSize  // Number of bytes to allocate.
    ) { void*obj=malloc(nSize); PAssert(obj!=NULL,POutOfMemory); return obj; }
    /* Get a new block of memory using the system standard $B$malloc()$B$
       function. This overrides the standard $B$new$B$ operator to put in a
       NULL pointer check in out of memory conditions.

       Returns: pointer to newly allocated block of memory.
     */

    void operator delete(
      void * ptr    // Pointer to memory block to deallocate.
    ) { free(ptr); }
    /* Free the memory used by the object. This is required to balance the
       override of the $B$new$B$ operator.
     */

#endif
};



///////////////////////////////////////////////////////////////////////////////
// Serialisation

class PUnSerialiser;

class PSerialRegistration {
/* This class is for registration of object classes that will be serialised and
   un-serialised.

   As objects are un-serialised, the objects need to be constructed. For the
   $H$PUnSerialiser instance to know what constructor to call, a registration
   of functions that call the appropriate constructor.

   The $H$PDECLARE_SERIAL macro creates a single instance of this class to
   register the class with the serialiser.

   Even though this class implements a hash table it does $U$not$U$ use the
   standard $H$PHashTable or $H$PDictionary classes due to recursive definition
   problems. Those classes need to register themselves with this class before
   they can be used!
 */

  public:
    typedef PObject * (*CreatorFunction)(PUnSerialiser * serial);
    /* This type is a pointer to a function to create objects during the
       un-serialisation operation.
     */

    PSerialRegistration(
      const char * clsNam,    // Name of class to register.
      CreatorFunction func    // Constructor function for the class.
    );
    /* Create a serialiser class registration. This is unversally called by
       static member variables in the $H$PDECLARE_SERIAL and
       $H$PIMPLEMENT_SERIAL macros.
     */

    static CreatorFunction GetCreator(
      const char * clsNam   // Name of class to get the creator function for.
    );
    /* Get the creator function for the class name specified.

       Returns: function to construct objects.
     */

  protected:
    const char * className;
    // This serialiser registrations class

    CreatorFunction creator;
    /* This serialiser registrations creator function - the function that will
       make a new object of the classes type and construct it with an instance
       of the $H$PSerialiser class.
     */

    PSerialRegistration * clash;
    // Pointer to next registration when a hash clash occurs.

    enum { HashTableSize = 41 };
    // Constant for size of hash table.

    static PINDEX HashFunction(
      const char * className    // Class name to calculate hash function for.
    );
    // Calculate the bucket for the hash table lookup.

    static PSerialRegistration * creatorHashTable[HashTableSize];
    // A static dictionary of class names to creator functions.
};


PDECLARE_CLASS(PSerialiser, PObject)
/* This class allows the serialisation of objects to an output stream. This
   packages up objects so that they can be reconstructed by an instance of the
   $H$PUnSerialiser class. The stream they are sent to can be any stream; file,
   string, pipe, socket etc.

   Serialisation can be done in two manners: binary or text. This depends on
   the serialiser instance that was constrcted. Each objects $B$Serialise()$B$
   function is called and it is up to that function to output in binary or
   text.

   To a large extent, if only the << operator is used on the $H$PSerialser
   instance, the text and binary versions of the $B$Serialise()$B$ function
   can be made identical.

   This class is an abstract class and descendents of $H$PTextSerialiser or
   $H$PBinarySerialiser should be created.
 */

  public:
    PSerialiser(
      ostream & strm  // Stream to output serialisation to.
    );
    // Construct a serialiser.

    virtual PSerialiser & operator<<(char) = 0;
    virtual PSerialiser & operator<<(unsigned char) = 0;
    virtual PSerialiser & operator<<(signed char) = 0;
    virtual PSerialiser & operator<<(short) = 0;
    virtual PSerialiser & operator<<(unsigned short) = 0;
    virtual PSerialiser & operator<<(int) = 0;
    virtual PSerialiser & operator<<(unsigned int) = 0;
    virtual PSerialiser & operator<<(long) = 0;
    virtual PSerialiser & operator<<(unsigned long) = 0;
    virtual PSerialiser & operator<<(float) = 0;
    virtual PSerialiser & operator<<(double) = 0;
    virtual PSerialiser & operator<<(long double) = 0;
    virtual PSerialiser & operator<<(const char *) = 0;
    virtual PSerialiser & operator<<(const unsigned char *) = 0;
    virtual PSerialiser & operator<<(const signed char *) = 0;
    virtual PSerialiser & operator<<(PObject &);
    /* Output the data to the serialiser object. When the operator is executed
       on a $H$PObject descendent then that objects $B$Serialise()$B$ function
       is called.
     */

  protected:
    ostream & stream;
    // Stream to output serial data to.
};


PDECLARE_CLASS(PUnSerialiser, PObject)
/* This class allows the un-serialisation of objects from an input stream. This
   reconstruct objects that where packaged earlier by an instance of the
   $H$PSerialise class. The stream they are received from can be any stream;
   file, string, pipe, socket etc.

   Serialisation can be done in two manners: binary or text. This depends on
   the serialiser instance that was constrcted. Each objects $B$Serialise()$B$
   function is called and it is up to that function to output in binary or
   text.

   To a large extent, if only the << operator is used on the $H$PSerialser
   instance, the text and binary versions of the $B$Serialise()$B$ function
   can be made identical.

   This class is an abstract class and descendents of $H$PTextSerialiser or
   $H$PBinarySerialiser should be created.
 */

  public:
    PUnSerialiser(
      istream & strm    // Stream to read the serialised objects from.
    );
    // Construct an un-serialiser.

    virtual PUnSerialiser & operator>>(char &) = 0;
    virtual PUnSerialiser & operator>>(unsigned char &) = 0;
    virtual PUnSerialiser & operator>>(signed char &) = 0;
    virtual PUnSerialiser & operator>>(short &) = 0;
    virtual PUnSerialiser & operator>>(unsigned short &) = 0;
    virtual PUnSerialiser & operator>>(int &) = 0;
    virtual PUnSerialiser & operator>>(unsigned int &) = 0;
    virtual PUnSerialiser & operator>>(long &) = 0;
    virtual PUnSerialiser & operator>>(unsigned long &) = 0;
    virtual PUnSerialiser & operator>>(float &) = 0;
    virtual PUnSerialiser & operator>>(double &) = 0;
    virtual PUnSerialiser & operator>>(long double &) = 0;
    virtual PUnSerialiser & operator>>(char *) = 0;
    virtual PUnSerialiser & operator>>(unsigned char *) = 0;
    virtual PUnSerialiser & operator>>(signed char *) = 0;
    virtual PUnSerialiser & operator>>(PObject &) = 0;
    /* Input the data from the un-serialiser object. When the operator is
       executed on a $H$PObject descendent then that objects
       $B$UnSerialise()$B$ function is called.
     */

  protected:
    istream & stream;
    // Stream the read un-serialiser data from.
};


/*$MACRO PDECLARE_SERIAL(cls)
   This macro is used to declare functions required by the serialisation
   system. It is used in conjunction with the $H$PIMPLEMENT_SERIAL macro.

   This declares the $B$PreSerialise()$B$ and $B$Serialise()$B$ functions
   which must be imeplemented by the user. The un-serialisation and
   registration is declared and implemented by these two functions
   automatically.
 */
#define PDECLARE_SERIAL(cls) \
  public: \
    virtual PINDEX PreSerialise(PSerialiser & strm); \
    virtual void Serialise(PSerialiser & serial); \
    virtual void UnSerialise(PUnSerialiser & serial); \
  protected: \
    cls(PUnSerialiser & serial); \
    static cls * UnSerialiseNew(PUnSerialiser & serial); \
  private: \
    PINDEX serialisedLength; \
    static PSerialRegistration pRegisterSerial; \

/*$MACRO PIMPLEMENT_SERIAL(cls)
   This macro is used to implement functions required by the serialisation
   system. It is used in conjunction with the $H$PDECLARE_SERIAL macro.
 */
#define PIMPLEMENT_SERIAL(cls) \
  cls * cls::UnSerialiseNew(PUnSerialiser & serial) \
    { return new cls(serial); } \
  PSerialRegistration cls::pRegisterSerial(cls::Class(), cls::UnSerialise); \


PDECLARE_CLASS(PTextSerialiser, PSerialiser)
/* This serialiser class serialises each object using ASCII text. This gives
   the highest level of portability for streams and platforms at the expense
   if larger amounts of data.
 */

  public:
    PTextSerialiser(
      ostream & strm,   // Stream to serialise to.
      PObject & data    // First object to serialise.
    );
    // Create a text serialiser.

    PSerialiser & operator<<(char);
    PSerialiser & operator<<(unsigned char);
    PSerialiser & operator<<(signed char);
    PSerialiser & operator<<(short);
    PSerialiser & operator<<(unsigned short);
    PSerialiser & operator<<(int);
    PSerialiser & operator<<(unsigned int);
    PSerialiser & operator<<(long);
    PSerialiser & operator<<(unsigned long);
    PSerialiser & operator<<(float);
    PSerialiser & operator<<(double);
    PSerialiser & operator<<(long double);
    PSerialiser & operator<<(const char *);
    PSerialiser & operator<<(const unsigned char *);
    PSerialiser & operator<<(const signed char *);
    /* Output the data to the serialiser object. When the operator is executed
       on a $H$PObject descendent then that objects $B$Serialise()$B$ function
       is called.
     */
};


class PSortedStringList;

PDECLARE_CLASS(PBinarySerialiser, PSerialiser)
/* This serialiser class serialises each object using binary data. This gives
   the highest level data density at the expense of some portability and
   possibly the speed of execution.
   
   This is because two passes through the objects is made, the first to
   determine the classes and sizes and the second to actually output the data.
   A table of classes must also be output to set the correspondence between
   the class codes used in the output and the class names that are required by
   the unserialiser to construct instances of those classes.
 */

  public:
    PBinarySerialiser(
      ostream & strm,   // Stream to serialise to.
      PObject & data    // First object to serialise.
    );
    // Create a binary serialiser.

    ~PBinarySerialiser();
    // Destroy the serialiser and its class table.

    PSerialiser & operator<<(char);
    PSerialiser & operator<<(unsigned char);
    PSerialiser & operator<<(signed char);
    PSerialiser & operator<<(short);
    PSerialiser & operator<<(unsigned short);
    PSerialiser & operator<<(int);
    PSerialiser & operator<<(unsigned int);
    PSerialiser & operator<<(long);
    PSerialiser & operator<<(unsigned long);
    PSerialiser & operator<<(float);
    PSerialiser & operator<<(double);
    PSerialiser & operator<<(long double);
    PSerialiser & operator<<(const char *);
    PSerialiser & operator<<(const unsigned char *);
    PSerialiser & operator<<(const signed char *);
    /* Output the data to the serialiser object. When the operator is executed
       on a $H$PObject descendent then that objects $B$Serialise()$B$ function
       is called.
     */

  protected:
    PSortedStringList * classesUsed;
    // List of classes used during serialisation.
};


PDECLARE_CLASS(PTextUnSerialiser, PUnSerialiser)
/* This un-serialiser class reconstructs each object using ASCII text. This
   gives the highest level of portability for streams and platforms at the
   expense if larger amounts of data.
 */

  public:
    PTextUnSerialiser(
      istream & strm    // Stream to read serialised objects from.
    );
    // Create a text un-serialiser.

    PUnSerialiser & operator>>(char &);
    PUnSerialiser & operator>>(unsigned char &);
    PUnSerialiser & operator>>(signed char &);
    PUnSerialiser & operator>>(short &);
    PUnSerialiser & operator>>(unsigned short &);
    PUnSerialiser & operator>>(int &);
    PUnSerialiser & operator>>(unsigned int &);
    PUnSerialiser & operator>>(long &);
    PUnSerialiser & operator>>(unsigned long &);
    PUnSerialiser & operator>>(float &);
    PUnSerialiser & operator>>(double &);
    PUnSerialiser & operator>>(long double &);
    PUnSerialiser & operator>>(char *);
    PUnSerialiser & operator>>(unsigned char *);
    PUnSerialiser & operator>>(signed char *);
    PUnSerialiser & operator>>(PObject &);
    /* Input the data from the un-serialiser object. When the operator is
       executed on a $H$PObject descendent then that objects
       $B$UnSerialise()$B$ function is called.
     */
};


class PStringArray;

PDECLARE_CLASS(PBinaryUnSerialiser, PUnSerialiser)
/* This un-serialiser class reconstructs each object using binary data. This
   gives the highest level data density at the expense of some portability and
   possibly the speed of execution.
   
   A table of classes must also be output
   to set the correspondence between the class codes used in the output and
   the class names that are required by the unserialiser to construct instances
   of those classes.
 */
  public:
    PBinaryUnSerialiser(
      istream & strm    // Stream to read serialised objects from.
    );
    // Create a binary un-serialiser.

    ~PBinaryUnSerialiser();
    // Destroy the un-serialiser and its class table.

    PUnSerialiser & operator>>(char &);
    PUnSerialiser & operator>>(unsigned char &);
    PUnSerialiser & operator>>(signed char &);
    PUnSerialiser & operator>>(short &);
    PUnSerialiser & operator>>(unsigned short &);
    PUnSerialiser & operator>>(int &);
    PUnSerialiser & operator>>(unsigned int &);
    PUnSerialiser & operator>>(long &);
    PUnSerialiser & operator>>(unsigned long &);
    PUnSerialiser & operator>>(float &);
    PUnSerialiser & operator>>(double &);
    PUnSerialiser & operator>>(long double &);
    PUnSerialiser & operator>>(char *);
    PUnSerialiser & operator>>(unsigned char *);
    PUnSerialiser & operator>>(signed char *);
    PUnSerialiser & operator>>(PObject &);
    /* Input the data from the un-serialiser object. When the operator is
       executed on a $H$PObject descendent then that objects
       $B$UnSerialise()$B$ function is called.
     */

  protected:
    PStringArray * classesUsed;
    // Class table used by the serialiser.
};


///////////////////////////////////////////////////////////////////////////////
// "Smart" pointers.

PDECLARE_CLASS(PSmartObject, PObject)
/* This is the base class for objects that use the $I$smart pointer$I$ system.
   In conjunction with the $H$PSmartPointer class, this class creates objects
   that can have the automatic deletion of the object instance when there are
   no more smart pointer instances pointing to it.

   A PSmartObject carries the reference count that the $H$PSmartPointer
   requires to determine if the pointer is needed any more and should be
   deleted.
 */

  public:
    PSmartObject() { referenceCount = 1; }
    /* Construct a new smart object, subject to a $H$PSmartPointer instance
       referencing it.
     */

  private:
    unsigned referenceCount;
    /* Count of number of instances of $H$PSmartPointer that currently
       reference the object instance.
     */


  friend class PSmartPointer;
};


PDECLARE_CLASS(PSmartPointer, PObject)
/* This is the class for pointers to objects that use the $I$smart pointer$I$
   system. In conjunction with the $H$PSmartObject class, this class references
   objects that can have the automatic deletion of the object instance when
   there are no more smart pointer instances pointing to it.

   A PSmartPointer carries the pointer to a $H$PSmartObject instance which
   contains a reference count. Assigning or copying instances of smart pointers
   will automatically increment and decrement the reference count. When the
   last instance that references a $H$PSmartObject instance is destroyed or
   overwritten, the $H$PSmartObject is deleted.

   A NULL value is possible for a smart pointer. It can be detected via the
   $B$IsNULL()$B$ function.
 */

  public:
    PSmartPointer(
      PSmartObject * obj = NULL   // Smart object to point to.
    ) { object = obj; }
    /* Create a new smart pointer instance and have it point to the specified
       $H$PSmartObject instance.
     */

    PSmartPointer(
      const PSmartPointer & ptr  // Smart pointer to make a copy of.
    );
    /* Create a new smart pointer and point it at the data pointed to by the
       $B$ptr$B$ parameter. The reference count for the object being pointed at
       is incremented.
     */

    virtual ~PSmartPointer();
    /* Destroy the smart pointer and decrement the reference count on the
       object being pointed to. If there are no more references then the
       object is deleted.
     */

    PSmartPointer & operator=(
      const PSmartPointer & ptr  // Smart pointer to assign.
    );
    /* Assign this pointer to the value specified in the $B$ptr$B$ parameter.

       The previous object being pointed to has its reference count
       decremented as this will no longer point to it. If there are no more
       references then the object is deleted.

       The new object being pointed to after the assignment has its reference
       count incremented.
     */

    virtual Comparison Compare(
      const PObject & obj   // Other smart pointer to compare against.
    ) const;
    /* Determine the relative rank of the pointers. This is identical to
       determining the relative rank of the integer values represented by the
       memory pointers.

       Returns: EqualTo if objects point to the same object instance, otherwise
                LessThan and Greater than may be returned depending on the
                relative values of the memory pointers.
     */

    BOOL IsNULL() const { return object == NULL; }
    /* Determine if the smart pointer has been set to point to an actual
       object instance.

       Returns: TRUE if the pointer is NULL.
     */

    PSmartObject * GetObject() const { return object; }
    /* Get the current value if the internal smart object pointer.

       Returns: pointer to object instance.
     */


  protected:
    PSmartObject * object;
    // Object the smart pointer points to.
};


/*$MACRO PDECLARE_POINTER_CLASS(cls, par, type)
   This macro is used to declare a smart pointer class. The class is not
   closed off allowing customisation of the new class being declared.

   The class $B$cls$$B is declared as a smart pointer, descended from the
   $B$par$B$ class, to the $B$type$B$ class.

   If no customisations are required use the $H$PDECLARE_POINTER_CLASS macro
   instead.

   The class declares the following functions:

      cls(type * obj);
        Constructor creating the smart pointer given the memory pointer.

      type * operator->() const;
        Access to the members of the smart object in the smart pointer.

      type & operator*() const;
        Access to the value of the smart object in the smart pointer.


   Note if this macro is used then the $H$PIMPLEMENT_POINTER macro must be
   used to implement some inline functions that the pointer class declares.
   The separate declaration and definition is sometimes required due to the
   order in which the PSmartPointer class and the PSmartObject class are
   declared. They may require a circular reference under some circumstances.
 */
#define PDECLARE_POINTER_CLASS(cls, par, type) \
  PDECLARE_CLASS(cls, par) \
    public: \
      cls(type * obj); \
      type * operator->() const; \
      type & operator*() const; \


/*$MACRO PDECLARE_POINTER(cls, par, type)
   This macro is used to declare a smart pointer. Unlike the
   $H$PDECLARE_POINTER_CLASS macro this closes off the class declaration.

   One additional constructor is created in this class declaration which
   will create a NULL smart pointer when no parameters are provided to the
   constructor.

   The class $B$cls$$B is declared as a smart pointer, descended from the
   $B$par$B$ class, to the $B$type$B$ class.

   Note if this macro is used then the $H$PIMPLEMENT_POINTER macro must be
   used to implement some inline functions that the pointer class declares.
   The separate declaration and definition is sometimes required due to the
   order in which the $H$PSmartPointer class and the $H$PSmartObject class are
   declared. They may require a circular reference under some circumstances.
 */
#define PDECLARE_POINTER(cls, par, type) \
  PDECLARE_POINTER_CLASS(cls, par, type) \
    public: \
      cls() { } \
  }

/*$MACRO PIMPLEMENT_POINTER(cls, par, type)
  This macro implements the optionally inline functions, using the $H$PINLINE
  macro.
 */
#define PIMPLEMENT_POINTER(cls, par, type) \
  PINLINE cls::cls(type * obj) : par(obj) { } \
  PINLINE type * cls::operator->() const \
    { return (type *)PAssertNULL(object); } \
  PINLINE type & cls::operator*() const \
    { return *(type *)PAssertNULL(object); } \

/*$MACRO PSMART_POINTER(cls, type)
   This macro is used to declare a smart pointer. Unlike the
   $H$PDECLARE_POINTER_CLASS macro this closes off the class declaration and
   assumes that it is descended directly off the PSmartPointer class. Also
   the member functions are implemented as inlines directly in the declaration.

   If the order in which the $H$PSmartPointer and the $B$PSmartObject classes
   are declared causes problems, use the separate $H$PDECLARE_POINTER_CLASS
   and $H$PIMPLEMENT_POINTER macros.
 */
#define PSMART_POINTER(cls, type) \
  PDECLARE_CLASS(cls, par) \
    public: \
      cls(type * obj = NULL) : par(obj) { } \
      type * operator->() const \
        { return (type *)PAssertNULL(object); } \
      type & operator*() const \
        { return *(type *)PAssertNULL(object); } \


///////////////////////////////////////////////////////////////////////////////
// General notification mechanism from one object to another

PDECLARE_CLASS(PNotifierFunction, PSmartObject)
/* This class is the $H$PSmartObject contents of the $H$PNotifier class.

   This is an abstract class for which a descendent is declared for every
   function that may be called. The $H$PDECLARE_NOTIFIER macro makes this
   declaration.

   The $H$PNotifier and PNotifierFunction classes build a completely type safe
   mechanism for calling arbitrary member functions on classes. The "pointer
   to a member function" capability built into C++ makes the assumption that
   the function name exists in an ancestor class. If you wish to call a member
   function name that does $U$not$U$ exist in any ancestor class, very type
   unsafe casting of the member functions must be made. Some compilers will
   even refuse to do it at all!

   To overcome this problem, as this mechanism is highly desirable for callback
   functions in the GUI part of the PWLib library, these classes and a macro
   are used to create all the classes and declarations to use polymorphism as
   the link between the caller, which has no knowledege of the function, and
   the receiver object and member function.

   This is most often used as the notification of actions being take by
   interactors in the PWLib library.
 */

  public:
    PNotifierFunction(
      void * obj    // Object instance that the function will be called on.
    ) { object = PAssertNULL(obj); }
    // Create a notification function instance.

    virtual void Call(
      PObject & notifier,  // Object that is making the notification.
      INT extra            // Extra information that may be passed to function.
    ) const = 0;
    /* Execute the call to the actual notification function on the object
       instance contained in this object.
     */

  protected:
    void * object;
    // Object instance to receive the notification function call.
};


PDECLARE_CLASS(PNotifier, PSmartPointer)
/* This class is the $H$PSmartPointer to the $H$PNotifierFunction class.

   The PNotifier and $H$PNotifierFunction classes build a completely type safe
   mechanism for calling arbitrary member functions on classes. The "pointer
   to a member function" capability built into C++ makes the assumption that
   the function name exists in an ancestor class. If you wish to call a member
   function name that does $U$not$U$ exist in any ancestor class, very type
   unsafe casting of the member functions must be made. Some compilers will
   even refuse to do it at all!

   To overcome this problem, as this mechanism is highly desirable for callback
   functions in the GUI part of the PWLib library, these classes and a macro
   are used to create all the classes and declarations to use polymorphism as
   the link between the caller, which has no knowledege of the function, and
   the receiver object and member function.

   This is most often used as the notification of actions being take by
   interactors in the PWLib library.
 */

  public:
    PNotifier(
      PNotifierFunction * func = NULL
    ) : PSmartPointer(func) { }
    // Create a new notification function smart pointer.

    virtual void operator()(
      PObject & notifier,  // Object that is making the notification.
      INT extra            // Extra information that may be passed to function.
    ) const { ((PNotifierFunction*)PAssertNULL(object))->Call(notifier, extra); }
    /* Execute the call to the actual notification function on the object
       instance contained in this object. This will make a polymorphic call to
       the function declared by the $H$PDECLARE_NOTIFIER macro which in turn
       calls the required function in the 
     */
};


/*$MACRO PDECLARE_NOTIFIER(notifier, notifiee, func)
  This macro declares the descendent class of $H$PNotifierFunction that will
  be used in instances of $H$PNotifier created by the $H$PCREATE_NOTIFIER or
  $H$PCREATE_NOTIFIER2 macros.

  The macro is expected to be used inside a class declaration. The class it
  declares will therefore be a nested class within the class being declared.
  The name of the new nested class is derived from the member function name
  which should guarentee the class names are unique.

  The $B$notifier$B$ parameter is the class of the function that will be
  calling the notification function. The $B$notifiee$B$ parameter is the class
  to which the called member function belongs. Finally the $B$func$B$ parameter
  is the name of the member function to be declared.

  This macro will also declare the member function itself. This will be:

      $F$void func(notifier & n, INT extra)$F$

  The implementation of the function is left for the user.
 */
#define PDECLARE_NOTIFIER(notifier, notifiee, func) \
  virtual void func(notifier & n, INT extra); \
  PCLASS func##_PNotifier : public PNotifierFunction { \
    public: \
      func##_PNotifier(notifiee * obj) : PNotifierFunction(obj) { } \
      virtual void Call(PObject & note, INT extra) const \
        { ((notifiee*)object)->func((notifier &)note, extra); } \
  }; \
  friend class func##_PNotifier

/*$MACRO PCREATE_NOTIFIER2(obj, func)
  This macro creates an instance of the particular $H$PNotifier class using
  the $B$func$B$ parameter as the member function to call.

  The $B$obj$B$ parameter is the instance to call the function against. If the
  instance to be called is the current instance, ie $B$obj$B$ is to $B$this$B$
  the the $H$PCREATE_NOTIFIER macro should be used.
 */
#define PCREATE_NOTIFIER2(obj, func) PNotifier(new func##_PNotifier(obj))

/*$MACRO PCREATE_NOTIFIER(func)
  This macro creates an instance of the particular $H$PNotifier class using
  the $B$func$B$ parameter as the member function to call.

  The $B$this$B$ object is used as the instance to call the function against.
  The $H$PCREATE_NOTIFIER2 macro may be used if the instance to be called is
  not the current object instance.
 */
#define PCREATE_NOTIFIER(func) PCREATE_NOTIFIER2(this, func)


// End Of File ///////////////////////////////////////////////////////////////

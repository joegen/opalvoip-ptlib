/*
 * $Id: object.h,v 1.2 1994/11/03 09:25:30 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: object.h,v $
 * Revision 1.2  1994/11/03 09:25:30  robertj
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
  PLogicError,
  POutOfMemory,
  PNullPointerReference,
  PInvalidArrayIndex,
  PInvalidArrayElement,
  PStackEmpty,
  PUnimplementedFunction,
  PInvalidParameter,
  POperatingSystemError,
  PFileNotOpen,
  PUnsupportedFeature,
  PInvalidWindow,
  PMaxStandardAssertMessage
};

#define PAssert(b, m) if(b);else PAssertFunc(__FILE__, __LINE__, (m))
#define PAssertOS(b) if(b);else PAssertFunc(__FILE__, __LINE__, POperatingSystemError)
#define PAssertNULL(p) ((&(p)&&(p)!=NULL)?(p):(PAssertFunc(__FILE__, __LINE__, PNullPointerReference), (p)))
#define PAssertAlways(m) PAssertFunc(__FILE__, __LINE__, (m))

void PAssertFunc(const char * file, int line, PStandardAssertMessage msg);
void PAssertFunc(const char * file, int line, const char * msg);

// Declaration for standard error output
extern ostream * PSTATIC PErrorStream;
#define PError (*PErrorStream)


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

#define PARRAYSIZE(array) ((PINDEX)(sizeof(array)/sizeof(array[0])))
#define PMIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#define PMAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define PABS(v) ((v) < 0 ? -(v) : (v))



///////////////////////////////////////////////////////////////////////////////
// The root of all evil ... umm classes

#if defined(PMEMORY_CHECK)

void * PMemoryCheckAllocate(size_t nSize,
                        const char * file, int line, const char * className);
void   PMemoryCheckDeallocate(void * ptr, const char * className);

#define PNEW_AND_DELETE_FUNCTIONS \
    void * operator new(size_t nSize, const char * file, int line) \
      { return PMemoryCheckAllocate(nSize, file, line, Class()); } \
    void * operator new(size_t nSize) \
      { return PMemoryCheckAllocate(nSize, NULL, 0, Class()); } \
    void operator delete(void * ptr) \
      { PMemoryCheckDeallocate(ptr, Class()); }

#define PNEW new(__FILE__, __LINE__)

#else // defined(PMEMORY_CHECK)

#define PNEW_AND_DELETE_FUNCTIONS
#define PNEW new

#endif // defined(PMEMORY_CHECK)


inline BOOL IsDescendant(const char *)
  { return FALSE; }


#define PCLASSINFO(cls, par) \
  public: \
    static const char * Class() \
      { return #cls; } \
    virtual const char * GetClass() const \
      { return cls::Class(); } \
    virtual BOOL IsClass(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0; } \
    virtual BOOL IsDescendant(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0 || par::IsDescendant(clsName); } \
    virtual int CompareObjectMemoryDirect(const PObject & obj) const \
      { return memcmp(this, &obj, sizeof(cls)); } \
    PNEW_AND_DELETE_FUNCTIONS

#define PDECLARE_CLASS(cls, par) PCLASS cls : public par { PCLASSINFO(cls, par)

class PSerialiser;


PCLASS PObject {
#ifdef _MSC_VER
#pragma warning(disable:4003)  // disable warning about insufficent parameters
#endif
  PCLASSINFO(PObject,)
#ifdef _MSC_VER
#pragma warning(default:4003)
#endif

  public:
    virtual ~PObject() { }

    virtual PObject * Clone() const;

    enum Comparison {
      LessThan = -1,
      EqualTo = 0,
      GreaterThan = 1
    };
    virtual Comparison Compare(const PObject & obj) const;
    BOOL operator==(const PObject & obj) const
      { return Compare(obj) == EqualTo; }
    BOOL PObject::operator!=(const PObject & obj) const
      { return Compare(obj) != EqualTo; }
    BOOL PObject::operator<(const PObject & obj) const
      { return Compare(obj) == LessThan; }
    BOOL PObject::operator>(const PObject & obj) const
      { return Compare(obj) == GreaterThan; }
    BOOL PObject::operator<=(const PObject & obj) const
      { return Compare(obj) != GreaterThan; }
    BOOL PObject::operator>=(const PObject & obj) const
      { return Compare(obj) != LessThan; }

    virtual ostream & PrintOn(ostream &strm) const;
    virtual istream & ReadFrom(istream &strm);

    virtual PINDEX PreSerialise(PSerialiser & strm);
    virtual PSerialiser & Serialise(PSerialiser & serial);

    virtual PINDEX HashFunction() const;

#if !defined(PMEMORY_CHECK)

    void * operator new(size_t nSize)
      { void*obj=malloc(nSize); PAssert(obj!=NULL,POutOfMemory); return obj; }
    void operator delete(void * ptr)
      { free(ptr); }

#endif
};

inline ostream & operator<<(ostream &strm, const PObject & obj)
  { return obj.PrintOn(strm); }

inline istream & operator>>(istream &strm, PObject & obj)
  { return obj.ReadFrom(strm); }



///////////////////////////////////////////////////////////////////////////////
// Serialisation

class PUnSerialiser;

typedef PObject * (*PSerialCreatorFunction)(PUnSerialiser * serial);

class PSerialRegistration {
  public:
    PSerialRegistration(const char * clsNam, PSerialCreatorFunction func);
      // Create a serialiser class registration. This is unversally called by
      // static member variables in the PIMPLEMENT_SERIAL() macro.

    static PSerialCreatorFunction GetCreator(const char * clsNam);
      // Return the creator function for the class name specified.

  protected:
    const char * className;
      // This serialiser registrations class

    PSerialCreatorFunction creator;
      // This serialiser registrations creator function - the function that
      // will make a new object of the classes type and construct it with an
      // instance of the PSerialiser class.

    PSerialRegistration * clash;
      // Pointer to next registration when a hash clash occurs.

    enum { HashTableSize = 41 };
    static PINDEX HashFunction(const char * className);
    static PSerialRegistration * creatorHashTable[HashTableSize];
      // A static dictionary of class names to creator functions
};


PDECLARE_CLASS(PSerialiser, PObject)
  public:
    PSerialiser(ostream & strm);

    void Register(const char * className);

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

  protected:
    ostream & stream;
};


PDECLARE_CLASS(PUnSerialiser, PObject)
  public:
    PUnSerialiser(istream & strm);

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

  protected:
    istream & stream;
};


#define PDECLARE_SERIAL(cls) \
  public: \
    virtual PINDEX PreSerialise(PSerialiser & strm); \
    virtual PSerialiser & Serialise(PSerialiser & serial); \
  protected: \
    cls(PSerialiser & serial); \
    static cls * UnSerialise(PSerialiser * serial); \
  private: \
    PINDEX serialisedLength; \
    static PSerialRegistration pRegisterSerial; \

#define PIMPLEMENT_SERIAL(cls) \
  cls * cls::UnSerialise(PSerialiser * serial) { return new cls(serial); } \
  PSerialRegistration cls::pRegisterSerial(cls::Class(), cls::UnSerialise); \
    

PDECLARE_CLASS(PTextSerialiser, PSerialiser)
  public:
    PTextSerialiser(ostream & strm, PObject & data);

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
};


class PSortedStringList;

PDECLARE_CLASS(PBinarySerialiser, PSerialiser)
  public:
    PBinarySerialiser(ostream & strm, PObject & data);
    ~PBinarySerialiser();

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

  protected:
    PSortedStringList * classesUsed;
};


PDECLARE_CLASS(PTextUnSerialiser, PUnSerialiser)
  public:
    PTextUnSerialiser(istream & strm);

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
};


class PStringArray;

PDECLARE_CLASS(PBinaryUnSerialiser, PUnSerialiser)
  public:
    PBinaryUnSerialiser(istream & strm);
    ~PBinaryUnSerialiser();

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

  protected:
    PStringArray * classesUsed;
};


///////////////////////////////////////////////////////////////////////////////
// General reference counting wrapper. This object "contains" another object

PDECLARE_CLASS(PWrapper, PObject)
  public:
    PWrapper(PObject * obj = NULL);
    PWrapper(const PWrapper & wrap);
    PWrapper & operator=(const PWrapper & wrap);
    virtual ~PWrapper();

    virtual Comparison Compare(const PObject & obj) const;

    BOOL IsNULL() const { return object == NULL; }
    PObject * GetObject() const { return object; }

  protected:
    PObject  * object;
    unsigned * referenceCount;
};

#ifdef PHAS_TEMPLATES

template <class cls> class Wrap : public Wrapper {
  public:
    Wrap(cls * obj) : Wrapper(obj) { }
    cls * operator->() const { return (cls *)PAssertNULL(object); }
    cls & operator*() const { return *(cls *)PAssertNULL(object); }
};

#define PDECLARE_WRAPPER(cls, type) typedef PWrap<type> cls

#else

#define PDECLARE_WRAPPER(cls, type) \
  PDECLARE_CLASS(cls, PWrapper) \
    public: \
      cls(type * obj) : PWrapper(obj) { } \
      cls * operator->() const { return (cls *)PAssertNULL(object); } \
      cls & operator*() const { return *(cls *)PAssertNULL(object); } \
  }

#endif


///////////////////////////////////////////////////////////////////////////////
// General notification mechanism from one object to another

PDECLARE_CLASS(PNotifierFunction, PObject)
  public:
    PNotifierFunction(void * obj) { object = obj; }
    virtual void Call(PObject & notifier, PInteger extra) const = 0;
  protected:
    void * object;
};


PDECLARE_CLASS(PNotifier, PWrapper)
  public:
    PNotifier(PNotifierFunction * func = NULL)
      : PWrapper(func) { }
    virtual void operator()(PObject & notifier, PInteger extra) const
      { ((PNotifierFunction*)PAssertNULL(object))->Call(notifier, extra); }
};


#define PDECLARE_NOTIFIER(notifier, notifiee, func) \
  PDECLARE_CLASS(func##_PNotifier, PNotifierFunction) \
    public: \
      func##_PNotifier(notifiee * obj) : PNotifierFunction(obj) { } \
      virtual void Call(PObject & note, PInteger extra) const \
        { ((notifiee*)PAssertNULL(object))->func((notifier &)note, extra);  } \
  }; \
  friend class func##_PNotifier; \
  void func(notifier & n, PInteger extra)

#define PCREATE_NOTIFIER2(obj, func) PNotifier(new func##_PNotifier(obj))

#define PCREATE_NOTIFIER(func) PCREATE_NOTIFIER2(this, func)


// End Of File ///////////////////////////////////////////////////////////////

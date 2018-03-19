/*
 * main.cxx
 *
 * PWLib application source file for map dictionary compparison.
 *
 * Main program entry point.
 *
 * Copyright 2009 Derek J Smithies
 *
 *  You can read the comments to this program in the file main.h
 *
 *  In linux, you can get nice html docs by doing "make docs"
 *
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/random.h>

#include <vector>
#include <map>

using namespace std;

#define NEW_DICTIONARY_ITERATOR 1

/*! \mainpage  map_dictionary 

The purpose of this program is to find out which is faster:
\li STL based map
\li PTLib based dictionary

By default, this program creates the 200 instances of the structure Element.
Each instance of the Element class is given a random (and hopefully) unique key.
The key can exist either as a string, or as a number.

Each instance of the class Element has a key to the next instance.

After creation of an instance of the class Element, it is put into a
map (or PDictionary), keyed of the random key.

Then, the code looks through the map (or PDictionary) for each created
instance. Since the code knows the first key, it can get the key of
the next element via the current element. In this way, the code is
made to make N acceses to seemingly random parts of the map (or
PDictionary).

Results so far:

Running 100000 lookups, 10000 iterates, over map/dictionary with 20 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:00.000  0:00.964  0:00.515  0:00.000
String Dictionary     0:00.000  0:00.140  0:00.666  0:00.000
Integer Map           0:00.000  0:00.603  0:00.514  0:00.000
Integer Dictionary    0:00.000  0:00.121  0:00.364  0:00.000

Running 50000 lookups, 5000 iterates, over map/dictionary with 100 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:00.002  0:00.647  0:01.246  0:00.004
String Dictionary     0:00.000  0:00.095  0:01.061  0:00.000
Integer Map           0:00.002  0:00.359  0:01.252  0:00.002
Integer Dictionary    0:00.001  0:00.061  0:00.816  0:00.001

Running 50000 lookups, 2000 iterates, over map/dictionary with 500 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:00.018  0:00.797  0:02.507  0:00.026
String Dictionary     0:00.009  0:00.065  0:01.843  0:00.014
Integer Map           0:00.018  0:00.430  0:02.505  0:00.017
Integer Dictionary    0:00.021  0:00.284  0:01.566  0:00.015

Running 20000 lookups, 500 iterates, over map/dictionary with 2000 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:00.054  0:00.422  0:02.525  0:00.182
String Dictionary     0:00.122  0:00.079  0:01.786  0:00.168
Integer Map           0:00.137  0:00.180  0:02.574  0:00.099
Integer Dictionary    0:00.249  0:00.314  0:01.542  0:00.166

Running 10000 lookups, 100 iterates, over map/dictionary with 10000 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:00.299  0:00.244  0:02.597  0:01.621
String Dictionary     0:00.287  0:00.280  0:01.853  0:02.135
Integer Map           0:00.158  0:00.099  0:02.573  0:01.166
Integer Dictionary    0:01.125  0:00.700  0:01.612  0:01.764

Running 5000 lookups, 50 iterates, over map/dictionary with 50000 elements.
Structure               Insert    Lookup   Iterate    Remove
String Map            0:01.691  0:00.115  0:06.581  0:08.311
String Dictionary     0:06.516  0:00.544  0:04.878  0:10.517
Integer Map           0:00.921  0:00.050  0:06.562  0:06.710
Integer Dictionary    0:22.441  0:01.899  0:04.348  0:10.303

So, depending on which parameter is most importaint, there are different
thresholds. So how many elements are required in a dictionary before you
should switch to a map:

Structure       Insert  Lookup  Iterate  Remove
String Keys      5,000  10,000  Never!   5,000
Integer Keys     1,000  10.000  Never!   1,000

*/

/**This class is the core of the thing. It is placed in the structure
   (map or dictionary) being tested. There are hundreds/thousands of
   these created. Each instance holds the key to the next
   element. Since the key to the next element is random, any code that
   follows this ends up walking over the entire map/directory in a
   random fashion */
class Element : public PObject
{
  PCLASSINFO(Element, PObject);

  /*Sopme data held in this class */
  PString m_stuff;
  PINDEX  m_moreStuff;
};


#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif
void DoNothing(const PString & key, const Element & data)
{
}


void DoNothing(int key, const Element & data)
{
}
#ifdef _MSC_VER
#pragma warning(default:4100)
#endif


std::vector<PString> StringKeys;
std::vector<PINDEX> IntKeys;
std::vector<Element> DataElements;

class Tester
{
  public:
    virtual ~Tester() { }

    virtual const char * GetName() const = 0;
    virtual void TestInsert() const = 0;
    virtual void TestLookup() const = 0;
    virtual void TestIterate() const = 0;
    virtual void TestRemove() const = 0;
};


class StringMap : public Tester
{
    typedef std::map<PString, Element *> Type;
    mutable Type data;

  public:
    virtual const char * GetName() const { return "String Map"; }

    virtual void TestInsert() const
    {
      for (size_t i = 0; i < StringKeys.size(); i++)
        data.insert(Type::value_type(StringKeys[i], &DataElements[i]));
    }

    virtual void TestLookup() const
    {
      data.find(StringKeys[StringKeys.size()/2]);
    }

    virtual void TestIterate() const
    {
      for (Type::iterator it = data.begin(); it != data.end(); ++it)
        DoNothing(it->first, *it->second);
    }

    virtual void TestRemove() const
    {
      for (size_t i = 0; i < StringKeys.size(); i++)
        data.erase(StringKeys[i]);
    }
};


class StringDict : public Tester
{
    typedef PDictionary<PString, Element> Type;
    mutable Type data;

  public:
    StringDict() { data.DisallowDeleteObjects(); }

    virtual const char * GetName() const { return "String Dictionary"; }

    virtual void TestInsert() const
    {
      for (size_t i = 0; i < StringKeys.size(); i++)
        data.Insert(StringKeys[i], &DataElements[i]);
    }

    virtual void TestLookup() const
    {
      data.GetAt(StringKeys[StringKeys.size()/2]);
    }

    virtual void TestIterate() const
    {
#if NEW_DICTIONARY_ITERATOR
      for (Type::iterator it = data.begin(); it != data.end(); ++it)
        DoNothing(it->first, it->second);
#else
      for (PINDEX i = 0; i < data.GetSize(); ++i)
        DoNothing(data.GetKeyAt(i), data.GetDataAt(i));
#endif
    }

    virtual void TestRemove() const
    {
      for (size_t i = 0; i < StringKeys.size(); i++)
        data.RemoveAt(StringKeys[i]);
    }
};


class IntMap : public Tester
{
    typedef std::map<int, Element *> Type;
    mutable Type data;
  public:
    virtual const char * GetName() const { return "Integer Map"; }

    virtual void TestInsert() const
    {
      for (size_t i = 0; i < IntKeys.size(); i++)
        data.insert(Type::value_type(IntKeys[i], &DataElements[i]));
    }

    virtual void TestLookup() const
    {
      data.find(IntKeys[IntKeys.size()/2]);
    }

    virtual void TestIterate() const
    {
      for (Type::iterator it = data.begin(); it != data.end(); ++it)
        DoNothing(it->first, *it->second);
    }

    virtual void TestRemove() const
    {
      for (size_t i = 0; i < IntKeys.size(); i++)
        data.erase(IntKeys[i]);
    }
};


class IntDict : public Tester
{
    typedef PDictionary<POrdinalKey, Element> Type;
    mutable Type data;
  public:
    IntDict() { data.DisallowDeleteObjects(); }

    virtual const char * GetName() const { return "Integer Dictionary"; }

    virtual void TestInsert() const
    {
      for (size_t i = 0; i < StringKeys.size(); i++)
        data.Insert(POrdinalKey(IntKeys[i]), &DataElements[i]);
    }

    virtual void TestLookup() const
    {
      data.GetAt(IntKeys[IntKeys.size()/2]);
    }

    virtual void TestIterate() const
    {
#if NEW_DICTIONARY_ITERATOR
      for (Type::iterator it = data.begin(); it != data.end(); ++it)
        DoNothing(it->first, it->second);
#else
      for (PINDEX i = 0; i < data.GetSize(); ++i)
        DoNothing(data.GetKeyAt(i), data.GetDataAt(i));
#endif
    }

    virtual void TestRemove() const
    {
      for (size_t i = 0; i < IntKeys.size(); i++)
        data.RemoveAt(IntKeys[i]);
    }
};


/**This is where all the activity happens. This class is launched on
   program startup, and does timing runs on the map and dictionaries
   to see which is faster */
class MapDictionary : public PProcess
{
  PCLASSINFO(MapDictionary, PProcess)

  public:
    MapDictionary()
      : PProcess("Derek Smithies code factory", "MapDictionary", 1, 0, AlphaCode, 1)
    {
    }

    void Main();
    void TestAll();
    void Test(const Tester & tester);

  protected:
    PINDEX m_size;
    PINDEX m_lookups;
    PINDEX m_iterates;
};


PCREATE_PROCESS(MapDictionary);


void MapDictionary::Main()
{
  PArgList & args = GetArguments();
  args.Parse("l-lookups:"
             "i-iterates:"
	     "s-size:"
             "-preset."
	     "h-help."
#if PTRACING
             "o-output:"
             "t-trace."
#endif
	     "v-version."
	     );

  if (args.HasOption('h')) {
    cout << "usage: " << GetFile().GetTitle() << " [ options ]\n"
         << "     -l --lookups #  : count of lookup to run over the map/dicts (10000)\n"
         << "     -i --iterates # : count of iterates to run over the map/dicts (1000)\n"
	 << "     -s --size  #    : number of elements to pu in map/dict (200)\n"
	 << "     -h --help       : Get this help message\n"
	 << "     -v --version    : Get version information\n"
#if PTRACING
         << "  -t --trace         : Enable trace, use multiple times for more detail\n"
         << "  -o --output        : File for trace output, default is stderr\n"
#endif
         << endl;
    return;
  }

  if (args.HasOption('v')) {
    cout << endl
         << "Product Name: " <<  (const char *)GetName() << endl
         << "Manufacturer: " <<  (const char *)GetManufacturer() << endl
         << "Version     : " <<  (const char *)GetVersion(true) << endl
         << "System      : " <<  (const char *)GetOSName() << '-'
         <<  (const char *)GetOSHardware() << ' '
         <<  (const char *)GetOSVersion() << endl
         << endl;
    return;
  }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  if (args.HasOption("preset")) {
    m_size = 20;    m_lookups = 100000; m_iterates = 10000; TestAll();
    m_size = 100;   m_lookups = 50000;  m_iterates = 5000;  TestAll();
    m_size = 500;   m_lookups = 50000;  m_iterates = 2000;  TestAll();
    m_size = 2000;  m_lookups = 20000;  m_iterates = 500;   TestAll();
    m_size = 10000; m_lookups = 10000;  m_iterates = 100;   TestAll();
    m_size = 50000; m_lookups = 5000;   m_iterates = 50;    TestAll();
    return;
  }

  if ((m_size = args.GetOptionString('s', "200").AsInteger()) <= 0) {
    cerr << "Illegal number of size\n";
    return;
  }

  if ((m_lookups = args.GetOptionString('l', "10000").AsInteger()) <= 0) {
    cerr << "Illegal number of lookups\n";
    return;
  }

  if ((m_iterates = args.GetOptionString('i', "1000").AsInteger()) <= 0) {
    cerr << "Illegal number of iterates\n";
    return;
  }

  TestAll();
}


void MapDictionary::TestAll()
{
  cout << "Running " << m_lookups << " lookups, " << m_iterates << " iterates, "
          "over map/dictionary with " << m_size << " elements." << endl;

  // Initialise data outside of timing, we are not checking this part!
  StringKeys.resize(m_size);
  IntKeys.resize(m_size);
  DataElements.resize(m_size);

  PRandom random;
  for (PINDEX i = 0; i < m_size; i++)
    StringKeys[i].sprintf("%08x", IntKeys[i] = random.Generate());

  // Now test 'em
  cout << setw(20) << left << "Structure" << right
       << setw(10) << "Insert"
       << setw(10) << "Lookup"
       << setw(10) << "Iterate"
       << setw(10) << "Remove"
       << endl;
  Test(StringMap());
  Test(StringDict());
  Test(IntMap());
  Test(IntDict());
  cout << endl;
}


void MapDictionary::Test(const Tester & tester)
{
  PTime a;
  tester.TestInsert();

  PTime b;
  for (PINDEX i = 0; i < m_lookups; i++)
    tester.TestLookup();

  PTime c;
  for (PINDEX i = 0; i < m_iterates; i++)
    tester.TestIterate();

  PTime d;
  tester.TestRemove();

  PTime e;

  cout << setw(20) << left << tester.GetName() << right
       << setw(10) << (b-a)
       << setw(10) << (c-b)
       << setw(10) << (d-c)
       << setw(10) << (e-d)
       << endl;
}


// End of File ///////////////////////////////////////////////////////////////

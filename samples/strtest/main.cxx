#include <ptlib.h>
#include <string>

////////////////////////////////////////////////
//
// test #1 - string concurrency test
//

#define SPECIALNAME     "openH323"
#define	COUNT_MAX	2000000

BOOL finishFlag;

template <class S>
struct StringConv {
  static const char * ToConstCharStar(const S &) { return NULL; }
};

template <class S, class C>
class StringHolder
{
  public:
    StringHolder(const S & _str)
      : str(_str) { }
    S GetString() const { return str; }
    S str;

    void TestString(int count, const char * label)
    {
      if (finishFlag)
        return;

      S s = GetString();
      const char * ptr = C::ToConstCharStar(s);
      //const char * ptr = s.c_str();
      char buffer[20];
      strncpy(buffer, ptr, 20);

      if (strcmp((const char *)buffer, SPECIALNAME)) {
        finishFlag = TRUE;
        cerr << "String compare failed at " << count << " in " << label << " thread" << endl;
        return;
      }
      if (count % 10000 == 0)
        cout << "tested " << count << " in " << label << " thread" << endl;
    }

    class TestThread : public PThread
    {
      PCLASSINFO(TestThread, PThread);
      public:
        TestThread(StringHolder & _holder) 
        : PThread(1000,NoAutoDeleteThread), holder(_holder)
        { Resume(); }

        void Main() 
        { int count = 0; while (!finishFlag && count < COUNT_MAX) holder.TestString(count++, "sub"); }

	StringHolder & holder;
    };

    PThread * StartThread()
    {
      return new TestThread(*this);
    }

};

struct PStringConv : public StringConv<PString> {
  static const char * ToConstCharStar(const PString & s) { return (const char *)s; }
};

struct StdStringConv : public StringConv<std::string> {
  static const char * ToConstCharStar(const std::string & s) { return s.c_str(); }
};

void Test1()
{
  /////////////////////
  //
  // test #1 - string concurrency test
  //

  // uncomment this to test std::string
  //StringHolder<std::string, StdStringConv> holder(SPECIALNAME);
  
  // uncomment this to test PString
  StringHolder<PString, PStringConv> holder(SPECIALNAME);

  PThread * thread = holder.StartThread();
  finishFlag = FALSE;
  int count = 0;
  while (!finishFlag && count < COUNT_MAX) 
    holder.TestString(count++, "main");
  finishFlag = TRUE;
  thread->WaitForTermination(9000);
  cerr << "finish" << endl;
}

////////////////////////////////////////////////
//
// test #2 - SIP URL test
//

#include <ptclib/url.h>

void Test2()
{
  const char * urls[] = {
    "sip:12345678@voxgratia.org",
    "sip:12345678:5060@voxgratia.org",
    "sip:12345678:1234@voxgratia.org",
    NULL
  };

  const char ** url = urls;
  while (*url != NULL) {
    PURL sipURL(*url);
    cout << "SIP URL : original = " << *url << ", URL = " << sipURL << endl;
    ++url;
  }
}

////////////////////////////////////////////////
//
// main
//

class StringTest : public PProcess
{
  PCLASSINFO(StringTest, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(StringTest);

void StringTest::Main()
{
  PArgList & args = GetArguments();

  if (args.GetCount() < 1) {
    cout << "usage: strtest num [args...]\n"
         << "\n"
         << "where num is one of the following tests\n"
         << "\n"
         << "    1     string concurrency test\n"
         << "    2     SIP URL test\n"
         << endl;
    return;
  }

  switch (args[0].AsInteger()) {
    case 1:   Test1(); return;
    case 2:   Test2(); return;
    default:  break;
  }

  cout << "error: unknown test number " << args[0] << endl;
}

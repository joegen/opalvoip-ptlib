//
// hello.cxx
//
// Equivalence Pty. Ltd.
//

#include <ptlib.h>
#include <ptlib/pprocess.h>



class Hello : public PProcess
{
  PCLASSINFO(Hello, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(Hello)

template <class String, typename Char>
void TestString(const Char * testStr)
{
  String string1;
  String string2(testStr[0]);
  String string3(testStr);
  String string4(testStr, 5);

  //const typename Char * cstr1 = string1;

  String istring1((short)1);
  String istring2((unsigned short)2);
  String istring3((int)3);
  String istring4((unsigned int)4);
  String istring5((long)5);
  String istring6((unsigned long)6);
  String istring7((PInt64)7);
  String istring8((PUInt64)8);

  String fstring1(String::Signed, 123);

  String fstring2; fstring2.sprintf("%i", 456);
}

void Hello::Main()
{
  cout << "Hello world!\n\n"
          "From " << GetOSClass() << ' ' << GetOSName() << " (" << GetOSVersion() << ")"
          " on " << GetOSHardware() << ", PTLib version " << GetLibVersion() << endl;

  TestString<XPString, char>("hello world");
  //TestString<XPWideString, wchar_t>(L"Hello World");

  //TestString<XPCaselessString, char>("Hello world");
  //TestString<XPCaselessWideString, wchar_t>(L"Hello World");
}

// End of hello.cxx

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include <ptclib/pjson.h>

class JSONTest : public PProcess
{
  PCLASSINFO(JSONTest, PProcess);
 public:
  JSONTest();
  void Main();
};

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define BUILD_TYPE    AlphaCode
#define BUILD_NUMBER 1 

PCREATE_PROCESS(JSONTest);


JSONTest::JSONTest()
  : PProcess("JSON Test Program", "JSONTest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}

void JSONTest::Main()
{
  PJSON object1 = PJSON::Parse("{\n'test' : 'hello world'\n}");
  cout << object1 << endl;

  PJSON object2 = PJSON::Object();
  object2.Insert("hello",  PJSON::String("world"));
  PJSON array = PJSON::Array();
  array.Append(PJSON::String("was"));
  array.Append(PJSON::String("here"));
  array.Append(PJSON::String("in"));
  array.Append(PJSON::Int(42));
  object2.Insert("kilroy", array);

  cout << object2 << endl;
}


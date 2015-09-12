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

PCREATE_PROCESS(JSONTest);


JSONTest::JSONTest()
  : PProcess("JSON Test Program", "JSONTest", 1, 0, AlphaCode, 0)
{
}

void JSONTest::Main()
{
  PArgList & args = GetArguments();
  if (args.GetCount() > 0) {
    PJSON json;
    if (args[0] == "-")
      cin >> json;
    else
      json.FromString(args[0]);
    if (json.IsValid())
      cout << json << endl;
    else
      cout << "Could not parse JSON\n";
    return;
  }

  PJSON json1(PJSON::e_Object);
  PJSON::Object & obj1 = json1.GetObject();
  obj1.SetString("one", "first");
  obj1.SetNumber("two", 2);
  obj1.SetBoolean("three", true);
  obj1.Set("four", PJSON::e_Array);
  PJSON::Array & arr1 = *obj1.Get<PJSON::Array>("four");
  arr1.AppendString("was");
  arr1.AppendString("here");
  arr1.AppendString("in");
  arr1.AppendNumber(42);
  arr1.AppendBoolean(true);
  obj1.Set("five", PJSON::e_Null);

  cout << json1 << endl;

  PJSON json2("{\n\"test\" : \"hello world\",\n\"field2\" : null }");
  cout << json2 << endl;

  PJSON json3("\n[ \"test\", \"hello world\"]");
  cout << json3 << endl;

  PJSON json4(json1.AsString());
  cout << json4 << endl;
}


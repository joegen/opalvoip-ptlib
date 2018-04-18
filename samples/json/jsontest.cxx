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
      json.FromString(args.GetParameters().ToString());
    if (json.IsValid())
      cout << "\n\n\nParsed JSON:\n" << setw(2) << json << endl;
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

  cout << "Test 1\n" << json1 << endl;

  PJSON json2("{\n\"test\" : \"hello world\",\n\"field2\" : null }");
  cout << "Test 2\n" << json2 << endl;

  PJSON json3("\n[ \"test\", \"hello world\"]");
  cout << "Test 3\n" << json3 << endl;

  PJSON json4(json1.AsString());
  cout << "Test 4\n" << json4 << endl;

  PJSON json5(json4);
  cout << "Test 5\n" << json5 << endl;

  json5 = json3;
  cout << "Test 6\n" << json5 << endl;

  cout << "Test 1 pretty A\n" << setw(4) << json1 << "\n"
          "Test 1 pretty B\n" << setprecision(4) << json1 << "\n"
          "Test 1 pretty C\n" << setprecision(3) << setw(6) << json1
       << endl;

#if P_SSL
  PJWT jwt;
  jwt.SetIssuer("Vox Lucida, Pty. Ltd.");
  jwt.SetIssuedAt(1516239022);
  jwt.SetPrivate("name", "Fred Nurk");
  PString enc = jwt.Encode("secret");
  bool good = enc == "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE1MTYyMzkwMjIsImlzcyI6IlZveCBMdWNpZGEsIFB0eS4gTHRkLiIsIm5hbWUiOiJGcmVkIE51cmsifQ.FVEXKAMmqRiOIozsmAZcSoGsN1GbPl4iZ_wCHRGjMQU";
  cout << "JWT: " << (good ? "(good) " : "(bad) ") << enc << endl;

  PJWT dec(enc, "secret");
  if (dec.IsValid())
    cout << "Decoded:\n" << dec << endl;
  else
    cout << "Invalid JWT\n";
#endif // P_SSL
}


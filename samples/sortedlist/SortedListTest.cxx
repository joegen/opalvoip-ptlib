
#include <ptlib.h>
#include "SortedListTest.h"
#include <ptclib/random.h>


PCREATE_PROCESS(SortedListTest);

class Fred : public PObject
{
    char m_character;
  public:
    Fred(char c)
      : m_character(c)
    {
    }

    Comparison Compare(const PObject & obj) const
    {
      const Fred & other = dynamic_cast<const Fred &>(obj);
      if (m_character < other.m_character)
        return GreaterThan;
      if (m_character > other.m_character)
        return LessThan;
      return EqualTo;
    }
};


PMutex coutMutex;

SortedListTest::SortedListTest()
  : PProcess("Reitek S.p.A.", "SortedListTest", 0, 0, BetaCode, 0)
{
}


void SortedListTest::Main()
{
#ifdef _MSC_VER
  // Tests for Visual Studio debugger autoexp.dat
  {
    PString str("fred");
    PCharArray chars("fred", 4);
    PBYTEArray bytes((const BYTE *)"fred", 4);
    PShortArray shorts((const short *)L"fred", 4);
    PWORDArray words((const WORD *)L"fred", 4);
    PArray<Fred> a_fred; a_fred.Append(new Fred('f')); a_fred.Append(new Fred('r')); a_fred.Append(new Fred('e')); a_fred.Append(new Fred('d'));
    PStringArray a_strings; a_strings.AppendString('f'); a_strings.AppendString('r'); a_strings.AppendString('e'); a_strings.AppendString('d');
    PList<Fred> l_fred; l_fred.Append(new Fred('f')); l_fred.Append(new Fred('r')); l_fred.Append(new Fred('e')); l_fred.Append(new Fred('d'));
    PStringList l_strings; l_strings.AppendString('f'); l_strings.AppendString('r'); l_strings.AppendString('e'); l_strings.AppendString('d');
    PSortedList<Fred> sl_fred; sl_fred.Append(new Fred('f')); sl_fred.Append(new Fred('r')); sl_fred.Append(new Fred('e')); sl_fred.Append(new Fred('d'));
    PSortedStringList ls_strings; ls_strings.AppendString('f'); ls_strings.AppendString('r'); ls_strings.AppendString('e'); ls_strings.AppendString('d');
    PSet<Fred> s_fred; s_fred.Append(new Fred('f')); s_fred.Append(new Fred('r')); s_fred.Append(new Fred('e')); s_fred.Append(new Fred('d'));
    PStringSet s_strings; s_strings+='f'; s_strings+='r'; s_strings+='e'; s_strings+='d';
    PStringToString ssd_strings; ssd_strings.SetAt('f','F'); ssd_strings.SetAt('r','R'); ssd_strings.SetAt('e','E'); ssd_strings.SetAt('d','D');

    cout << endl;
  }
#endif

  {
    PSortedStringList ss;
    PAssert(ss.begin() == ss.end(), "Bad PSortedStringList implemetation");
    ss.AppendString("fred");
    ss.AppendString("nurk");
    ss.AppendString("rocky");
    ss.AppendString("bullwinkle");
    ss.AppendString("boris");
    ss.AppendString("natasha");

    cout << "front()=\"" << ss.front() << "\", back()=\"" << ss.back() << "\"\n"
            "Forwards, iteration:\n";
    for (PSortedStringList::iterator it = ss.begin(); it != ss.end(); ++it)
      cout << "  \"" << *it << "\"\n";

    cout << "Reverse iteration:\n";
    for (PSortedStringList::iterator it = ss.rbegin(); it != ss.rend(); --it)
      cout << "  \"" << *it << "\"\n";
    cout << endl;

    PSortedStringList::iterator found = ss.find("fred");
    PAssert(found != ss.end() && *found == "fred", "Bad PSortedStringList implemetation");
    ss.erase(found);
  }

  for (PINDEX i = 0; i < 15; i++) {
    if (i < 10)
      new DoSomeThing1(i);
    else
      new DoSomeThing2(i);
  }

  Suspend();
}


DoSomeThing1::DoSomeThing1(PINDEX _index)
  : PThread(1000, AutoDeleteThread, NormalPriority, psprintf("DoSomeThing1 %u", _index)), index(_index)
{
  Resume();
}


void DoSomeThing1::Main()
{
  list.AllowDeleteObjects();

  PRandom rand(PRandom::Number());

  PINDEX i;

  for (i = 0; i < 5000; i++) {
    PString * p = new PString(rand.Generate());
    list.Append(p);
//    coutMutex.Wait();
//    cout << GetThreadName() << ": Added " << *p << " element to sorted list" << endl;
//    coutMutex.Signal();
  }

  for (;;) {

    PINDEX remove = rand.Generate() % (list.GetSize() + 1);
    for (i = 0; i < remove; i++) {
      PINDEX index = rand.Generate() % list.GetSize();
      coutMutex.Wait();
      cout << GetThreadName() << ": Removing element " << list[index] << " at index position " << index << endl;
      coutMutex.Signal();
      if (index%2)
        list.Remove(&list[index]);
      else
        list.RemoveAt(index);
    }

    PINDEX add = rand.Generate() % 1000 + 300;
    for (i = 0; i < add; i++) {
      PString * p = new PString(rand.Generate());
      coutMutex.Wait();
      cout << GetThreadName() << ": Adding element " << *p << "to sorted list" << endl;
      coutMutex.Signal();
      list.Append(p);
    }
  }
}


PSafeString::PSafeString(const PString & _string)
  : string(_string)
{
}


void PSafeString::PrintOn(ostream &strm) const
{
  strm << string;
}


DoSomeThing2::DoSomeThing2(PINDEX _index)
  : PThread(1000, AutoDeleteThread, NormalPriority, psprintf("DoSomeThing2 %u", _index))
  , index(_index)
{
  Resume();
}


void DoSomeThing2::Main()
{
  PRandom rand(PRandom::Number());

  PINDEX i;

  for (i = 0; i < 5000; i++) {
    PSafeString * p = new PSafeString(rand.Generate());
    list.Append(p);
//    coutMutex.Wait();
//    cout << GetThreadName() << ": Added " << *p << " element to sorted list" << endl;
//    coutMutex.Signal();
  }

  for (;;) {

    PINDEX remove = rand.Generate() % (list.GetSize() + 1);
    for (i = 0; i < remove; i++) {
      PINDEX index = rand.Generate() % list.GetSize();
      coutMutex.Wait();
      PSafePtr<PSafeString> str = list.GetAt(index, PSafeReference);
      cout << GetThreadName() << ": Removing element " << *str << " at index position " << index << endl;
      coutMutex.Signal();
      list.Remove(&(*str));
    }

    PINDEX add = rand.Generate() % 1000 + 300;
    for (i = 0; i < add; i++) {
      PSafeString * p = new PSafeString(rand.Generate());
      coutMutex.Wait();
      cout << GetThreadName() << ": Adding element " << *p << "to sorted list" << endl;
      coutMutex.Signal();
      list.Append(p);
    }

    list.DeleteObjectsToBeRemoved();
  }
}



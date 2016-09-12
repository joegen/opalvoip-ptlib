
#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/safecoll.h>

class SortedListTest:public PProcess {
  PCLASSINFO(SortedListTest, PProcess);
public:
  SortedListTest();
  void Main();
};


class DoSomeThing1:public PThread {
  PCLASSINFO(DoSomeThing1, PThread);
public:
  DoSomeThing1(PINDEX _index);
  void Main();
private:
  PINDEX m_index;
  PSortedList<PString> list;
};

class PSafeString:public PSafeObject {
  PCLASSINFO(PSafeString, PSafeObject);
  PSafeString(const PString & _string);
  void PrintOn(ostream &strm) const;
private:
  PString m_string;
};

class DoSomeThing2:public PThread {
  PCLASSINFO(DoSomeThing2, PThread);
public:
  DoSomeThing2(PINDEX _index);
  void Main();
private:
  PINDEX m_index;
  PSafeSortedList<PSafeString> list;
};



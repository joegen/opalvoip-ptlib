#include <ptlib.h>
#include "abstract.h"

extern unsigned PTraceCurrentLevel;

class Factory : public PProcess
{
  public:
    Factory()
    : PProcess() { }
    void Main();
};

PCREATE_PROCESS(Factory)

void Factory::Main()
{
  PGenericFactory<MyAbstractClass>::KeyList_T keyList = PGenericFactory<MyAbstractClass>::GetKeyList();
  PGenericFactory<MyAbstractClass>::KeyList_T::const_iterator r;

  cout << "List of concrete types:" << endl;
  for (r = keyList.begin(); r != keyList.end(); ++r)
    cout << "  " << *r << endl;
  cout << endl;

  unsigned i;
  for (i = 0; i < keyList.size(); i++) {
    for (int j = 0; j < 3; j++)
    {
      MyAbstractClass * c = PGenericFactory<MyAbstractClass>::CreateInstance(keyList[i]);
      if (c == NULL) 
        cout << "Cannot instantiate class " << keyList[i] << endl;
      else
        cout << keyList[i] << "::Function returned " << c->Function() << ", instance " << (void *)c << endl;
    }
  }
}

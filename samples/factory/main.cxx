#include <ptlib.h>
#include "abstract.h"

extern unsigned PTraceCurrentLevel;
PAbstractFactory<PProcessStartup, PTraceLevelSetStartup<4> > level("SetTraceLevel");

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
  PGenericFactory<AbstractClass>::KeyList keyList = PGenericFactory<AbstractClass>::GetKeyList();
  PGenericFactory<AbstractClass>::KeyList::const_iterator r;

  cout << "List of concrete types:" << endl;
  for (r = keyList.begin(); r != keyList.end(); ++r)
    cout << "  " << *r << endl;
  cout << endl;

  unsigned i;
  for (i = 0; i < keyList.size(); i++) {
    AbstractClass * c = PGenericFactory<AbstractClass>::CreateInstance(keyList[i]);
    if (c == NULL) 
      cout << "Cannot instantiate class " << keyList[i] << endl;
    else
      cout << keyList[i] << "::Function returned " << c->Function() << endl;
  }
}

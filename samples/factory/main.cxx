#include <ptlib.h>

#include <ptclib/http.h>
#include <ptclib/ptts.h>

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

template <class FactoryClass>
void TestFactory(const char * title)
{
  cout << "Testing " << title << endl;
  PFactory<FactoryClass>::KeyList_T keys = PFactory<FactoryClass>::GetKeyList();

  cout << "keys = " << endl;
  PFactory<FactoryClass>::KeyList_T::const_iterator r;
  for (r = keys.begin(); r != keys.end(); ++r) {
    cout << "  " << *r << endl;
  }
  
  cout << endl;
}

void Factory::Main()
{
  PFactory<MyAbstractClass>::KeyList_T keyList = PFactory<MyAbstractClass>::GetKeyList();
  PFactory<MyAbstractClass>::KeyList_T::const_iterator r;

  cout << "List of concrete types:" << endl;
  for (r = keyList.begin(); r != keyList.end(); ++r)
    cout << "  " << *r << endl;
  cout << endl;

  unsigned i;
  for (i = 0; i < keyList.size(); i++) {
    for (int j = 0; j < 3; j++)
    {
      MyAbstractClass * c = PFactory<MyAbstractClass>::CreateInstance(keyList[i]);
      if (c == NULL) 
        cout << "Cannot instantiate class " << keyList[i] << endl;
      else
        cout << keyList[i] << "::Function returned " << c->Function() << ", instance " << (void *)c << endl;
    }
  }

  TestFactory<PURLScheme>("PURLScheme");
  TestFactory<PTextToSpeech>("PTextToSpeech");
  TestFactory<PPluginModuleManager>("PPluginModuleManager");
}

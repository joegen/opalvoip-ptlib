
#include <ptlib.h>

#include <ptclib/http.h>
#include <ptclib/ptts.h>
#include <ptclib/pwavfile.h>

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

template <class FactoryClass, typename TypeClass>
void DisplayFactory(const char * title)
{
  cout << "Testing " << title << endl;
  PFactory<FactoryClass, TypeClass>::KeyList_T keys = PFactory<FactoryClass, TypeClass>::GetKeyList();

  cout << "keys = " << endl;
  PFactory<FactoryClass, TypeClass>::KeyList_T::const_iterator r;
  for (r = keys.begin(); r != keys.end(); ++r) {
    cout << "  " << *r << endl;
  }
  
  cout << endl;
}

template <class BaseClass, class TypeClass>
void DisplayTestFactory()
{
  PFactory<BaseClass, TypeClass>::KeyList_T keyList = PFactory<MyAbstractClass, TypeClass>::GetKeyList();
  PFactory<BaseClass, TypeClass>::KeyList_T::const_iterator r;

  cout << "List of concrete types:" << endl;
  for (r = keyList.begin(); r != keyList.end(); ++r)
    cout << "  " << *r << endl;
  cout << endl;

  unsigned i;
  for (i = 0; i < keyList.size(); i++) {
    for (int j = 0; j < 3; j++)
    {
      MyAbstractClass * c = PFactory<MyAbstractClass, TypeClass>::CreateInstance(keyList[i]);
      if (c == NULL) 
        cout << "Cannot instantiate class " << keyList[i] << endl;
      else
        cout << keyList[i] << "::Function returned " << c->Function() << ", instance " << (void *)c << endl;
    }
  }
}

void Factory::Main()
{
  DisplayTestFactory<MyAbstractClass, PString>();
  DisplayTestFactory<MyAbstractClass, unsigned>();

  DisplayFactory<PURLScheme, PString>("PURLScheme");
  DisplayFactory<PTextToSpeech, PString>("PTextToSpeech");
  DisplayFactory<PPluginModuleManager, PString>("PPluginModuleManager");
  DisplayFactory<PWAVFileConverter, unsigned>("PWAVFileConverter");
}

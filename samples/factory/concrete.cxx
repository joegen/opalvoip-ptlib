#include <ptlib.h>

#include "abstract.h"

class MyConcreteClass : public MyAbstractClass
{
  public:
    PString Function()
    { return "Concrete"; }
};

PFactory<MyAbstractClass>::Worker<MyConcreteClass> concreteFactory("concrete", false);

class MyConcrete2Class : public MyAbstractClass
{
  public:
    PString Function()
    { return "Concrete2"; }
};

PFactory<MyAbstractClass>::Worker<MyConcrete2Class> concrete2Factory("concrete2", false);

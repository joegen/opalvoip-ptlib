#include <ptlib.h>

#include "abstract.h"

class MyConcreteClass : public MyAbstractClass
{
  public:
    PString Function()
    { return "Concrete"; }
};

PAbstractFactory<MyAbstractClass, MyConcreteClass> concreteFactory("concrete");

class MyConcrete2Class : public MyAbstractClass
{
  public:
    PString Function()
    { return "Concrete2"; }
};

PAbstractFactory<MyAbstractClass, MyConcrete2Class> concrete2Factory("concrete2");

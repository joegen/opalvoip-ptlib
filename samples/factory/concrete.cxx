#include <ptlib.h>

#include "abstract.h"

class ConcreteClass : public AbstractClass
{
  public:
    PString Function()
    { return "Concrete"; }
};

PAbstractFactory<AbstractClass, ConcreteClass> concreteFactory("concrete");
PINSTANTIATE_FACTORY(AbstractClass)

class Concrete2Class : public AbstractClass
{
  public:
    PString Function()
    { return "Concrete2"; }
};

PAbstractFactory<AbstractClass, Concrete2Class> concrete2Factory("concrete2");

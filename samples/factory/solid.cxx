#include <ptlib.h>

#include "abstract.h"

class SolidClass : public AbstractClass
{
  public:
    PString Function()
    { return "Solid"; }
};

PAbstractFactory<AbstractClass, SolidClass> solidFactory("solid");

class Solid2Class : public AbstractClass
{
  public:
    PString Function()
    { return "Solid2"; }
};

PAbstractFactory<AbstractClass, Solid2Class> solid2Factory("solid2");

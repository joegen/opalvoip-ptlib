#include <ptlib.h>

#include "abstract.h"

class MySolidClass : public MyAbstractClass
{
  public:
    PString Function()
    { return "Solid"; }
};

PAbstractFactory<MyAbstractClass, MySolidClass> solidFactory("solid");

class MySolid2Class : public MyAbstractClass
{
  public:
    PString Function()
    { return "Solid2"; }
};

PAbstractSingletonFactory<MyAbstractClass, MySolid2Class> solid2Factory("solid2");

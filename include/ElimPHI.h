#ifndef __ELIMPHI_H__
#define __ELIMPHI_H__

#include "Unit.h"

class ElimPHI
{
    Unit *unit;

public:
    ElimPHI(Unit *unit) : unit(unit){};
    void pass();
};

#endif
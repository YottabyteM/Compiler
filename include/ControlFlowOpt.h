#ifndef __CONTROLFLOWOPT_H__
#define __CONTROLFLOWOPT_H__

#include "Unit.h"

class ControlFlowOpt
{
    Unit *unit;
    // std::set<BasicBlock*> visited;

public:
    ControlFlowOpt(Unit *unit) : unit(unit){};
    void dfs(BasicBlock* bb);
    void pass();
};

#endif
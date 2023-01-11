#ifndef __CONTROLFLOWOPT_H__
#define __CONTROLFLOWOPT_H__

#include "Unit.h"

/*
    control flow optimization for IR:
        1)
        2)
*/
class ControlFlowOpt
{
    Unit *unit;
    // std::set<BasicBlock*> visited;

public:
    ControlFlowOpt(Unit *unit) : unit(unit){};
    void dfs(BasicBlock *bb);
    void pass();
};

#endif
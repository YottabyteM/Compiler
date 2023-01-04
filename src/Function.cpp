#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

extern FILE *yyout;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
}

Function::~Function()
{
    auto delete_list = block_list;
    for (auto &i : delete_list)
        delete i;
    parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    FunctionType *funcType = dynamic_cast<FunctionType *>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    fprintf(yyout, "define dso_local %s %s(", retType->toStr().c_str(), sym_ptr->toStr().c_str());
    fprintf(stderr, "define dso_local %s %s(", retType->toStr().c_str(), sym_ptr->toStr().c_str());
    for (auto it = paramslist.begin(); it != paramslist.end(); it++)
    {
        if (it != paramslist.begin())
        {
            fprintf(yyout, ", ");
            fprintf(stderr, ", ");
        }
        fprintf(yyout, "%s %s", (*it)->getType()->toStr().c_str(), (*it)->toStr().c_str());
        fprintf(stderr, "%s %s", (*it)->getType()->toStr().c_str(), (*it)->toStr().c_str());
    }
    fprintf(yyout, ") #0{\n");
    fprintf(stderr, ") #0{\n");
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    fprintf(yyout, "}\n");
    fprintf(stderr, "}\n");
}

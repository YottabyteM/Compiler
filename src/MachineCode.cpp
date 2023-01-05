#include "MachineCode.h"
extern FILE *yyout;

MachineOperand::MachineOperand(int tp, float val, bool is_float)
{
    this->type = tp;
    if (tp == MachineOperand::IMM)
        this->val = val;
    else
        this->reg_no = (int)val;
    this->is_float = is_float;
}

MachineOperand::MachineOperand(std::string label)
{
    this->type = MachineOperand::LABEL;
    this->label = label;
}

bool MachineOperand::operator==(const MachineOperand &a) const
{
    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

bool MachineOperand::operator<(const MachineOperand &a) const
{
    if (this->type == a.type)
    {
        if (this->type == IMM)
            return this->val < a.val;
        assert(this->type == VREG || this->type == REG);
        return this->reg_no < a.reg_no;
    }
    return this->type < a.type;
}

void MachineOperand::PrintReg()
{
    if (is_float)
    {
        switch (reg_no)
        {
        case 32:
            fprintf(yyout, "FPSCR");
            break;
        default:
            fprintf(yyout, "s%d", reg_no);
            break;
        }
    }
    else
    {
        switch (reg_no)
        {
        case 11:
            fprintf(yyout, "fp");
            break;
        case 13:
            fprintf(yyout, "sp");
            break;
        case 14:
            fprintf(yyout, "lr");
            break;
        case 15:
            fprintf(yyout, "pc");
            break;
        default:
            fprintf(yyout, "r%d", reg_no);
            break;
        }
    }
}

void MachineOperand::output()
{
    /* HINT：print operand
     * Example:
     * immediate num 1 -> print #1;
     * register 1 -> print r1;
     * lable addr_a -> print addr_a; */
    switch (this->type)
    {
    case IMM:
        if (is_float)
            fprintf(yyout, "#%u", reinterpret_cast<unsigned &>(this->val));
        else
            fprintf(yyout, "#%d", (int)this->val);
        break;
    case VREG:
        fprintf(yyout, "v%d", this->reg_no); // 不用区分浮点？
        break;
    case REG:
        PrintReg();
        break;
    case LABEL:
        if (this->label.substr(0, 2) == ".L")
            fprintf(yyout, "%s", this->label.c_str());
        else if (this->label.substr(0, 1) == "@")
            fprintf(yyout, "%s", this->label.c_str() + 1);
        else
            fprintf(yyout, "addr_%s", this->label.c_str());
    default:
        break;
    }
}

// 合法的第二操作数循环移位偶数位后可以用8bit表示
bool MachineOperand::isIllegalOp2()
{
    assert(this->isImm());
    unsigned bin_val;
    if (this->is_float)
        bin_val = reinterpret_cast<unsigned &>(this->val);
    else
    {
        signed sval = (int)(this->val);
        bin_val = reinterpret_cast<unsigned &>(sval);
    }
    for (int i = 0; i < 32; i += 2)
    {
        unsigned shift_val = (bin_val >> i) | (bin_val << (32 - i)); // 循环右移i位
        if (!(shift_val & 0xFFFFFF00))
            return false;
    }
    return true;
}

void MachineInstruction::PrintCond()
{
    switch (cond)
    {
    case EQ:
        fprintf(yyout, "eq");
        break;
    case NE:
        fprintf(yyout, "ne");
        break;
    case LT:
        fprintf(yyout, "lt");
        break;
    case LE:
        fprintf(yyout, "le");
        break;
    case GT:
        fprintf(yyout, "gt");
        break;
    case GE:
        fprintf(yyout, "ge");
        break;
    default:
        break;
    }
}

BinaryMInstruction::BinaryMInstruction(
    MachineBlock *p, int op,
    MachineOperand *dst, MachineOperand *src1, MachineOperand *src2,
    int cond)
{
    this->parent = p;
    this->type = MachineInstruction::BINARY;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
}

void BinaryMInstruction::output()
{
    if (def_list[0]->isFloat())
    {
        switch (this->op)
        {
        case BinaryMInstruction::ADD:
            fprintf(yyout, "\tvadd.f32 ");
            break;
        case BinaryMInstruction::SUB:
            fprintf(yyout, "\tvsub.f32 ");
            break;
        case BinaryMInstruction::MUL:
            fprintf(yyout, "\tvmul.f32 ");
            break;
        case BinaryMInstruction::DIV:
            fprintf(yyout, "\tvdiv.f32 ");
            break;
        default:
            break;
        }
    }
    else
    {
        switch (this->op)
        {
        case BinaryMInstruction::ADD:
            fprintf(yyout, "\tadd ");
            break;
        case BinaryMInstruction::SUB:
            fprintf(yyout, "\tsub ");
            break;
        case BinaryMInstruction::MUL:
            fprintf(yyout, "\tmul ");
            break;
        case BinaryMInstruction::DIV:
            fprintf(yyout, "\tsdiv ");
            break;
        default:
            break;
        }
    }
    this->PrintCond();
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, "\n");
}

LoadMInstruction::LoadMInstruction(MachineBlock *p,
                                   MachineOperand *dst, MachineOperand *src1, MachineOperand *src2,
                                   int cond)
{
    this->parent = p;
    this->type = MachineInstruction::LOAD;
    this->op = -1;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    if (src2)
        this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    if (src2)
        src2->setParent(this);
}

void LoadMInstruction::output()
{
    // 小的立即数用MOV优化一下
    if ((this->use_list.size() == 1) && this->use_list[0]->isImm() && !this->use_list[0]->isIllegalOp2())
    {
        if (this->def_list[0]->isFloat())
            fprintf(yyout, "\tvmov");
        else
            fprintf(yyout, "\tmov");
    }
    else
    {
        if (this->def_list[0]->isFloat())
            fprintf(yyout, "\tvldr.32 ");
        else
            fprintf(yyout, "\tldr ");
    }
    this->def_list[0]->output();
    fprintf(yyout, ", ");

    // Load immediate num, eg: ldr r1, =8
    if (this->use_list[0]->isImm())
    {
        fprintf(yyout, "=%d\n", this->use_list[0]->getVal());
        return;
    }

    // Load address
    if (this->use_list[0]->isReg() || this->use_list[0]->isVReg())
        fprintf(yyout, "[");

    this->use_list[0]->output();
    if (this->use_list.size() > 1)
    {
        fprintf(yyout, ", ");
        this->use_list[1]->output();
    }

    if (this->use_list[0]->isReg() || this->use_list[0]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

StoreMInstruction::StoreMInstruction(MachineBlock *p,
                                     MachineOperand *src1, MachineOperand *src2, MachineOperand *src3,
                                     int cond)
{
    this->parent = p;
    this->type = MachineInstruction::STORE;
    this->op = -1;
    this->cond = cond;
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    if (src3)
        this->use_list.push_back(src3);
    src1->setParent(this);
    src2->setParent(this);
    if (src3)
        src3->setParent(this);
}

void StoreMInstruction::output()
{
    if (this->use_list[0]->isFloat())
        fprintf(yyout, "\tvstr.32 ");
    else
        fprintf(yyout, "\tstr ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");

    // Store address
    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
        fprintf(yyout, "[");

    this->use_list[1]->output();
    if (this->use_list.size() > 2)
    {
        fprintf(yyout, ", ");
        this->use_list[2]->output();
    }

    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

MovMInstruction::MovMInstruction(MachineBlock *p, int op,
                                 MachineOperand *dst, MachineOperand *src,
                                 int cond)
{
    assert(!src->isImm());
    this->parent = p;
    this->type = MachineInstruction::MOV;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void MovMInstruction::output()
{
    switch (this->op)
    {
    case MovMInstruction::MOV:
        fprintf(yyout, "\tmov");
        break;
    // case MovMInstruction::MVN:
    //     fprintf(yyout, "\tmvn");
    //     break;
    // case MovMInstruction::MOVT:
    //     fprintf(yyout, "\tmovt");
    //     break;
    case MovMInstruction::VMOV:
        fprintf(yyout, "\tvmov");
        break;
    // case MovMInstruction::VMOVF32:
    //     fprintf(yyout, "\tvmov.f32");
    //     break;
    default:
        break;
    }
    PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");
}

BranchMInstruction::BranchMInstruction(MachineBlock *p, int op,
                                       MachineOperand *dst,
                                       int cond)
{
    // TODO
    this->type = MachineInstruction::BRANCH;
    this->op = op;
    this->parent = p;
    this->cond = cond;
    this->def_list.push_back(dst);
    dst->setParent(this);
}

void BranchMInstruction::output()
{
    // TODO
    switch (op)
    {
    case BL:
        fprintf(yyout, "\tbl ");
        break;
    case B:
        fprintf(yyout, "\tb");
        PrintCond();
        fprintf(yyout, " ");
        break;
        break;
    // case BX:
    //     fprintf(yyout, "\tbx ");
    //     break;
    default:
        break;
    }
    this->def_list[0]->output();
    fprintf(yyout, "\n");
}

CmpMInstruction::CmpMInstruction(MachineBlock *p,
                                 MachineOperand *src1, MachineOperand *src2,
                                 int cond)
{
    // TODO
    this->type = MachineInstruction::CMP;
    this->parent = p;
    this->op = cond = cond;
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    src1->setParent(this);
    src2->setParent(this);
}

void CmpMInstruction::output()
{
    // TODO
    // Jsut for reg alloca test
    // delete it after test
    fprintf(yyout, "\tcmp ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, "\n");
}

StackMInstruction::StackMInstruction(MachineBlock *p, int op,
                                     MachineOperand *src,
                                     int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    this->use_list.push_back(src);
    src->setParent(this);
}

StackMInstruction::StackMInstruction(MachineBlock *p, int op,
                                     std::vector<MachineOperand *> src,
                                     int cond)
{
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    this->use_list = src;
    for (auto ope : use_list)
    {
        ope->setParent(this);
    }
}

void StackMInstruction::output()
{
    std::string op_str;
    if (this->use_list[0]->isFloat())
    {
        switch (op)
        {
        case PUSH:
            op_str = "\tvpush {";
            break;
        case POP:
            op_str = "\tvpop {";
            break;
        }
    }
    else
    {
        switch (op)
        {
        case PUSH:
            op_str = "\tpush {";
            break;
        case POP:
            op_str = "\tpop {";
            break;
        }
    }
    // 浮点寄存器可能会很多 每次只能push/pop16个
    size_t i = 0;
    while (i != use_list.size())
    {
        fprintf(yyout, op_str.c_str());
        this->use_list[i++]->output();
        for (size_t j = 1; i != use_list.size() && j < 16; i++, j++)
        {
            fprintf(yyout, ", ");
            this->use_list[i]->output();
        }
        fprintf(yyout, "}\n");
    }
}

ZextMInstruction::ZextMInstruction(MachineBlock *p, MachineOperand *dst, MachineOperand *src, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::ZEXT;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void ZextMInstruction::output()
{
    fprintf(yyout, "\tuxtb ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

VcvtMInstruction::VcvtMInstruction(MachineBlock *p,
                                   int op,
                                   MachineOperand *dst,
                                   MachineOperand *src,
                                   int cond)
{
    this->parent = p;
    this->type = MachineInstruction::VCVT;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void VcvtMInstruction::output()
{
    switch (this->op)
    {
    case VcvtMInstruction::F2S:
        fprintf(yyout, "\tvcvt.s32.f32 ");
        break;
    case VcvtMInstruction::S2F:
        fprintf(yyout, "\tvcvt.f32.s32 ");
        break;
    default:
        break;
    }
    PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");
}

MachineFunction::MachineFunction(MachineUnit *p, SymbolEntry *sym_ptr)
{
    this->parent = p;
    this->sym_ptr = sym_ptr;
    this->stack_size = 0;
};

void MachineBlock::output()
{
    fprintf(yyout, ".L%d:\n", this->no);
    for (auto iter : inst_list)
        iter->output();
}

void MachineFunction::output()
{
    const char *func_name = this->sym_ptr->toStr().c_str() + 1;
    fprintf(yyout, "\t.global %s\n", func_name);
    fprintf(yyout, "\t.type %s , %%function\n", func_name);
    fprintf(yyout, "%s:\n", func_name);
    // TODO
    /* Hint:
     *  1. Save fp
     *  2. fp = sp
     *  3. Save callee saved register
     *  4. Allocate stack space for local variable */

    // Traverse all the block in block_list to print assembly code.
    for (auto iter : block_list)
        iter->output();
}

void MachineUnit::PrintGlobalDecl()
{
    // TODO:
    // You need to print global variable/const declarition code;
    fprintf(yyout, "\t.data\n");
    for (auto v : global_list) {
        if (!v->getType()->isARRAY()) {
            fprintf(yyout, "\t.global %s\n", v->toStr().erase(0,1).c_str());
            fprintf(yyout, "\t.align 4\n");
            fprintf(yyout,"\t.size %s, %d\n", v->toStr().erase(0,1).c_str(), v->getType()->getSize());
            fprintf(yyout,"%s:\n", v->toStr().erase(0,1).c_str());
            if(v->getType()->isInt()) {
                fprintf(yyout, "\t.word %d\n", int(v->getValue()));
            } else {
                auto value = float(v->getValue());
                uint32_t temp = reinterpret_cast<uint32_t&>(value);
                fprintf(yyout, "\t.word %u\n", temp);
            }
        }
        else {
            // TODO
        }
    }
}

void MachineUnit::output()
{
    // TODO
    /* Hint:
     * 1. You need to print global variable/const declarition code;
     * 2. Traverse all the function in func_list to print assembly code;
     * 3. Don't forget print bridge label at the end of assembly code!! */
    fprintf(yyout, "\t.arch armv8-a\n");
    fprintf(yyout, "\t.fpu vfpv3-d16\n");
    fprintf(yyout, "\t.arch_extension crc\n");
    fprintf(yyout, "\t.arm\n");
    PrintGlobalDecl();
    for (auto iter : func_list)
        iter->output();
    // for (auto v : global_list) {
    //     fprintf(yyout, "addr_%s_%d:\n", v->toStr().erase(0, 1).c_str(), n);
    //     fprintf(yyout, "\t.word %s\n", v->toStr().erase(0,1).c_str());
    // }
}

void MachineBlock::insertBefore(MachineInstruction *pos, MachineInstruction *cont)
{
    auto p = find(inst_list.begin(), inst_list.end(), pos);
    inst_list.insert(p, cont);
}

void MachineBlock::insertAfter(MachineInstruction *pos, MachineInstruction *cont)
{
    auto p = find(inst_list.begin(), inst_list.end(), pos);
    if (p == inst_list.end())
    {
        inst_list.push_back(cont);
        return;
    }
    inst_list.insert(p + 1, cont);
}
#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include "Function.h"
#include "Type.h"
extern FILE *yyout;

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
    fprintf(stderr, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
    fprintf(stderr, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb) : Instruction(STORE, insert_bb)
{
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    // TODO : Array
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
    fprintf(stderr, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    if (operands[0]->getType() == TypeSystem::intType)
    {
        switch (opcode)
        {
        case ADD:
            op = "add";
            break;
        case SUB:
            op = "sub";
            break;
        case MUL:
            op = "mul";
            break;
        case DIV:
            op = "sdiv";
            break;
        case MOD:
            op = "srem";
            break;
        default:
            break;
        }
    }
    else
    {
        // fprintf(stderr, "oprendType is Array : %d", operands[0]->getType()->isARRAY());
        // assert(operands[0]->getType() == TypeSystem::floatType);
        switch (opcode)
        {
        case ADD:
            op = "fadd";
            break;
        case SUB:
            op = "fsub";
            break;
        case MUL:
            op = "fmul";
            break;
        case DIV:
            op = "fdiv";
            break;
        default:
            break;
        }
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    fprintf(stderr, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(CMP, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    if (operands[1]->getType()->isInt() || operands[1]->getType()->isConstInt()) // i1/i32
    {
        switch (opcode)
        {
        case E:
            op = "eq";
            break;
        case NE:
            op = "ne";
            break;
        case L:
            op = "slt";
            break;
        case LE:
            op = "sle";
            break;
        case G:
            op = "sgt";
            break;
        case GE:
            op = "sge";
            break;
        default:
            op = "";
            break;
        }
        fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
        fprintf(stderr, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
    else
    {
        assert(operands[1]->getType() == TypeSystem::floatType || operands[1]->getType() == TypeSystem::constFloatType);
        switch (opcode)
        {
        case E:
            op = "oeq";
            break;
        case NE:
            op = "one";
            break;
        case L:
            op = "olt";
            break;
        case LE:
            op = "ole";
            break;
        case G:
            op = "ogt";
            break;
        case GE:
            op = "oge";
            break;
        default:
            op = "";
            break;
        }
        fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
        fprintf(stderr, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(stderr, "  br label %%B%d\n", branch->getNo());
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(BasicBlock *true_branch, BasicBlock *false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb)
{
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    type = operands[0]->getType()->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
    fprintf(stderr, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if (src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if (!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if (operands.empty())
    {
        fprintf(yyout, "  ret void\n");
        fprintf(stderr, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
        fprintf(stderr, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

ZextInstruction::ZextInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(ZEXT, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

ZextInstruction::~ZextInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void ZextInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = zext %s %s to %s\n", dst.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
    fprintf(stderr, "  %s = zext %s %s to %s\n", dst.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}

IntFloatCastInstruction::IntFloatCastInstruction(unsigned opcode, Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(IFCAST, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

IntFloatCastInstruction::~IntFloatCastInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void IntFloatCastInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    std::string op;
    switch (opcode)
    {
    case S2F:
        op = "sitofp";
        break;
    case F2S:
        op = "fptosi";
        break;
    default:
        op = "";
        break;
    }
    fprintf(yyout, "  %s = %s %s %s to %s\n", dst.c_str(), op.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
    fprintf(stderr, "  %s = %s %s %s to %s\n", dst.c_str(), op.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}

FuncCallInstruction::FuncCallInstruction(Operand *dst, std::vector<Operand *> params, IdentifierSymbolEntry *funcse, BasicBlock *insert_bb) : Instruction(CALL, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    for (auto it : params)
    {
        operands.push_back(it);
        it->addUse(this);
    }
    func_se = funcse;
}

FuncCallInstruction::~FuncCallInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    for (size_t i = 1; i != operands.size(); ++i)
        operands[i]->removeUse(this);
}

void FuncCallInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    Type *returnType = dynamic_cast<FunctionType *>(func_se->getType())->getRetType();
    if (!returnType->isVoid())
    { // 仅当返回值为非void时，向临时寄存器赋值
        fprintf(yyout, "  %s = call %s %s(", dst.c_str(), dst_type.c_str(), func_se->toStr().c_str());
        fprintf(stderr, "  %s = call %s %s(", dst.c_str(), dst_type.c_str(), func_se->toStr().c_str());
    }
    else
    {
        fprintf(yyout, "  call %s %s(", dst_type.c_str(), func_se->toStr().c_str());
        fprintf(stderr, "  call %s %s(", dst_type.c_str(), func_se->toStr().c_str());
    }
    for (size_t i = 1; i != operands.size(); i++)
    {
        std::string src = operands[i]->toStr();
        std::string src_type = operands[i]->getType()->toStr();
        if (i != 1)
        {
            fprintf(yyout, ", ");
            fprintf(stderr, ", ");
        }
        fprintf(yyout, "%s %s", src_type.c_str(), src.c_str());
        fprintf(stderr, "%s %s", src_type.c_str(), src.c_str());
    }
    fprintf(yyout, ")\n");
    fprintf(stderr, ")\n");
}

MachineOperand *Instruction::genMachineOperand(Operand *ope)
{
    auto se = ope->getEntry();
    MachineOperand *mope = nullptr;
    if (se->isConstant())
        mope = new MachineOperand(MachineOperand::IMM, se->getValue(), se->getType());
    else if (se->isTemporary())
        mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry *>(se)->getLabel(), se->getType());
    else if (se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry *>(se);
        if (id_se->getType()->isConst()) // 常量折叠
            mope = new MachineOperand(MachineOperand::IMM, se->getValue(), se->getType());
        else if (id_se->isGlobal())
            mope = new MachineOperand(id_se->toStr().c_str());
        else if (id_se->isParam())
        {
            int paramNo = id_se->getParamNo();
            if (paramNo >= 0 && paramNo <= 3)
                mope = new MachineOperand(MachineOperand::REG, paramNo, se->getType());
        }
        else
        {
            assert(0);
            exit(0);
        }
    }
    return mope;
}

MachineOperand *Instruction::genMachineReg(int reg, Type *valType)
{
    return new MachineOperand(MachineOperand::REG, reg, valType);
}

MachineOperand *Instruction::genMachineVReg(Type *valType)
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel(), valType);
}

MachineOperand *Instruction::genMachineImm(double val, Type *valType)
{
    return new MachineOperand(MachineOperand::IMM, val, valType);
}

MachineOperand *Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buf;
    buf << ".L" << block_no;
    std::string label = buf.str();
    return new MachineOperand(label);
}

void AllocaInstruction::genMachineCode(AsmBuilder *builder)
{
    /* HINT:
     * Allocate stack space for local variabel
     * Store frame offset in symbol entry */
    auto cur_func = builder->getFunction();
    auto id_se = dynamic_cast<IdentifierSymbolEntry *>(se);
    if (id_se->isParam())
    {
        if (id_se->getParamNo() < 4) // >= 0
        {
            int offset = cur_func->AllocSpace(/*se->getType()->getSize()*/ 4);
            dynamic_cast<TemporarySymbolEntry *>(operands[0]->getEntry())->setOffset(-offset);
        }
        else
        {
            // int below_dist = 0;
            // for (int i = 4; i != id_se->getParamNo(); i++)
            //     below_dist += parent->getParent()->getParamsList()[i]->getType()->getSize();
            int below_dist = 4 * (id_se->getParamNo() - 4);
            dynamic_cast<TemporarySymbolEntry *>(operands[0]->getEntry())->setOffset(below_dist);
        }
    }
    else
    {
        assert(id_se->isLocal());
        int offset = cur_func->AllocSpace(/*se->getType()->getSize()*/ 4);
        dynamic_cast<TemporarySymbolEntry *>(operands[0]->getEntry())->setOffset(-offset);
    }
}

void LoadInstruction::genMachineCode(AsmBuilder *builder)
{
    // TODO：Array
    auto cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    // Load global operand
    if (operands[1]->getEntry()->isVariable() && dynamic_cast<IdentifierSymbolEntry *>(operands[1]->getEntry())->isGlobal())
    {
        auto dst = genMachineOperand(operands[0]);
        // auto internal_reg1 = genMachineVReg();
        auto internal_reg1 = dst->getValType()->isFloat() ? genMachineVReg() : new MachineOperand(*dst);
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto src = genMachineOperand(operands[1]);
        // example: ldr r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, src);
        cur_block->InsertInst(cur_inst);
        // example: ldr r1, [r0]
        cur_inst = new LoadMInstruction(cur_block, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // Load local operand
    else if (operands[1]->getEntry()->isTemporary() && operands[1]->getDef() && operands[1]->getDef()->isAlloc())
    {
        // example: ldr r1, [fp, #4]
        auto dst = genMachineOperand(operands[0]);
        auto fp = genMachineReg(11);
        auto offset = genMachineImm(dynamic_cast<TemporarySymbolEntry *>(operands[1]->getEntry())->getOffset());
        // 如果是函数参数(第四个以后)，由于函数栈帧初始化时会将一些寄存器压栈，在FuncDef打印时还需要偏移一个值，这里保存未偏移前的值，方便后续调整
        if (dynamic_cast<TemporarySymbolEntry *>(operands[1]->getEntry())->getOffset() >= 0)
            cur_block->getParent()->addAdditionalArgsOffset(offset);
        if (offset->isIllegalShifterOperand())
            offset = cur_block->insertLoadImm(offset);
        cur_inst = new LoadMInstruction(cur_block, dst, fp, offset);
        cur_block->InsertInst(cur_inst);
    }
    // Load operand from temporary variable
    else
    {
        // example: ldr r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        cur_inst = new LoadMInstruction(cur_block, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

void StoreInstruction::genMachineCode(AsmBuilder *builder)
{
    // TODO : Array
    auto cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    MachineOperand *src = genMachineOperand(operands[1]);
    // src对应的参数不在前四个
    if (src == nullptr)
    {
        // auto id_se = dynamic_cast<IdentifierSymbolEntry *>(operands[1]->getEntry());
        // assert(id_se->isParam());
        // // int below_dist = 0;
        // // for (int i = 4; i != id_se->getParamNo(); i++)
        // //     below_dist += parent->getParent()->getParamsList()[i]->getType()->getSize();
        // int below_dist = 4 * (id_se->getParamNo() - 4);
        // dynamic_cast<TemporarySymbolEntry *>(operands[0]->getEntry())->setOffset(below_dist);
        return;
    }
    // 如果src为常数，需要先load进来
    if (src->isImm())
        src = cur_block->insertLoadImm(src);
    // Store global operand
    if (operands[0]->getEntry()->isVariable() && dynamic_cast<IdentifierSymbolEntry *>(operands[0]->getEntry())->isGlobal())
    {
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto dst = genMachineOperand(operands[0]);
        // example: ldr r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, internal_reg1, dst);
        cur_block->InsertInst(cur_inst);
        // example: str r1, [r0]
        cur_inst = new StoreMInstruction(cur_block, src, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // Store local operand
    else if (operands[0]->getEntry()->isTemporary() && operands[0]->getDef() && operands[0]->getDef()->isAlloc())
    {
        // example: str r1, [fp, #-4]
        auto fp = genMachineReg(11);
        auto offset = genMachineImm(dynamic_cast<TemporarySymbolEntry *>(operands[0]->getEntry())->getOffset());
        if (offset->isIllegalShifterOperand())
            offset = cur_block->insertLoadImm(offset);
        cur_inst = new StoreMInstruction(cur_block, src, fp, offset);
        cur_block->InsertInst(cur_inst);
    }
    // Store operand from temporary variable
    else
    {
        // example: str r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        cur_inst = new StoreMInstruction(cur_block, src, dst);
        cur_block->InsertInst(cur_inst);
    }
}

void BinaryInstruction::genMachineCode(AsmBuilder *builder)
{
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    /* HINT:
     * The source operands of ADD instruction in ir code both can be immediate num.
     * However, it's not allowed in assembly code.
     * So you need to insert LOAD/MOV instruction to load immediate num into register.
     * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/
    MachineInstruction *cur_inst = nullptr;
    if (opcode == MUL || opcode == DIV || opcode == MOD)
    {
        if (src2->isImm())
            src2 = cur_block->insertLoadImm(src2);
    }
    if (src1->isImm() && src2->isImm())
        src1 = cur_block->insertLoadImm(src1);
    if (src1->isImm() && !src2->isImm())
    {
        if (opcode == ADD)
            std::swap(src1, src2);
        else
            src1 = cur_block->insertLoadImm(src1);
    }
    if (src2->isImm() && src2->isIllegalShifterOperand())
        src2 = cur_block->insertLoadImm(src2);
    switch (opcode)
    {
    case ADD:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, src1, src2);
        break;
    case SUB:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, src2);
        break;
    case MUL:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst, src1, src2);
        break;
    case DIV:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
        break;
    case MOD:
    {
        // a % b = a - a / b * b
        auto internal_reg1 = dst;
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, internal_reg1, src1, src2);
        cur_block->InsertInst(cur_inst);
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, internal_reg2, internal_reg1, src2);
        cur_block->InsertInst(cur_inst);
        dst = new MachineOperand(*internal_reg2);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, internal_reg2);
        break;
    }
    default:
        break;
    }
    cur_block->InsertInst(cur_inst);
}

void CmpInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *cur_block = builder->getBlock();
    MachineOperand *src1 = genMachineOperand(operands[1]);
    MachineOperand *src2 = genMachineOperand(operands[2]);
    MachineInstruction *cur_inst = nullptr;
    if (src1->isImm())
        src1 = cur_block->insertLoadImm(src1);
    if (src2->isImm() && src2->isIllegalShifterOperand())
        src2 = cur_block->insertLoadImm(src2);
    cur_inst = new CmpMInstruction(cur_block, src1, src2);
    cur_block->InsertInst(cur_inst);
    if (src1->getValType()->isFloat())
    {
        cur_inst = new VmrsMInstruction(cur_block);
        cur_block->InsertInst(cur_inst);
    }
    builder->setCmpOpcode(opcode);
    // 采用条件存储的方式将1/0存储到dst中
    bool res_needed = false;
    for (auto user : operands[0]->getUses())
        if (!user->isCond())
        {
            res_needed = true;
            break;
        }
    if (res_needed)
    {
        MachineOperand *dst = genMachineOperand(operands[0]);
        MachineOperand *trueOperand = genMachineImm(1, TypeSystem::boolType);
        MachineOperand *falseOperand = genMachineImm(0, TypeSystem::boolType);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, trueOperand, opcode);
        cur_block->InsertInst(cur_inst);
        if (opcode == CmpInstruction::E || opcode == CmpInstruction::NE)
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, falseOperand, 1 - opcode);
        else
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, dst, falseOperand, 7 - opcode);
        cur_block->InsertInst(cur_inst);
    }
}

// TODO：把真假分支的指令序列分别加上相应的条件，改成bfs输出mblock，合并真假分支，这样就无需（无）条件跳转指令了

void UncondBrInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *cur_block = builder->getBlock();
    MachineOperand *dst = genMachineLabel(branch->getNo());
    MachineInstruction *cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
}

void CondBrInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *cur_block = builder->getBlock();
    MachineOperand *true_dst = genMachineLabel(true_branch->getNo());
    MachineOperand *false_dst = genMachineLabel(false_branch->getNo());
    // 符合当前块跳转条件有条件跳转到真分支
    MachineInstruction *cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, true_dst, builder->getCmpOpcode());
    cur_block->InsertInst(cur_inst);
    // 不符合当前块跳转条件无条件跳转到假分支
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, false_dst);
    cur_block->InsertInst(cur_inst);
}

void RetInstruction::genMachineCode(AsmBuilder *builder)
{
    /* HINT:
     * 1. Generate mov instruction to save return value in r0
     * 2. Restore callee saved registers and sp, fp
     * 3. Generate bx instruction */
    auto cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    // Generate mov instruction to save return value in r0
    if (!operands.empty())
    {
        auto src = genMachineOperand(operands[0]);
        if (src->isImm() && src->isIllegalShifterOperand())
            src = cur_block->insertLoadImm(src);
        auto dst = new MachineOperand(MachineOperand::REG, 0, src->getValType());
        cur_inst = new MovMInstruction(cur_block, src->getValType()->isFloat() ? MovMInstruction::VMOV : MovMInstruction::MOV, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

void ZextInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineBlock *cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    MachineOperand *src = genMachineOperand(operands[1]);
    // if (src->isImm() && src->isIllegalShifterOperand())
    //     src = cur_block->insertLoadImm(src);
    MachineOperand *dst = genMachineOperand(operands[0]);
    cur_inst = new ZextMInstruction(cur_block, dst, src);
    cur_block->InsertInst(cur_inst);
}

void IntFloatCastInstruction::genMachineCode(AsmBuilder *builder)
{
    MachineInstruction *cur_inst;
    auto cur_block = builder->getBlock();
    auto src = genMachineOperand(operands[1]);
    auto dst = genMachineOperand(operands[0]);
    if (src->isImm() && src->isIllegalShifterOperand())
        src = cur_block->insertLoadImm(src);
    switch (opcode)
    {
    case F2S:
    {
        auto internal_reg1 = new MachineOperand(*src);
        cur_inst = new VcvtMInstruction(cur_block, VcvtMInstruction::F2S, internal_reg1, src); // todo：internal_reg1和src应该可以是一个寄存器
        cur_block->InsertInst(cur_inst);
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
        break;
    }
    case S2F:
    {
        auto internal_reg1 = dst;
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, internal_reg1, src);
        cur_block->InsertInst(cur_inst);
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        cur_inst = new VcvtMInstruction(cur_block, VcvtMInstruction::S2F, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
        break;
    }
    }
}

void FuncCallInstruction::genMachineCode(AsmBuilder *builder)
{
    auto cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    // 传递参数
    for (size_t i = operands.size() - 1; i; i--)
    {
        auto arg = genMachineOperand(operands[i]);
        if (arg->isImm() && arg->isIllegalShifterOperand())
            arg = cur_block->insertLoadImm(arg);
        // 左起前4个参数通过r0-r3/s0-s3传递
        if (i - 1 < 4)
        {
            auto dst = new MachineOperand(MachineOperand::REG, i - 1, arg->getValType());
            cur_inst = new MovMInstruction(cur_block, arg->getValType()->isFloat() ? MovMInstruction::VMOV : MovMInstruction::MOV, dst, arg);
            cur_block->InsertInst(cur_inst);
        }
        // 后面的参数压栈
        else
        {
            cur_inst = new StackMInstruction(cur_block, StackMInstruction::PUSH, arg);
            cur_block->InsertInst(cur_inst);
        }
    }
    // 生成跳转指令进入callee函数，保存pc到lr，callee要保存lr
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::BL, new MachineOperand(func_se->toStr()));
    cur_block->InsertInst(cur_inst);
    cur_block->getParent()->addSavedRegs(14); // lr
    // 传递是否需要8 bytes aligned
    if (func_se->need8BytesAligned())
        dynamic_cast<IdentifierSymbolEntry *>(cur_block->getParent()->getSymPtr())->set8BytesAligned();
    // 如果之前通过压栈的方式传递了参数，需要恢复 SP 寄存器
    if (operands.size() > 5)
    {
        // int below_dist = 0;
        // for (size_t i = 5; i != operands.size(); i++)
        //     below_dist += operands[i]->getType()->getSize();
        int below_dist = (operands.size() - 5) * 4;
        auto offset = genMachineImm(below_dist);
        if (offset->isIllegalShifterOperand())
            offset = cur_block->insertLoadImm(offset);
        auto old_sp = genMachineReg(13);
        auto new_sp = genMachineReg(13);
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, new_sp, old_sp, offset);
        cur_block->InsertInst(cur_inst);
    }
    // 如果函数执行结果被用到，还需要保存 R0 寄存器中的返回值。
    if (operands[0]->getType() != TypeSystem::voidType) // TODO：这个判断并不精准，返回值非void不代表一定用到,后续可以通过窥孔优化删除无用指令
    {
        auto dst = genMachineOperand(operands[0]);
        auto src = new MachineOperand(MachineOperand::REG, 0, dst->getValType()); // r0/s0
        cur_inst = new MovMInstruction(cur_block, dst->getValType()->isFloat() ? MovMInstruction::VMOV : MovMInstruction::MOV, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

GepInstruction::GepInstruction(Operand* dst,
                               Operand* arr,
                               Operand* idx,
                               BasicBlock* insert_bb,
                               bool paramFirst)
    : Instruction(GEP, insert_bb), paramFirst(paramFirst) {
    operands.push_back(dst);
    operands.push_back(arr);
    operands.push_back(idx);
    dst->setDef(this);
    arr->addUse(this);
    idx->addUse(this);
    first = false;
    init = nullptr;
    last = false;
}

void GepInstruction::output() const {
    Operand* dst = operands[0];
    Operand* arr = operands[1];
    Operand* idx = operands[2];
    std::string arrType = arr->getType()->toStr();
    if (paramFirst)
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, i32 %s\n",
                dst->toStr().c_str(),
                arrType.substr(0, arrType.size() - 1).c_str(), arrType.c_str(),
                arr->toStr().c_str(), idx->toStr().c_str());
    else
        fprintf(
            yyout, "  %s = getelementptr inbounds %s, %s %s, i32 0, i32 %s\n",
            dst->toStr().c_str(), arrType.substr(0, arrType.size() - 1).c_str(),
            arrType.c_str(), arr->toStr().c_str(), idx->toStr().c_str());
}

GepInstruction::~GepInstruction() {
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void GepInstruction::genMachineCode(AsmBuilder* builder) {
    // auto cur_block = builder->getBlock();
    // MachineInstruction* cur_inst;
    // auto dst = genMachineOperand(operands[0]);
    // auto idx = genMachineOperand(operands[2]);
    // if(init){
    //     if(last){
    //         auto base = genMachineOperand(init);
    //         cur_inst = new BinaryMInstruction(
    //             cur_block, BinaryMInstruction::ADD, dst, base, genMachineImm(4));
    //         cur_block->InsertInst(cur_inst);
    //     }
    //     return;
    // }
    // MachineOperand* base = nullptr;
    // int size;
    // auto idx1 = genMachineVReg();
    // if (idx->isImm()) {
    //     if (idx->getVal() < 255) {
    //         cur_inst =
    //             new MovMInstruction(cur_block, MovMInstruction::MOV, idx1, idx);
    //     } else {
    //         cur_inst = new LoadMInstruction(cur_block, idx1, idx);
    //     }
    //     idx = new MachineOperand(*idx1);
    //     cur_block->InsertInst(cur_inst);
    // }
    // if (paramFirst) {
    //     size =
    //         ((PointerType*)(operands[1]->getType()))->getType()->getSize() / 8;
    // } else {
    //     if (first) {
    //         base = genMachineVReg();
    //         if (operands[1]->getEntry()->isVariable() &&
    //             ((IdentifierSymbolEntry*)(operands[1]->getEntry()))
    //                 ->isGlobal()) {
    //             auto src = genMachineOperand(operands[1]);
    //             cur_inst = new LoadMInstruction(cur_block, base, src);
    //         } else {
    //             int offset = ((TemporarySymbolEntry*)(operands[1]->getEntry()))
    //                              ->getOffset();
    //             if (offset > -255 && offset < 255) {
    //                 cur_inst =
    //                     new MovMInstruction(cur_block, MovMInstruction::MOV,
    //                                         base, genMachineImm(offset));
    //             } else {
    //                 cur_inst = new LoadMInstruction(cur_block, base,
    //                                                 genMachineImm(offset));
    //             }
    //         }
    //         cur_block->InsertInst(cur_inst);
    //     }
    //     ArrayType* type =
    //         (ArrayType*)(((PointerType*)(operands[1]->getType()))->getType());
    //     size = type->getElemType()->getSize() / 8;
    // }
    // auto size1 = genMachineVReg();
    // if (size > -255 && size < 255) {
    //     cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, size1,
    //                                    genMachineImm(size));
    // } else {
    //     cur_inst = new LoadMInstruction(cur_block, size1, genMachineImm(size));
    // }
    // cur_block->InsertInst(cur_inst);
    // auto off = genMachineVReg();
    // cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, off,
    //                                   idx, size1);
    // off = new MachineOperand(*off);
    // cur_block->InsertInst(cur_inst);
    // if (paramFirst || !first) {
    //     auto arr = genMachineOperand(operands[1]);
    //     cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD,
    //                                       dst, arr, off);
    //     cur_block->InsertInst(cur_inst);
    // } else {
    //     auto addr = genMachineVReg();
    //     auto base1 = new MachineOperand(*base);
    //     cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD,
    //                                       addr, base1, off);
    //     cur_block->InsertInst(cur_inst);
    //     addr = new MachineOperand(*addr);
    //     if (operands[1]->getEntry()->isVariable() &&
    //         ((IdentifierSymbolEntry*)(operands[1]->getEntry()))->isGlobal()) {
    //         cur_inst =
    //             new MovMInstruction(cur_block, MovMInstruction::MOV, dst, addr);
    //     } else {
    //         auto fp = genMachineReg(11);
    //         cur_inst = new BinaryMInstruction(
    //             cur_block, BinaryMInstruction::ADD, dst, fp, addr);
    //     }
    //     cur_block->InsertInst(cur_inst);
    // }
}
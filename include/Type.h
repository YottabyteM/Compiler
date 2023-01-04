#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>

class Type
{
private:
    int kind;

protected:
    enum
    {
        INT,
        FLOAT,
        VOID,
        FUNC,
        CONST_INT,
        CONST_FLOAT,
        PTR
    };
    int size;

public:
    Type(int kind) : kind(kind){};
    virtual ~Type(){};
    virtual std::string toStr() = 0;
    bool isInt() const { return kind == INT || kind == CONST_INT; }; // int/bool/const int/const bool
    bool isBool() const { return kind == INT && size == 1; };
    bool isFloat() const { return kind == FLOAT || kind == CONST_FLOAT; };
    bool isVoid() const { return kind == VOID; };
    bool isFunc() const { return kind == FUNC; };
    bool isConstInt() const { return kind == CONST_INT; }; // const int/const bool
    bool isConstBool() const { return kind == CONST_INT && size == 1; };
    bool isConstFloat() const { return kind == CONST_FLOAT; };
    bool isConst() const { return (kind == CONST_INT) || (kind == CONST_FLOAT); };
    bool isPTR() const { return kind == PTR; };
    int getSize() const { return (size == 32) ? 4 : 1; }; // 单位是字节数
};

class IntType : public Type
{
public:
    IntType(int size) : Type(Type::INT) { this->size = size; };
    std::string toStr();
};

class FloatType : public Type
{
public:
    FloatType(int size) : Type(Type::FLOAT) { this->size = size; };
    std::string toStr();
};

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};

class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type *> paramsType;

public:
    FunctionType(Type *returnType, std::vector<Type *> paramsType);
    std::string toStr();
    std::vector<Type *> getParamsType() { return paramsType; };
    void setParamsType(std::vector<Type *> paramsType) { this->paramsType = paramsType; };
    Type *getRetType() { return returnType; };
};

class ConstIntType : public Type
{
public:
    ConstIntType(int size) : Type(Type ::CONST_INT) { this->size = size; };
    std::string toStr();
};

class ConstFloatType : public Type
{
public:
    ConstFloatType(int size) : Type(Type ::CONST_FLOAT) { this->size = size; };
    std::string toStr();
};

class PointerType : public Type
{
private:
    Type *valueType;

public:
    PointerType(Type *valueType);
    std::string toStr();
};

class TypeSystem
{
private:
    static IntType commonInt;
    static FloatType commonFloat;
    static IntType commonBool;
    static VoidType commonVoid;
    static ConstIntType commonConstInt;
    static ConstFloatType commonConstFloat;
    static ConstIntType commonConstBool;

public:
    static Type *intType;
    static Type *floatType;
    static Type *boolType;
    static Type *voidType;
    static Type *constIntType;
    static Type *constFloatType;
    static Type *constBoolType;

    enum
    {
        PLUS,
        MINUS,
        NOT
    } unaryOp;
};
bool isIllegalOpType(Type *type);
Type *unaryMax(Type *type, unsigned int opcode);
Type *calcMax(Type *type1, Type *type2);
Type *logicMax(Type *type1, Type *type2);
Type *relMax(Type *type1, Type *type2);
Type *Var2Const(Type *type);
bool convertible(Type *from, Type *to);
#endif

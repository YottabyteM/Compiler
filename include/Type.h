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
        INT_ARRAY,
        FLOAT_ARRAY,
        CONST_INT_ARRAY,
        CONST_FLOAT_ARRAY,
        PTR
    };
    int size;

public:
    Type(int kind) : kind(kind){};
    virtual ~Type(){};
    virtual std::string toStr() = 0;
    bool isInt() const { return kind == INT || kind == CONST_INT; }; // int/bool/const int/const bool
    bool isBool() const { return (kind == INT || kind == CONST_INT) && size == 1; };
    bool isFloat() const { return kind == FLOAT || kind == CONST_FLOAT; };
    bool isVoid() const { return kind == VOID; };
    bool isFunc() const { return kind == FUNC; };
    bool isConstInt() const { return kind == CONST_INT; }; // const int/const bool
    bool isConstBool() const { return kind == CONST_INT && size == 1; };
    bool isConstFloat() const { return kind == CONST_FLOAT; };
    bool isConst() const { return (kind == CONST_INT) || (kind == CONST_FLOAT) || (kind == CONST_INT_ARRAY) || (kind == CONST_FLOAT_ARRAY); };
    bool isPTR() const { return kind == PTR; };
    int getSize() const { return (size - 1) / 8 + 1; }; // 单位是字节数, TODO：Array
    bool isIntArray() const { return kind == INT_ARRAY || kind == CONST_INT_ARRAY; };
    bool isConstIntArray() const { return kind == CONST_INT_ARRAY; };
    bool isFloatArray() const { return kind == FLOAT_ARRAY || kind == CONST_FLOAT_ARRAY; };
    bool isConstFloatArray() const { return kind == CONST_FLOAT_ARRAY; };
    bool isARRAY() const { return isIntArray() || isFloatArray(); };
};

class IntType : public Type
{
public:
    IntType(int size) : Type(Type::INT) { this->size = size; };
    std::string toStr();
};

class FloatType : public Type
{
private:
    bool is_Need = false;

public:
    FloatType(int size) : Type(Type::FLOAT) { this->size = size; };
    void need_Fp() { is_Need = true; };
    bool is_need() { return is_Need; };
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
private:
    bool is_need = false;

public:
    ConstFloatType(int size) : Type(Type ::CONST_FLOAT) { this->size = size; };
    void Need_Fp() { is_need = true; };
    bool is_Need() { return is_need; };
    std::string toStr();
};

class PointerType : public Type
{
private:
    Type *valueType;

public:
    PointerType(Type *valueType);
    std::string toStr();
    Type *getValType() { return valueType; };
};

class ArrayType : public Type
{
protected:
    std::vector<int> dim;
    bool is_pointer;

public:
    ArrayType(int elemType);
    // virtual ~ArrayType(){};
    void addDim(int d)
    {
        dim.push_back(d);
        size *= d;
    };
    std::vector<int> fetch() { return dim; };
    void SetPointer() { is_pointer = true; };
    void SetDim(std::vector<int> d) { dim = d; };
    bool isPointer() { return is_pointer; };
    int getLength() { return dim.size(); };
    Type *getElemType();
    virtual std::string toStr() = 0;
};

class IntArrayType : public ArrayType
{
public:
    IntArrayType() : ArrayType(Type::INT_ARRAY){};
    std::string toStr();
};

class ConstIntArrayType : public ArrayType
{
public:
    ConstIntArrayType() : ArrayType(Type::CONST_INT_ARRAY){};
    std::string toStr();
};

class FloatArrayType : public ArrayType
{
private:
    std::vector<int> dim;
    bool is_pointer = false;

public:
    FloatArrayType() : ArrayType(Type::FLOAT_ARRAY){};
    std::string toStr();
};

class ConstFloatArrayType : public ArrayType
{
private:
    std::vector<int> dim;
    bool is_pointer = false;

public:
    ConstFloatArrayType() : ArrayType(Type::CONST_FLOAT_ARRAY){};
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

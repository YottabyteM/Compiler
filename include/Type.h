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
    int getSize() const { return (size == 32) ? 4 : 1; }; // 单位是字节数, TODO：Array
    bool isIntArray() const { return kind == INT_ARRAY || kind == CONST_INT_ARRAY; };
    bool isFloatArray() const { return kind == FLOAT_ARRAY || kind == CONST_FLOAT_ARRAY; };
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
    bool is_need = false;
public:
    FloatType(int size) : Type(Type::FLOAT) { this->size = size; };
    bool need_Fp() { is_need = true; };
    bool is_need() { return is_need; };
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
};

class IntArrayType : public Type
{
private:
    std::vector<int> dim;
    bool isPointer = false;
public:
    IntArrayType() : Type(Type::INT_ARRAY){};
    void AddDim(int d) { dim.push_back(d); };
    std::vector<int> fetch() { return dim; };
    void SetPointer() { isPointer = true; };
    bool ispointer() { return isPointer; };
    std::string toStr();
};

class ConstIntArrayType : public Type
{
private:
    std::vector<int> dim;
    bool isPointer = false;
public:
    ConstIntArrayType() : Type(Type::CONST_INT_ARRAY){};
    void AddDim(int d) { dim.push_back(d); };
    std::vector<int> fetch() { return dim; };
    void SetPointer() { isPointer = true; };
    bool ispointer() { return isPointer; };
    std::string toStr();
};

class FloatArrayType : public Type
{
private:
    std::vector<int> dim;
    bool isPointer = false;
public:
    FloatArrayType() : Type(Type::FLOAT_ARRAY){};
    void AddDim(int d) { dim.push_back(d); };
    std::vector<int> fetch() { return dim; };
    void SetPointer() { isPointer = true; };
    bool ispointer() { return isPointer; };
    std::string toStr();
};

class ConstFloatArrayType : public Type
{
private:
    std::vector<int> dim;
    bool isPointer = false;
public:
    ConstFloatArrayType() : Type(Type::CONST_FLOAT_ARRAY){};
    void AddDim(int d) { dim.push_back(d); };
    std::vector<int> fetch() { return dim; };
    void SetPointer() { isPointer = true; };
    bool ispointer() { return isPointer; };
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

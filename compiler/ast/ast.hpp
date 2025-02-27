//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

//
// Contains the variants for all AST nodes
//
enum class V_AstType {
    None,
    
    // Global Statements
    ExternFunc,
    Func,
    StructDef,
    Block,
    
    // Statements
    Return,
    ExprStmt,
    BlockStmt,
    
    FuncCallStmt,
    
    VarDec,
    StructDec,
    
    If,
    While,
    Repeat,
    For,
    ForAll,
    
    Break,
    Continue,
    
    // Operators
    Neg,
    
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    
    And,
    Or,
    Xor,
    Lsh,
    Rsh,
    
    EQ,
    NEQ,
    GT,
    LT,
    GTE,
    LTE,
    
    LogicalAnd,
    LogicalOr,
    
    Sizeof,
    
    // Literals and identifiers
    CharL,
    IntL,
    FloatL,
    StringL,
    ID,
    ArrayAccess,
    StructAccess,
    
    // Expression list
    ExprList,
    
    // Function call expressions
    FuncCallExpr,
    
    // Various reference operators
    FuncRef,
    PtrTo,
    Ref,
    
    // Data types
    Void,
    Bool,
    Char,
    Int8,
    Int16,
    Int32,
    Int64,
    Float32,
    Float64,
    
    String,
    Ptr,
    Struct,
    Object
};

//
// Attributes needed by some languages
//
enum class Attr {
    Public,
    Protected,
    Private
};

// Forward declarations
struct AstExpression;
struct AstStatement;
struct AstFunction;

//
// The base of all AST nodes
//
struct AstNode {
public:
    explicit AstNode();
    explicit AstNode(V_AstType type);
    virtual void print() {}
    
    V_AstType type = V_AstType::None;
};

//
// AST data types
//

// The base of all AST data types
struct AstDataType : AstNode {
    explicit AstDataType(V_AstType type);
    explicit AstDataType(V_AstType type, bool _isUnsigned);
    void print() override;

    bool is_unsigned = false;
};

// Represents a pointer type
struct AstPointerType : AstDataType {
    explicit AstPointerType(std::shared_ptr<AstDataType> base_type); 
    void print() override;
    
    std::shared_ptr<AstDataType> base_type = nullptr;
};

// Represents a structure type
struct AstStructType : AstDataType {
    explicit AstStructType(std::string name);
    void print() override;
    
    std::string name = "";
};

// Represents an object type
struct AstObjectType : AstDataType {
    explicit AstObjectType(std::string name);
    void print() override;
    
    std::string name = "";
};

// Var
struct Var {
    explicit Var();
    explicit Var(std::shared_ptr<AstDataType> type, std::string name = "");
    
    std::string name;
    std::shared_ptr<AstDataType> type;
};

//
// AstStruct
// Represents a struct
//
struct AstStruct : AstNode {
    explicit AstStruct(std::string name);
    
    void addItem(Var var, std::shared_ptr<AstExpression> default_expression);
    
    void print();
    std::string dot(std::string parent);
    
    // Member variables
    std::string name;
    std::vector<Var> items;
    std::map<std::string, std::shared_ptr<AstExpression>> default_expressions;
    int size = 0;
};

//
// AstClass
// Represents a class
//
struct AstClass {
    explicit AstClass(std::string name) {
        this->name = name;
    }
    
    void addFunction(std::shared_ptr<AstFunction> func) {
        functions.push_back(func);
    }
    
    void print();
    
    std::string name = "";
    std::vector<std::shared_ptr<AstFunction>> functions;
};

//
// AstEnum
// Represents an enumeration
//
struct AstEnum {
    std::string name;
    std::shared_ptr<AstDataType> type;
    std::map<std::string, std::shared_ptr<AstExpression>> values;
};

//
// Represents an AstBlock
// Blocks hold tables and symbol information
//
struct AstBlock : AstNode {
    AstBlock() : AstNode(V_AstType::Block) {}

    void addStatement(std::shared_ptr<AstStatement> stmt);
    void addStatements(std::vector<std::shared_ptr<AstStatement>> block);
    std::vector<std::shared_ptr<AstStatement>> getBlock();
    
    size_t getBlockSize();
    std::shared_ptr<AstStatement> getStatementAt(size_t i);
    
    void removeAt(size_t pos);
    void insertAt(std::shared_ptr<AstStatement> stmt, size_t pos);
    
    void addSymbol(std::string name, std::shared_ptr<AstDataType> dataType);
    void mergeSymbols(std::shared_ptr<AstBlock> parent);
    std::map<std::string, std::shared_ptr<AstDataType>> getSymbolTable();
    std::shared_ptr<AstDataType> getDataType(std::string name);
    
    bool isVar(std::string name);
    int isConstant(std::string name);
    bool isFunc(std::string name);
    
    void print(int indent = 4);
    std::string dot(std::string parent);
    
    // Members
    std::vector<std::shared_ptr<AstStatement>> block;
    std::map<std::string, std::shared_ptr<AstDataType>> symbolTable;
    std::vector<std::string> vars;
    
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> globalConsts;
    std::map<std::string, std::pair<std::shared_ptr<AstDataType>, std::shared_ptr<AstExpression>>> localConsts;
    std::vector<std::string> funcs;
};

//
// -----------------------------------------------------
// AstExpressions
// -----------------------------------------------------
//

// Represents an AST expression
struct AstExpression : AstNode {
    explicit AstExpression() : AstNode(V_AstType::None) {}
    explicit AstExpression(V_AstType type) : AstNode(type) {}
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
};

// Holds a list of expressions
struct AstExprList : AstExpression {
    AstExprList() : AstExpression(V_AstType::ExprList) {}
    void add_expression(std::shared_ptr<AstExpression> expr) { list.push_back(expr); }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::vector<std::shared_ptr<AstExpression>> list;
};

// Represents the base of operators
struct AstOp : AstExpression {
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    bool is_binary = true;
};

// Represents the base of a unary expression
struct AstUnaryOp : AstOp {
    AstUnaryOp() {
        is_binary = false;
    }
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> value;
};

// Represents a negate expression
struct AstNegOp : AstUnaryOp {
    AstNegOp() {
        this->type = V_AstType::Neg;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the base of a binary expression
struct AstBinaryOp : AstOp {
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> lval;
    std::shared_ptr<AstExpression> rval;
    int precedence = 0;
};

// Represents an assignment operation
struct AstAssignOp : AstBinaryOp {
    explicit AstAssignOp() {
        this->type = V_AstType::Assign;
        this->precedence = 16;
    }
    
    explicit AstAssignOp(std::shared_ptr<AstExpression> lval, std::shared_ptr<AstExpression> rval) {
        this->type = V_AstType::Assign;
        this->precedence = 16;
        this->lval = lval;
        this->rval = rval;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an add operation
struct AstAddOp : AstBinaryOp {
    AstAddOp() {
        this->type = V_AstType::Add;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a subtraction operation
struct AstSubOp : AstBinaryOp {
    AstSubOp() {
        this->type = V_AstType::Sub;
        this->precedence = 4;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a multiplication operation
struct AstMulOp : AstBinaryOp {
    AstMulOp() {
        this->type = V_AstType::Mul;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
struct AstDivOp : AstBinaryOp {
    AstDivOp() {
        this->type = V_AstType::Div;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents the modulus operation
struct AstModOp : AstBinaryOp {
    AstModOp() {
        this->type = V_AstType::Mod;
        this->precedence = 3;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a division operation
struct AstAndOp : AstBinaryOp {
    AstAndOp() {
        this->type = V_AstType::And;
        this->precedence = 8;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an or operation
struct AstOrOp : AstBinaryOp {
    AstOrOp() {
        this->type = V_AstType::Or;
        this->precedence = 10;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a xor operation
struct AstXorOp : AstBinaryOp {
    AstXorOp() {
        this->type = V_AstType::Xor;
        this->precedence = 9;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an LSH operation
class AstLshOp : public AstBinaryOp {
public:
    AstLshOp() {
        this->type = V_AstType::Lsh;
        this->precedence = 10;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an RSH operation
class AstRshOp : public AstBinaryOp {
public:
    AstRshOp() {
        this->type = V_AstType::Rsh;
        this->precedence = 10;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents an equal-to operation
struct AstEQOp : AstBinaryOp {
    AstEQOp() {
        this->type = V_AstType::EQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a not-equal-to operation
struct AstNEQOp : AstBinaryOp {
    AstNEQOp() {
        this->type = V_AstType::NEQ;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than operation
struct AstGTOp : AstBinaryOp {
    AstGTOp() {
        this->type = V_AstType::GT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than operation
struct AstLTOp : AstBinaryOp {
    AstLTOp() {
        this->type = V_AstType::LT;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a greater-than-or-equal operation
struct AstGTEOp : AstBinaryOp {
    AstGTEOp() {
        this->type = V_AstType::GTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a less-than-or-equal operation
struct AstLTEOp : AstBinaryOp {
    AstLTEOp() {
        this->type = V_AstType::LTE;
        this->precedence = 6;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical AND operation
struct AstLogicalAndOp : AstBinaryOp {
    AstLogicalAndOp() {
        this->type = V_AstType::LogicalAnd;
        this->precedence = 11;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a logical OR operation
struct AstLogicalOrOp : AstBinaryOp {
    AstLogicalOrOp() {
        this->type = V_AstType::LogicalOr;
        this->precedence = 12;
    }
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a character literal
struct AstChar : AstExpression {
    explicit AstChar(char val) : AstExpression(V_AstType::CharL) {
        this->value = val;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    char value = 0;
};

// Represents an integer literal
struct AstInt : AstExpression {
    explicit AstInt(uint64_t value) : AstExpression(V_AstType::IntL) {
        this->value = value;
        this->size = 32;
    }
    
    explicit AstInt(uint64_t value, int size) : AstExpression(V_AstType::IntL) {
        this->value = value;
        this->size = size;
    }
    
    uint64_t value = 0;
    int size = 32;
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a floating-point literal
struct AstFloat : AstExpression {
    explicit AstFloat(double value) : AstExpression(V_AstType::FloatL) {
        this->value = value;
    }
    
    void print();
    
    double value = 0;
};

// Represents a string literal
struct AstString : AstExpression {
    explicit AstString(std::string value) : AstExpression(V_AstType::StringL) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};

// Represents a variable reference
struct AstID: AstExpression {
    explicit AstID(std::string val) : AstExpression(V_AstType::ID) {
        this->value = val;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};


// Represents a function reference
struct AstFuncRef : AstExpression {
    explicit AstFuncRef(std::string value) : AstExpression(V_AstType::FuncRef) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};

// Represents a pointer to something
struct AstPtrTo : AstExpression {
    explicit AstPtrTo(std::string value) : AstExpression(V_AstType::PtrTo) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};


// Represents a reference to something
struct AstRef : AstExpression {
    explicit AstRef(std::string value) : AstExpression(V_AstType::Ref) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string value = "";
};

// Represents an array access
struct AstArrayAccess : AstExpression {
    explicit AstArrayAccess(std::string value) : AstExpression(V_AstType::ArrayAccess) {
        this->value = value;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string value = "";
    std::shared_ptr<AstExpression> index;
};

// Represents a structure access
struct AstStructAccess : AstExpression {
    explicit AstStructAccess(std::string var, std::string member) : AstExpression(V_AstType::StructAccess) {
        this->var = var;
        this->member = member;
    }

    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string var = "";
    std::string member = "";
    
    // TODO: I don't love this
    // This is specific for members that are arrays
    std::shared_ptr<AstExpression> access_expression = nullptr;
};

// Represents a function call
struct AstFuncCallExpr : AstExpression {
    explicit AstFuncCallExpr(std::string name) : AstExpression(V_AstType::FuncCallExpr) {
        this->name = name;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    // Member variables
    std::shared_ptr<AstExpression> args;
    std::string name = "";
    std::string object_name;
};

// Represents the sizeof operator
struct AstSizeof : AstExpression {
    explicit AstSizeof(std::shared_ptr<AstID> value) : AstExpression(V_AstType::Sizeof) {
        this->value = value;
    }
    
    void print();

    std::shared_ptr<AstID> value;
};

//
// -----------------------------------------------------
// AstStatements
// -----------------------------------------------------
//

// Represents an AST statement
struct AstStatement : AstNode {
    explicit AstStatement();
    explicit AstStatement(V_AstType type);
    bool hasExpression();
    
    virtual void print() {}
    virtual std::string dot(std::string parent) { return ""; }
    
    std::shared_ptr<AstExpression> expression = nullptr;
};

// Represents an extern function
struct AstExternFunction : AstStatement {
    explicit AstExternFunction(std::string name) : AstStatement(V_AstType::ExternFunc) {
        this->name = name;
    }
    
    void addArgument(Var arg) { this->args.push_back(arg); }
    
    void print() override;
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstDataType> data_type;
    bool varargs = false;
};

// Represents a function
struct AstFunction : AstStatement {
    explicit AstFunction(std::string name) : AstStatement(V_AstType::Func) {
        this->name = name;
        block = std::make_shared<AstBlock>();
    }
    
    explicit AstFunction(std::string name, std::shared_ptr<AstDataType> data_type) : AstStatement(V_AstType::Func) {
        this->name = name;
        this->data_type = data_type;
        block = std::make_shared<AstBlock>();
    }
    
    void addStatement(std::shared_ptr<AstStatement> statement) {
        block->addStatement(statement);
    }
    
    void print() override;
    std::string dot(std::string parent) override;

    // Member variables
    std::string name = "";
    std::vector<Var> args;
    std::shared_ptr<AstBlock> block;
    std::shared_ptr<AstDataType> data_type;
    std::string dtName = "";
    
    // These attributes are language specific
    Attr attr = Attr::Public;
    bool routine = false;
};

// Represents an annontated block statement
// An annotated block has a name and multiple clauses
struct AstBlockStmt : AstStatement {
    explicit AstBlockStmt(std::string name = "") : AstStatement(V_AstType::BlockStmt) {
        this->name = name;
        block = std::make_shared<AstBlock>();
    }
    
    void print(int indent = 0);
    std::string dot(std::string parent) override;
    
    // Member variables
    std::string name = "";
    std::vector<std::string> clauses;
    std::shared_ptr<AstBlock> block;
};

// Represents an AST expression statement
// This is basically the same as a statement
struct AstExprStatement : AstStatement {
    explicit AstExprStatement() : AstStatement(V_AstType::ExprStmt) {}
    
    void setDataType(std::shared_ptr<AstDataType> dataType) {
        this->dataType = dataType;
    }
    
    std::shared_ptr<AstDataType> getDataType() { return dataType; }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::shared_ptr<AstDataType> dataType;
    
    // Language-specific attributes
    // TODO: I would like to eventually get rid of this
    std::string name = "";
};

// Represents a function call statement
struct AstFuncCallStmt : AstStatement {
    explicit AstFuncCallStmt(std::string name) : AstStatement(V_AstType::FuncCallStmt) {
        this->name = name;
    }
    
    std::string getName() { return name; }
    void print();
    std::string dot(std::string parent) override;
    
    std::string name = "";
    
    // Language-specific attributes
    std::string object_name = "";
};

// Represents a return statement
struct AstReturnStmt : AstStatement {
    explicit AstReturnStmt() : AstStatement(V_AstType::Return) {}
    
    void print();
    std::string dot(std::string parent) override;
};

// Represents a variable declaration
struct AstVarDec : AstStatement {
    explicit AstVarDec(std::string name, std::shared_ptr<AstDataType> data_type) : AstStatement(V_AstType::VarDec) {
        this->name = name;
        this->data_type = data_type;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string name = "";
    std::shared_ptr<AstDataType> data_type;
    
    // Language-specific attributes
    std::string class_name = "";
};

// Represents a structure declaration
struct AstStructDec : AstStatement {
    explicit AstStructDec(std::string var_name, std::string struct_name) : AstStatement(V_AstType::StructDec) {
        this->var_name = var_name;
        this->struct_name = struct_name;
    }
    
    void print();
    std::string dot(std::string parent) override;
    
    std::string var_name = "";
    std::string struct_name = "";
    bool no_init = false;
};

// Represents a conditional statement
struct AstIfStmt : AstStatement {
    explicit AstIfStmt() : AstStatement(V_AstType::If) {}
    
    void print(int indent);
    std::string dot(std::string parent) override;
    
    std::shared_ptr<AstBlock> true_block = nullptr;
    std::shared_ptr<AstBlock> false_block = nullptr;
};

// Represents a while statement
struct AstWhileStmt : AstStatement {
    explicit AstWhileStmt() : AstStatement(V_AstType::While) {}
    
    void print(int indent = 0);
    std::string dot(std::string parent) override;

    std::shared_ptr<AstBlock> block = nullptr;
};

// Represents an infinite loop statement
struct AstRepeatStmt : public AstStatement {
    explicit AstRepeatStmt() : AstStatement(V_AstType::Repeat) {}
    
    void print(int indent = 0);
    
    std::shared_ptr<AstBlock> block = nullptr;
};

// Represents a for loop
struct AstForStmt : public AstStatement {
    explicit AstForStmt() : AstStatement(V_AstType::For) {
        step = std::make_shared<AstInt>(1);
        block = std::make_shared<AstBlock>();
    }
    
    void print(int indent = 0);
    
    std::shared_ptr<AstID> index;
    std::shared_ptr<AstExpression> start;
    std::shared_ptr<AstExpression> end;
    std::shared_ptr<AstExpression> step;
    std::shared_ptr<AstDataType> data_type;
    std::shared_ptr<AstBlock> block = nullptr;
};

// Represents a for-all loop
struct AstForAllStmt : public AstStatement {
    explicit AstForAllStmt() : AstStatement(V_AstType::ForAll) {}
    
    void print(int indent = 0);
    
    std::shared_ptr<AstID> index;
    std::shared_ptr<AstID> array;
    std::shared_ptr<AstBlock> block = nullptr;
    std::shared_ptr<AstDataType> data_type;
};

// Represents a break statement for a loop
struct AstBreak : AstStatement {
    explicit AstBreak() : AstStatement(V_AstType::Break) {}
    void print();
    std::string dot(std::string parent) override;
};

// Represents a continue statement for a loop
struct AstContinue : AstStatement {
    explicit AstContinue() : AstStatement(V_AstType::Continue) {}
    void print();
    std::string dot(std::string parent) override;
};

//
// Represents an AST tree
//
struct AstTree {
    explicit AstTree(std::string file);
    ~AstTree();
    bool hasStruct(std::string name);
    
    void addGlobalStatement(std::shared_ptr<AstStatement> stmt) {
        block->addStatement(stmt);
    }
    
    void addStruct(std::shared_ptr<AstStruct> s) {
        structs.push_back(s);
    }
    
    void addClass(std::shared_ptr<AstClass> c) {
        classes.push_back(c);
    }
    
    void print();
    void dot();
    
    std::string file = "";
    std::shared_ptr<AstBlock> block;
    std::vector<std::shared_ptr<AstStruct>> structs;
    std::vector<std::shared_ptr<AstClass>> classes;
};


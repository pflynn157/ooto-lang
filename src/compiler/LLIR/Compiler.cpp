//
// Copyright 2021 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//

#include <iostream>
#include <exception>
#include <algorithm>

#include "Compiler.hpp"

Compiler::Compiler(AstTree *tree, CFlags cflags) {
    this->tree = tree;
    this->cflags = cflags;

    mod = new LLIR::Module(cflags.name);
    builder = new LLIR::IRBuilder(mod);
}

void Compiler::compile() {
    // Build the structures used by the program
    for (auto str : tree->getStructs()) {
        std::vector<LLIR::Type *> elementTypes;
        
        for (auto v : str->getItems()) {
            LLIR::Type *t = translateType(v.type, v.subType);
            elementTypes.push_back(t);
        }
        
        LLIR::StructType *s = new LLIR::StructType(str->getName(), elementTypes);
        structTable[str->getName()] = s;
        structElementTypeTable[str->getName()] = elementTypes;
    }

    // Build all other functions
    for (auto global : tree->getGlobalStatements()) {
        switch (global->getType()) {
            case AstType::Func: {
                symtable.clear();
                typeTable.clear();
                ptrTable.clear();
                
                compileFunction(global);
            } break;
            
            case AstType::ExternFunc: {
                compileExternFunction(global);
            } break;

            default: {}
        }
    }
}

void Compiler::debug() {
    //mod->print(errs(), nullptr);
    mod->print();
}

// Compiles an individual statement
void Compiler::compileStatement(AstStatement *stmt) {
    switch (stmt->getType()) {
        // Expression statement
        case AstType::ExprStmt: {
            AstExprStatement *expr_stmt = static_cast<AstExprStatement *>(stmt);
            compileValue(expr_stmt->getExpression(), expr_stmt->getDataType());
        } break;
    
        // A variable declaration (alloca) statement
        case AstType::VarDec: {
            AstVarDec *vd = static_cast<AstVarDec *>(stmt);
            LLIR::Type *type = translateType(vd->getDataType(), vd->getPtrType());
            
            LLIR::Reg *var = builder->createAlloca(type);
            symtable[vd->getName()] = var;
            typeTable[vd->getName()] = vd->getDataType();
            ptrTable[vd->getName()] = vd->getPtrType();
        } break;
        
        // A structure declaration
        case AstType::StructDec: {
            AstStructDec *sd = static_cast<AstStructDec *>(stmt);
            LLIR::StructType *type = structTable[sd->getStructName()];
            
            if (sd->isNoInit()) {
                LLIR::PointerType *typePtr = new LLIR::PointerType(type);
                LLIR::Reg *alloca = builder->createAlloca(typePtr);
                symtable[sd->getVarName()] = alloca;
                typeTable[sd->getVarName()] = DataType::Ptr;
                ptrTable[sd->getVarName()] = DataType::Struct;
                structVarTable[sd->getVarName()] = sd->getStructName();
                structArgs.push_back(sd->getVarName());            // This is a hack. Aka, it sucks
            } else {
                LLIR::Reg *var = builder->createAlloca(type);
                symtable[sd->getVarName()] = var;
                typeTable[sd->getVarName()] = DataType::Struct;
                structVarTable[sd->getVarName()] = sd->getStructName();
                
                AstStruct *str = nullptr;
                for (AstStruct *s : tree->getStructs()) {
                    if (s->getName() == sd->getStructName()) {
                        str = s;
                        break;
                    }
                }
                if (str == nullptr) return;
                
                // Init the elements
                int index = 0;
                for (Var member : str->getItems()) {
                    AstExpression *defaultExpr = str->getDefaultExpression(member.name);
                    LLIR::Operand *defaultVal = compileValue(defaultExpr, member.type);
                    
                    builder->createStructStore(type, var, index, defaultVal);
                    
                    ++index;
               }
            }
        } break;
        
        // Function call statements
        case AstType::FuncCallStmt: {
            compileFuncCallStatement(stmt);
        } break;
        
        // A return statement
        case AstType::Return: {
            compileReturnStatement(stmt);
        } break;
        
        // An IF statement
        case AstType::If: {
            compileIfStatement(stmt);
        } break;
        
        // A while loop
        case AstType::While: {
            compileWhileStatement(stmt);
        } break;
        
        // A break statement
        case AstType::Break: {
            builder->createBr(breakStack.top());
        } break;
        
        // A continue statement
        case AstType::Continue: {
            builder->createBr(continueStack.top());
        } break;
        
        default: {}
    }
}

// Converts an AST value to an LLVM value
LLIR::Operand *Compiler::compileValue(AstExpression *expr, DataType dataType, LLIR::Block *destBlock, bool isAssign) {
    LLIR::Type *type = translateType(dataType);

    switch (expr->getType()) {
        case AstType::I8L: {
            AstI8 *i8 = static_cast<AstI8 *>(expr);
            return builder->createI8(i8->getValue());
        } break;
        
        case AstType::I16L: {
            AstI16 *i16 = static_cast<AstI16 *>(expr);
            return builder->createI16(i16->getValue());
        } break;
        
        case AstType::I32L: {
            AstI32 *ival = static_cast<AstI32 *>(expr);
            return builder->createI32(ival->getValue());
        } break;
        
        case AstType::I64L: {
            AstI64 *i64 = static_cast<AstI64 *>(expr);
            return builder->createI64(i64->getValue());
        } break;
        
        case AstType::CharL: {
            AstChar *cval = static_cast<AstChar *>(expr);
            return builder->createI8(cval->getValue());
        } break;
        
        case AstType::StringL: {
            AstString *str = static_cast<AstString *>(expr);
            return builder->createString(str->getValue());
        } break;
        
        case AstType::ID: {
            AstID *id = static_cast<AstID *>(expr);
            LLIR::Reg *ptr = symtable[id->getValue()];
            LLIR::Type *type = translateType(typeTable[id->getValue()], ptrTable[id->getValue()]);
                        
            if (typeTable[id->getValue()] == DataType::Ptr && ptrTable[id->getValue()] == DataType::Struct) {
                return ptr;
            }
            
            if (typeTable[id->getValue()] == DataType::Struct || isAssign) return ptr;
            return builder->createLoad(type, ptr);
        } break;
        
        case AstType::ArrayAccess: {
            AstArrayAccess *acc = static_cast<AstArrayAccess *>(expr);
            LLIR::Reg *ptr = symtable[acc->getValue()];
            DataType ptrType = typeTable[acc->getValue()];
            LLIR::Operand *index = compileValue(acc->getIndex());
            
            if (ptrType == DataType::String) {
                LLIR::PointerType *strPtrType = LLIR::PointerType::createI8PtrType();
                LLIR::Type *i8Type = LLIR::Type::createI8Type();
                
                LLIR::Operand *ptrLd = builder->createLoad(strPtrType, ptr);
                LLIR::Operand *ep = builder->createGEP(strPtrType, ptrLd, index);
                if (isAssign) return ep;
                return builder->createLoad(i8Type, ep);
            } else {
                DataType subType = ptrTable[acc->getValue()];
                LLIR::Type *arrayPtrType = translateType(ptrType, subType);
                LLIR::Type *arrayElementType = translateType(subType);
                
                LLIR::Operand *ptrLd = builder->createLoad(arrayPtrType, ptr);
                LLIR::Operand *ep = builder->createGEP(arrayPtrType, ptrLd, index);
                if (isAssign) return ep;
                return builder->createLoad(arrayElementType, ep);
            }
        } break;

        case AstType::StructAccess: {
            AstStructAccess *sa = static_cast<AstStructAccess *>(expr);
            LLIR::Reg *ptr = symtable[sa->getName()];
            int pos = getStructIndex(sa->getName(), sa->getMember());
            
            std::string strTypeName = structVarTable[sa->getName()];
            LLIR::StructType *strType = structTable[strTypeName];
            LLIR::Type *elementType = structElementTypeTable[strTypeName][pos];
            
            if (std::find(structArgs.begin(), structArgs.end(), sa->getName()) != structArgs.end()) {
                LLIR::PointerType *strPtrType = new LLIR::PointerType(strType);
                LLIR::PointerType *elementPtrType = new LLIR::PointerType(elementType);
                
                ptr = builder->createLoad(strPtrType, ptr);
                LLIR::Operand *ep = builder->createGEP(elementPtrType, ptr, new LLIR::Imm(pos));
                if (isAssign) return ep;
                return builder->createLoad(elementType, ep);
            } else {
                return builder->createStructLoad(strType, ptr, pos);
            }
        } break;
        
        case AstType::FuncCallExpr: {
            AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(expr);
            std::vector<LLIR::Operand *> args;
            
            AstExprList *list = static_cast<AstExprList *>(fc->getArgExpression());
            for (auto arg : list->getList()) {
                LLIR::Operand *val = compileValue(arg);
                args.push_back(val);
            }
            
            return builder->createCall(type, fc->getName(), args);
        } break;
        
        case AstType::Assign: {
            AstAssignOp *op = static_cast<AstAssignOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            
            std::string name = "";
            switch (lvalExpr->getType()) {
                case AstType::ID: name = static_cast<AstID *>(lvalExpr)->getValue(); break;
                case AstType::ArrayAccess: {
                    name = static_cast<AstArrayAccess *>(lvalExpr)->getValue();
                } break;
                case AstType::StructAccess: {
                    name = static_cast<AstStructAccess *>(lvalExpr)->getName();
                } break;
                
                default: {}
            }
            
            LLIR::Operand *ptr = compileValue(lvalExpr, dataType, nullptr, true);
            LLIR::Operand *rval = compileValue(op->getRVal(), dataType);
            
            DataType ptrType = typeTable[name];
            LLIR::Type *type = translateType(ptrType);
            if (ptrType == DataType::Struct) {
                std::string strTypeName = structVarTable[name];
                type = structTable[strTypeName];
            }
            
            builder->createStore(type, rval, ptr);
        } break;
        
        case AstType::Neg: {
            AstNegOp *op = static_cast<AstNegOp *>(expr);
            LLIR::Operand *val = compileValue(op->getVal(), dataType);
            
            return builder->createNeg(type, val);
        } break;
        
        case AstType::LogicalAnd:
        case AstType::LogicalOr: {
            AstBinaryOp *op = static_cast<AstBinaryOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            AstExpression *rvalExpr = op->getRVal();
            
            // We only want the LVal first
            //LLIR::Operand *lval = compileValue(lvalExpr, dataType);
            
            // Create the blocks
            LLIR::Block *trueBlock = new LLIR::Block("true" + std::to_string(blockCount));
            ++blockCount;
            
            LLIR::Block *current = builder->getInsertPoint();
            builder->addBlockAfter(current, trueBlock);
            
            // Create the conditional branch
            if (expr->getType() == AstType::LogicalAnd) {
                LLIR::Block *falseBlock = logicalAndStack.top();
                LLIR::Operand *lval = compileValue(lvalExpr, dataType, trueBlock);
                builder->createBr(falseBlock);
            } else if (expr->getType() == AstType::LogicalOr) {
                LLIR::Block *trueBlock1 = logicalOrStack.top();
                LLIR::Operand *lval = compileValue(lvalExpr, dataType, trueBlock1);
                builder->createBr(trueBlock);
            }
            
            // Now, build the body of the second block
            builder->setInsertPoint(trueBlock);
            return compileValue(rvalExpr, dataType, destBlock);
        } break;
        
        case AstType::Add:
        case AstType::Sub: 
        case AstType::Mul:
        case AstType::Div:
        case AstType::And:
        case AstType::Or:
        case AstType::Xor:
        case AstType::EQ:
        case AstType::NEQ:
        case AstType::GT:
        case AstType::LT:
        case AstType::GTE:
        case AstType::LTE: {
            AstBinaryOp *op = static_cast<AstBinaryOp *>(expr);
            AstExpression *lvalExpr = op->getLVal();
            AstExpression *rvalExpr = op->getRVal();
            
            // Do a type detection so we don't end up with null comparisons
            DataType dType = dataType;
            if (dataType == DataType::Void) {
                if (lvalExpr->getType() == AstType::ID) {
                    AstID *id = static_cast<AstID *>(lvalExpr);
                    type = translateType(typeTable[id->getValue()], ptrTable[id->getValue()]);
                    dType = typeTable[id->getValue()];
                } else if (lvalExpr->getType() == AstType::ID) {
                    AstID *id = static_cast<AstID *>(rvalExpr);
                    type = translateType(typeTable[id->getValue()], ptrTable[id->getValue()]);
                    dType = typeTable[id->getValue()];
                }
            }
            
            // Now, compile the operands
            LLIR::Operand *lval = compileValue(lvalExpr, dType);
            LLIR::Operand *rval = compileValue(rvalExpr, dType);
            
            bool strOp = false;
            bool rvalStr = false;
            
            if (lvalExpr->getType() == AstType::StringL || rvalExpr->getType() == AstType::StringL) {
                strOp = true;
                rvalStr = true;
            } else if (lvalExpr->getType() == AstType::StringL && rvalExpr->getType() == AstType::CharL) {
                strOp = true;
            } else if (lvalExpr->getType() == AstType::ID && rvalExpr->getType() == AstType::CharL) {
                AstID *lvalID = static_cast<AstID *>(lvalExpr);
                if (typeTable[lvalID->getValue()] == DataType::String) strOp = true;
            } else if (lvalExpr->getType() == AstType::ID && rvalExpr->getType() == AstType::ID) {
                AstID *lvalID = static_cast<AstID *>(lvalExpr);
                AstID *rvalID = static_cast<AstID *>(rvalExpr);
                
                if (typeTable[lvalID->getValue()] == DataType::String) strOp = true;
                if (typeTable[rvalID->getValue()] == DataType::String) {
                    strOp = true;
                    rvalStr = true;
                } else if (typeTable[rvalID->getValue()] == DataType::Char ||
                           typeTable[rvalID->getValue()] == DataType::I8) {
                    strOp = true;          
                }
            }
            
            // Build a string comparison if necessary
            if (strOp) {
                std::vector<LLIR::Operand *> args;
                args.push_back(lval);
                args.push_back(rval);
            
                if (op->getType() == AstType::EQ || op->getType() == AstType::NEQ) {
                    LLIR::Operand *strcmpCall = builder->createCall(LLIR::Type::createI32Type(), "stringcmp", args);
                    
                    int cmpVal = 0;
                    if (op->getType() == AstType::NEQ) cmpVal = 0;
                    LLIR::Operand *cmpValue = builder->createI32(cmpVal);
                    
                    return builder->createBeq(LLIR::Type::createI32Type(), strcmpCall, cmpValue, destBlock);
                } else if (op->getType() == AstType::Add) {
                    if (rvalStr) {
                        return builder->createCall(LLIR::PointerType::createI8PtrType(), "strcat_str", args);
                    } else {
                        return builder->createCall(LLIR::PointerType::createI8PtrType(), "strcat_char", args);
                    }
                } else {
                    // Invalid
                    return nullptr;
                }
            }
            
            // Otherwise, build a normal comparison
            switch (expr->getType()) {
                case AstType::Add: return builder->createAdd(type, lval, rval);
                case AstType::Sub: return builder->createSub(type, lval, rval);
                case AstType::Mul: return builder->createSMul(type, lval, rval);
                case AstType::Div: return builder->createSDiv(type, lval, rval);
                
                case AstType::And: return builder->createAnd(type, lval, rval);
                case AstType::Or:  return builder->createOr(type, lval, rval);
                case AstType::Xor: return builder->createXor(type, lval, rval);
                    
                case AstType::EQ: return builder->createBeq(type, lval, rval, destBlock);
                case AstType::NEQ: return builder->createBne(type, lval, rval, destBlock);
                case AstType::GT: return builder->createBgt(type, lval, rval, destBlock);
                case AstType::LT: return builder->createBlt(type, lval, rval, destBlock);
                case AstType::GTE: return builder->createBge(type, lval, rval, destBlock);
                case AstType::LTE: return builder->createBle(type, lval, rval, destBlock);
                    
                default: {}
            }
        } break;
        
        default: {}
    }
    
    return nullptr;
}

LLIR::Type *Compiler::translateType(DataType dataType, DataType subType, std::string typeName) {
    LLIR::Type *type;
            
    switch (dataType) {
        case DataType::Char:
        case DataType::I8:
        case DataType::U8: type = LLIR::Type::createI8Type(); break;
        
        case DataType::I16:
        case DataType::U16: type = LLIR::Type::createI16Type(); break;
        
        case DataType::Bool:
        case DataType::I32:
        case DataType::U32: type = LLIR::Type::createI32Type(); break;
        
        case DataType::I64:
        case DataType::U64: type = LLIR::Type::createI64Type(); break;
        
        case DataType::String: type = LLIR::PointerType::createI8PtrType(); break;
        
        case DataType::Ptr: {
            switch (subType) {
                case DataType::Void: type = LLIR::PointerType::createVoidPtrType(); break;
            
                case DataType::Char:
                case DataType::I8:
                case DataType::U8: type = LLIR::PointerType::createI8PtrType(); break;
                
                case DataType::I16:
                case DataType::U16: type = LLIR::PointerType::createI16PtrType(); break;
                
                case DataType::I32:
                case DataType::U32: type = LLIR::PointerType::createI32PtrType(); break;
                
                case DataType::I64:
                case DataType::U64: type = LLIR::PointerType::createI64PtrType(); break;
                
                case DataType::String: type = new LLIR::PointerType(LLIR::PointerType::createI8PtrType()); break;
                
                case DataType::Struct: type = new LLIR::PointerType(structTable[typeName]); break;
                
                default: {}
            }
        } break;
        
        case DataType::Struct: {
            return structTable[typeName];
        } break;
        
        default: type = LLIR::Type::createVoidType();
    }
    
    return type;
}

int Compiler::getStructIndex(std::string name, std::string member) {
    std::string name2 = structVarTable[name];
    if (name2 != "") name = name2;
    
    for (auto s : tree->getStructs()) {
        if (s->getName() != name) continue;

        std::vector<Var> members = s->getItems();
        for (int i = 0; i<members.size(); i++) {
            if (members.at(i).name == member) return i;
        }
    }
    
    return 0;
}


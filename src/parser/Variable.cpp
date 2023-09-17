//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

extern "C" {
#include <lex/lex.h>
}

// Builds a variable declaration
// A variable declaration is composed of an Alloca and optionally, an assignment
bool Parser::buildVariableDec(AstBlock *block) {
    token tk = lex_get_next(scanner);
    std::vector<std::string> toDeclare;
    toDeclare.push_back(lex_get_id(scanner));
    
    if (tk != t_id) {
        syntax->addError(0, "Expected variable name.");
        return false;
    }
    
    tk = lex_get_next(scanner);
    
    while (tk != t_colon) {
        if (tk == t_comma) {
            tk = lex_get_next(scanner);
            
            if (tk != t_id) {
                syntax->addError(0, "Expected variable name.");
                return false;
            }
            
            toDeclare.push_back(lex_get_id(scanner));
        } else if (tk != t_colon) {
            syntax->addError(0, "Invalt_id tk in variable declaration.");
            return false;
        }
        
        tk = lex_get_next(scanner);
    }
    
    AstDataType *dataType = buildDataType(false);
    tk = lex_get_next(scanner);
    
    // We have an array
    if (tk == t_lbracket) {
        dataType = AstBuilder::buildPointerType(dataType);
        std::shared_ptr<AstVarDec> empty = std::make_shared<AstVarDec>("", dataType);
        std::shared_ptr<AstExpression> arg = buildExpression(block, AstBuilder::buildInt32Type(), t_rbracket);
        if (!arg) return false;
        empty->setExpression(arg); 
        
        tk = lex_get_next(scanner);
        if (tk != t_semicolon) {
            syntax->addError(0, "Error: Expected \';\'.");
            return false;
        }
        
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            block->addStatement(vd);
            vd->setExpression(empty->getExpression());
            
            // Create an assignment to a malloc call
            std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
            va->setDataType(dataType);
            block->addStatement(va);
            
            std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
            std::shared_ptr<AstFuncCallExpr> callMalloc = std::make_shared<AstFuncCallExpr>("malloc");
            std::shared_ptr<AstAssignOp> assign = std::make_shared<AstAssignOp>(id, callMalloc);
            
            va->setExpression(assign);
            
            // In order to get a proper malloc, we need to multiply the argument by
            // the size of the type. Get the arguments, and do that
            std::shared_ptr<AstExprList> list = std::make_shared<AstExprList>();
            callMalloc->setArgExpression(list);
            
            std::shared_ptr<AstI32> size;
            AstDataType *baseType = static_cast<AstPointerType *>(dataType)->getBaseType();
            if (baseType->getType() == V_AstType::Int32) size = std::make_shared<AstI32>(4);
            else if (baseType->getType() == V_AstType::Int64) size = std::make_shared<AstI32>(8);
            else if (baseType->getType() == V_AstType::String) size = std::make_shared<AstI32>(8);
            else size = std::make_shared<AstI32>(1);
            
            std::shared_ptr<AstMulOp> op = std::make_shared<AstMulOp>();
            op->setLVal(size);
            op->setRVal(vd->getExpression());
            list->addExpression(op);
            
            // Finally, set the size of the declaration
            vd->setPtrSize(vd->getExpression());
            
            block->addSymbol(name, dataType);
        }
    
    // We're at the end of the declaration
    } else if (tk == t_semicolon) {
        syntax->addError(0, "Expected init expression.");
        return false;
        
    // Otherwise, we have a regular variable
    } else {
        std::shared_ptr<AstExpression> arg = buildExpression(block, dataType);
        if (!arg) return false;
    
        for (std::string name : toDeclare) {
            std::shared_ptr<AstVarDec> vd = std::make_shared<AstVarDec>(name, dataType);
            block->addStatement(vd);
            
            std::shared_ptr<AstID> id = std::make_shared<AstID>(name);
            std::shared_ptr<AstAssignOp> assign = std::make_shared<AstAssignOp>(id, arg);
            
            std::shared_ptr<AstExprStatement> va = std::make_shared<AstExprStatement>();
            va->setDataType(dataType);
            va->setExpression(assign);
            block->addStatement(va);
            
            // Add the variable to the blocks symbol table
            block->addSymbol(name, dataType);
        }
    }
    
    return true;
}

// Builds a variable or an array assignment
bool Parser::buildVariableAssign(AstBlock *block, token t_idToken) {
    AstDataType *dataType = block->getDataType(lex_get_id(scanner));
    
    std::shared_ptr<AstExpression> expr = buildExpression(block, dataType);
    if (!expr) return false;
    
    std::shared_ptr<AstExprStatement> stmt = std::make_shared<AstExprStatement>();
    stmt->setDataType(dataType);
    stmt->setExpression(expr);
    block->addStatement(stmt);
    
    return true;
}

// Builds a constant variable
bool Parser::buildConst(bool isGlobal) {
    token tk = lex_get_next(scanner);
    std::string name = lex_get_id(scanner);
    
    // Make sure we have a name for our constant
    if (tk != t_id) {
        syntax->addError(0, "Expected constant name.");
        return false;
    }
    
    // Syntax check
    tk = lex_get_next(scanner);
    if (tk != t_colon) {
        syntax->addError(0, "Expected \':\' in constant expression.");
        return false;
    }
    
    // Get the data type
    AstDataType *dataType = buildDataType(false);
    
    // Final syntax check
    tk = lex_get_next(scanner);
    if (tk != t_assign) {
        syntax->addError(0, "Expected \'=\' after const assignment.");
        return false;
    }
    
    // Build the expression. We create a dummy statement for this
    std::shared_ptr<AstExpression> expr = buildExpression(nullptr, dataType, t_semicolon, true);
    if (!expr) return false;
    
    // Put it all together
    if (isGlobal) {
        globalConsts[name] = std::pair<AstDataType *, std::shared_ptr<AstExpression>>(dataType, expr);
    } else {
        localConsts[name] = std::pair<AstDataType *, std::shared_ptr<AstExpression>>(dataType, expr);
    }
    
    return true;
}


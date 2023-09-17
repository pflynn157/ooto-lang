//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>

extern "C" {
#include <lex/lex.h>
}

// Called if a conditional statement has only one operand. If it does,
// we have to expand to have two operands before we get down to the
// compiler layer
std::shared_ptr<AstExpression> Parser::checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck) {
    std::shared_ptr<AstExpression> expr = toCheck;
    
    switch (toCheck->getType()) {
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(toCheck);
            AstDataType *dataType = block->getDataType(id->getValue());            
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->setLVal(id);
            
            switch (dataType->getType()) {
                case V_AstType::Bool: eq->setRVal(std::make_shared<AstI32>(1)); break;
                case V_AstType::Int8: eq->setRVal(std::make_shared<AstI8>(1)); break;
                case V_AstType::Int16: eq->setRVal(std::make_shared<AstI16>(1)); break;
                case V_AstType::Int32: eq->setRVal(std::make_shared<AstI32>(1)); break;
                case V_AstType::Int64: eq->setRVal(std::make_shared<AstI64>(1)); break;
                
                default: {}
            }
            
            expr = eq;
        } break;
        
        case V_AstType::I32L: {
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->setLVal(expr);
            eq->setRVal(std::make_shared<AstI32>(1));
            expr = eq;
        } break;
        
        default: {}
    }
    
    return expr;
}

// Builds a conditional statement
bool Parser::buildConditional(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstIfStmt> cond = std::make_shared<AstIfStmt>();
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, t_then);
    if (!arg) return false;
    cond->setExpression(arg);
    block->addStatement(cond);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, cond->getExpression());
    cond->setExpression(expr);
    
    std::shared_ptr<AstBlock> trueBlock = std::make_shared<AstBlock>();
    trueBlock->mergeSymbols(block);
    cond->setTrueBlock(trueBlock);
    
    std::shared_ptr<AstBlock> falseBlock = std::make_shared<AstBlock>();
    falseBlock->mergeSymbols(block);
    cond->setFalseBlock(falseBlock);
    buildBlock(trueBlock, cond);
    
    return true;
}

// Builds a while statement
bool Parser::buildWhile(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstWhileStmt> loop = std::make_shared<AstWhileStmt>();
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, t_do);
    if (!arg) return false;
    loop->setExpression(arg);
    block->addStatement(loop);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, loop->getExpression());
    loop->setExpression(expr);
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->setBlock(block2);
    
    return true;
}

// Builds a loop keyword
bool Parser::buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak) {
    if (isBreak) block->addStatement(std::make_shared<AstBreak>());
    else block->addStatement(std::make_shared<AstContinue>());
    
    token tk = lex_get_next(scanner);
    if (tk != t_semicolon) {
        syntax->addError(0, "Expected \';\' after break or continue.");
        return false;
    }
    
    return true;
}


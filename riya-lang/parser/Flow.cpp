//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <memory>

#include <parser/Parser.hpp>
#include <ast/ast.hpp>
#include <lex/lex.hpp>

// Called if a conditional statement has only one operand. If it does,
// we have to expand to have two operands before we get down to the
// compiler layer
std::shared_ptr<AstExpression> Parser::checkCondExpression(std::shared_ptr<AstBlock> block, std::shared_ptr<AstExpression> toCheck) {
    std::shared_ptr<AstExpression> expr = toCheck;
    
    switch (toCheck->type) {
        case V_AstType::ID: {
            std::shared_ptr<AstID> id = std::static_pointer_cast<AstID>(toCheck);
            std::shared_ptr<AstDataType> dataType = block->getDataType(id->value);            
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->lval = id;
            
            switch (dataType->type) {
                case V_AstType::Bool: eq->rval = std::make_shared<AstInt>(1); break;
                case V_AstType::Int8: eq->rval = std::make_shared<AstInt>(1, 8); break;
                case V_AstType::Int16: eq->rval = std::make_shared<AstInt>(1, 16); break;
                case V_AstType::Int32: eq->rval = std::make_shared<AstInt>(1); break;
                case V_AstType::Int64: eq->rval = std::make_shared<AstInt>(1, 64); break;
                
                default: {}
            }
            
            expr = eq;
        } break;
        
        case V_AstType::IntL: {
            std::shared_ptr<AstEQOp> eq = std::make_shared<AstEQOp>();
            eq->lval = expr;
            eq->rval = std::make_shared<AstInt>(1);
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
    cond->expression = arg;
    block->addStatement(cond);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, cond->expression);
    cond->expression = expr;
    
    std::shared_ptr<AstBlock> true_block = std::make_shared<AstBlock>();
    true_block->mergeSymbols(block);
    cond->true_block = true_block;
    
    std::shared_ptr<AstBlock> false_block = std::make_shared<AstBlock>();
    false_block->mergeSymbols(block);
    cond->false_block = false_block;
    buildBlock(true_block, cond);
    
    return true;
}

// Builds a while statement
bool Parser::buildWhile(std::shared_ptr<AstBlock> block) {
    std::shared_ptr<AstWhileStmt> loop = std::make_shared<AstWhileStmt>();
    std::shared_ptr<AstExpression> arg = buildExpression(block, nullptr, t_do);
    if (!arg) return false;
    loop->expression = arg;
    block->addStatement(loop);
    
    std::shared_ptr<AstExpression> expr = checkCondExpression(block, loop->expression);
    loop->expression = expr;
    
    std::shared_ptr<AstBlock> block2 = std::make_shared<AstBlock>();
    block2->mergeSymbols(block);
    buildBlock(block2);
    loop->block = block2;
    
    return true;
}

// Builds a loop keyword
bool Parser::buildLoopCtrl(std::shared_ptr<AstBlock> block, bool isBreak) {
    if (isBreak) block->addStatement(std::make_shared<AstBreak>());
    else block->addStatement(std::make_shared<AstContinue>());
    consume_token(t_semicolon, "Expected \';\' after break or continue.");
    return true;
}


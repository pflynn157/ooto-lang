//
// Copyright 2021 Patrick Flynn
// This file is part of the Espresso compiler.
// Espresso is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <memory>

#include <parser/Parser.hpp>

Parser::Parser(std::string input) : BaseParser(input) {
    scanner = std::make_unique<Scanner>(input);
}

bool Parser::parse() {
    Token token;
    do {
        token = scanner->getNext();
        bool code = true;
        
        switch (token.type) {
            case Public:
            case Protected:
            case Private:
            case Routine:
            case Func: {
                code = buildFunction(token);
            } break;
            
            case Const: code = buildConst(true); break;
            case Enum: code = buildEnum(); break;
            
            case Eof:
            case Nl: break;
            
            default: {
                syntax->addError(scanner->getLine(), "Invalid token in global scope.");
                token.print();
                code = false;
            }
        }
        
        if (!code) break;
    } while (token.type != Eof);
    
    // Check for errors, and print if so
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    syntax->printWarnings();
    return true;
}

// Builds a statement block
bool Parser::buildBlock(AstBlock *block, int stopLayer, AstIfStmt *parentBlock, bool inElif) {
    Token token = scanner->getNext();
    while (token.type != Eof) {
        bool code = true;
        bool end = false;
        
        switch (token.type) {
            case VarD: code = buildVariableDec(block); break;
            case Const: code = buildConst(false); break;
            
            case Id: {
                Token idToken = token;
                token = scanner->getNext();
                
                if (token.type == Assign) {
                    code = buildVariableAssign(block, idToken);
                } else if (token.type == LBracket) {
                    code = buildArrayAssign(block, idToken);
                } else if (token.type == LParen) {
                    Token varToken;
                    varToken.type = EmptyToken;
                    code = buildFunctionCallStmt(block, idToken, varToken);
                } else if (token.type == Dot) {
                    Token memberToken = scanner->getNext();
                    if (memberToken.type != Id) {
                        syntax->addError(scanner->getLine(), "Expected member name.");
                    }
                    
                    token = scanner->getNext();
                    if (token.type == LParen) {
                        code = buildFunctionCallStmt(block, memberToken, idToken);
                    }
                    // TODO: Catch others
                } else {
                    syntax->addError(scanner->getLine(), "Invalid use of identifier.");
                    token.print();
                    return false;
                }
            } break;
            
            case Return: code = buildReturn(block); break;
            
            // Handle conditionals
            // Yes, ELIF and ElSE are similar, but if you look closely, there is a subtle
            // difference (one very much needed)
            case If: code = buildConditional(block); break;
            case Elif: {
                if (inElif) {
                    scanner->rewind(token);
                    end = true;
                } else {
                    code = buildElif(parentBlock);
                }
            } break;
            case Else: {
                if (inElif) {
                    scanner->rewind(token);
                    end = true;
                } else {
                    code = buildElse(parentBlock);
                    end = true;
                }
            } break;
            
            // Handle loops
            case While: code = buildWhile(block); break;
            case Repeat: code = buildRepeat(block); break;
            case For: code = buildFor(block); break;
            case ForAll: code = buildForAll(block); break;
            
            case Break: code = buildLoopCtrl(block, true); break;
            case Continue: code = buildLoopCtrl(block, false); break;
            
            // Handle the END keyword
            // This is kind of tricky in conditionals
            case End: {
                if (inElif) {
                    scanner->rewind(token);
                    end = true;
                    break;
                }
                if (layer == stopLayer) {
                    end = true;
                }
                if (layer > 0) --layer;
            } break;
            
            case Nl: break;
            
            default: {
                syntax->addError(scanner->getLine(), "Invalid token in expression.");
                token.print();
                return false;
            }
        }
        
        if (end) break;
        if (!code) return false;
        token = scanner->getNext();
    }
    
    return true;
}

// Builds an expression
bool Parser::buildExpression(AstStatement *stmt, DataType currentType, TokenType stopToken, TokenType separateToken,
                             AstExpression **dest, bool isConst) {
    std::stack<AstExpression *> output;
    std::stack<AstExpression *> opStack;
    
    DataType varType = currentType;
    
    bool lastWasOp = true;

    Token token = scanner->getNext();
    while (token.type != Eof && token.type != stopToken) {
        if (token.type == separateToken && output.size() > 0) {
            AstExpression *expr = output.top();
            output.pop();
            
            if (stmt == nullptr) {
                if ((*dest)->getType() == AstType::FuncCallExpr) {
                    AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(*dest);
                    fc->addArgument(expr);
                } else {
                    *dest = expr;
                }
            } else {
                stmt->addExpression(expr);
            }
            continue;
        }
    
        switch (token.type) {
            case True: {
                lastWasOp = false;
                output.push(new AstBool(1));
            } break;
            
            case False: {
                lastWasOp = false;
                output.push(new AstBool(0));
            } break;
            
            case CharL: {
                lastWasOp = false;
                AstChar *c = new AstChar(token.i8_val);
                output.push(c);
            } break;
            
            case Int32: {
                lastWasOp = false;
                AstInt *i32 = new AstInt(token.i32_val);
                output.push(i32);
            } break;
            
            case String: {
                lastWasOp = false;
                AstString *str = new AstString(token.id_val);
                output.push(str);
            } break;
            
            case Id: {
                lastWasOp = false;
                
                if (isConst) {
                    syntax->addError(scanner->getLine(), "Invalid constant value.");
                    return false;
                }
            
                std::string name = token.id_val;
                if (varType == DataType::Void) {
                    varType = typeMap[name].first;
                    if (varType == DataType::Array) varType = typeMap[name].second;
                }
                
                token = scanner->getNext();
                if (token.type == LBracket) {
                    AstExpression *index = nullptr;
                    buildExpression(nullptr, DataType::Int32, RBracket, EmptyToken, &index);
                    
                    AstArrayAccess *acc = new AstArrayAccess(name);
                    acc->setIndex(index);
                    output.push(acc);
                } else if (token.type == LParen) {
                    AstFuncCallExpr *fc = new AstFuncCallExpr(name);
                    AstExpression *fcExpr = fc;
                    buildExpression(nullptr, varType, RParen, Comma, &fcExpr);
                    
                    output.push(fc);
                } else if (token.type == Scope) {
                    if (enums.find(name) == enums.end()) {
                        syntax->addError(scanner->getLine(), "Unknown enum.");
                        return false;
                    }
                    
                    token = scanner->getNext();
                    if (token.type != Id) {
                        syntax->addError(scanner->getLine(), "Expected identifier.");
                        return false;
                    }
                    
                    EnumDec dec = enums[name];
                    AstExpression *val = dec.values[token.id_val];
                    output.push(val);
                } else {
                    int constVal = isConstant(name);
                    if (constVal > 0) {
                        if (constVal == 1) {
                            AstExpression *expr = globalConsts[name].second;
                            output.push(expr);
                        } else if (constVal == 2) {
                            AstExpression *expr = localConsts[name].second;
                            output.push(expr);
                        }
                    } else {
                        AstID *id = new AstID(name);
                        output.push(id);
                    }
                    
                    scanner->rewind(token);
                }
            } break;
            
            case Sizeof: {
                lastWasOp = false;
                
                if (isConst) {
                    syntax->addError(scanner->getLine(), "Invalid constant value.");
                    return false;
                }
                
                std::string name = token.id_val;
                
                Token token1 = scanner->getNext();
                Token token2 = scanner->getNext();
                Token token3 = scanner->getNext();
                
                if (token1.type != LParen || token2.type != Id || token3.type != RParen) {
                    syntax->addError(scanner->getLine(), "Invalid token in sizeof.");
                    token.print();
                    return false;
                }
                
                AstID *id = new AstID(token2.id_val);
                AstSizeof *size = new AstSizeof(id);
                output.push(size);
            } break;
            
            case Plus: 
            case Minus:
            case And:
            case Or:
            case Xor:
            case Lsh:
            case Rsh: {
                if (opStack.size() > 0) {
                    AstType type = opStack.top()->getType();
                    if (type == AstType::Mul || type == AstType::Div) {
                        AstExpression *rval = checkExpression(output.top(), varType);
                        output.pop();
                        
                        AstExpression *lval = checkExpression(output.top(), varType);
                        output.pop();
                        
                        AstBinaryOp *op = static_cast<AstBinaryOp *>(opStack.top());
                        opStack.pop();
                        
                        op->setLVal(lval);
                        op->setRVal(rval);
                        output.push(op);
                    }
                }
                
                if (token.type == Plus) {
                    AstAddOp *add = new AstAddOp;
                    opStack.push(add);
                } else if (token.type == And) {
                    opStack.push(new AstAndOp);
                } else if (token.type == Or) {
                    opStack.push(new AstOrOp);
                } else if (token.type == Xor) {
                    opStack.push(new AstXorOp);
                } else if (token.type == Lsh) {
                    opStack.push(new AstLshOp);
                } else if (token.type == Rsh) {
                    opStack.push(new AstRshOp);
                } else {
                    if (lastWasOp) {
                        opStack.push(new AstNegOp);
                    } else {
                        AstSubOp *sub = new AstSubOp;
                        opStack.push(sub);
                    }
                }
                
                lastWasOp = true;
            } break;
            
            case Mul: {
                lastWasOp = true;
                AstMulOp *mul = new AstMulOp;
                opStack.push(mul);
            } break;
            
            case Div: {
                lastWasOp = true;
                AstDivOp *div = new AstDivOp;
                opStack.push(div);
            } break;
            
            case Mod: {
                lastWasOp = true;
                AstRemOp *rem = new AstRemOp;
                opStack.push(rem);
            } break;
            
            case EQ: opStack.push(new AstEQOp); lastWasOp = true; break;
            case NEQ: opStack.push(new AstNEQOp); lastWasOp = true; break;
            case GT: opStack.push(new AstGTOp); lastWasOp = true; break;
            case LT: opStack.push(new AstLTOp); lastWasOp = true; break;
            case GTE: opStack.push(new AstGTEOp); lastWasOp = true; break;
            case LTE: opStack.push(new AstLTEOp); lastWasOp = true; break;
            
            case Step: {
                lastWasOp = false;       
                
                if (stmt->getType() != AstType::For) {
                    syntax->addError(scanner->getLine(), "Step is only valid with for loops");
                    return false;
                }
                
                token = scanner->getNext();
                if (token.type != Int32) {
                    syntax->addError(scanner->getLine(), "Expected integer literal with \"step\"");
                    return false;
                }
                
                AstForStmt *forStmt = static_cast<AstForStmt *>(stmt);
                forStmt->setStep(token.i32_val);
            } break;
            
            default: {}
        }
        
        if (!lastWasOp && opStack.size() > 0) {
            if (opStack.top()->getType() == AstType::Neg) {
                AstExpression *val = checkExpression(output.top(), varType);
                output.pop();
                
                AstNegOp *op = static_cast<AstNegOp *>(opStack.top());
                opStack.pop();
                op->setVal(val);
                output.push(op);
            }
        }
        
        token = scanner->getNext();
    }
    
    // Build the expression
    while (opStack.size() > 0) {
        AstExpression *rval = checkExpression(output.top(), varType);
        output.pop();
        
        AstExpression *lval = checkExpression(output.top(), varType);
        output.pop();
        
        AstBinaryOp *op = static_cast<AstBinaryOp *>(opStack.top());
        opStack.pop();
        
        op->setLVal(lval);
        op->setRVal(rval);
        output.push(op);
    }
    
    // Add the expressions back
    if (output.size() == 0) {
        return true;
    }
    
    // Type check the top
    AstExpression *expr = checkExpression(output.top(), varType);
    
    if (stmt == nullptr) {
        if ((*dest) == nullptr) {
            *dest = expr;
        } else if ((*dest)->getType() == AstType::FuncCallExpr) {
            AstFuncCallExpr *fc = static_cast<AstFuncCallExpr *>(*dest);
            fc->addArgument(expr);
        } else {
            *dest = expr;
        }
    } else {
        stmt->addExpression(expr);
    }
    
    return true;
}


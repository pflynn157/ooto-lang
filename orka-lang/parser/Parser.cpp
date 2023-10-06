//
// Copyright 2021-2022 Patrick Flynn
// This file is part of the Tiny Lang compiler.
// Tiny Lang is licensed under the BSD-3 license. See the COPYING file for more information.
//
#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>

#include <parser/Parser.hpp>
#include <ast/ast_builder.hpp>
#include <lex/lex.hpp>

Parser::Parser(std::string input) : BaseParser(input) {
    scanner = std::make_unique<Scanner>(input);
    
    // Add the built-in functions
    //string malloc(string)
    tree->block->funcs.push_back("malloc");
    std::shared_ptr<AstExternFunction> FT1 = std::make_shared<AstExternFunction>("malloc");
    FT1->addArgument(Var(AstBuilder::buildInt32Type(), "size"));
    FT1->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT1);
    
    //println(string)
    /*funcs.push_back("println");
    std::shared_ptr<AstExternFunction> FT2 = std::make_shared<AstExternFunction>("println");
    FT2->varargs = true;
    FT2->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT2->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(FT2);*/
    
    //print(string)
    tree->block->funcs.push_back("print");
    std::shared_ptr<AstExternFunction> FT3 = std::make_shared<AstExternFunction>("print");
    FT3->varargs = true;
    FT3->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT3->data_type = AstBuilder::buildVoidType();
    tree->addGlobalStatement(FT3);
    
    //i32 strlen(string)
    tree->block->funcs.push_back("strlen");
    std::shared_ptr<AstExternFunction> FT4 = std::make_shared<AstExternFunction>("strlen");
    FT4->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT4->data_type = AstBuilder::buildInt32Type();
    tree->addGlobalStatement(FT4);
    
    //i32 stringcmp(string, string)
    tree->block->funcs.push_back("stringcmp");
    std::shared_ptr<AstExternFunction> FT5 = std::make_shared<AstExternFunction>("stringcmp");
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT5->data_type = AstBuilder::buildInt32Type();
    tree->addGlobalStatement(FT5);
    
    //string strcat_str(string, string)
    tree->block->funcs.push_back("strcat_str");
    std::shared_ptr<AstExternFunction> FT6 = std::make_shared<AstExternFunction>("strcat_str");
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT6->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT6);
    
    //string strcat_char(string, char)
    tree->block->funcs.push_back("strcat_char");
    std::shared_ptr<AstExternFunction> FT7 = std::make_shared<AstExternFunction>("strcat_char");
    FT7->addArgument(Var(AstBuilder::buildStringType(), "str"));
    FT7->addArgument(Var(AstBuilder::buildCharType(), "c"));
    FT7->data_type = AstBuilder::buildStringType();
    tree->addGlobalStatement(FT7);
}

Parser::~Parser() {
}

bool Parser::parse() {
    Token tk;
    do {
        tk = scanner->getNext();
        bool code = true;
        
        switch (tk.type) {
            case t_extern:
            case t_func: {
                code = buildFunction(tk);
            } break;
            
            case t_const: code = buildConst(tree->block, true); break;
            case t_struct: code = buildStruct(); break;
            
            case t_eof: break;
            
            default: {
                syntax->addError(0, "Invalid token in global scope.");
                tk.print();
                code = false;
            }
        }
        
        if (!code) break;
    } while (tk.type != t_eof);
    
    // Check for errors, and print if so
    if (syntax->errorsPresent()) {
        syntax->printErrors();
        return false;
    }
    
    syntax->printWarnings();
    return true;
}

// Builds a statement block
bool Parser::buildBlock(std::shared_ptr<AstBlock> block, std::shared_ptr<AstNode> parent) {
    Token tk = scanner->getNext();
    while (tk.type != t_end && tk.type != t_eof) {
        bool code = true;
        bool end = false;
        
        switch (tk.type) {
            case t_var: code = buildVariableDec(block); break;
            case t_struct: code = buildStructDec(block); break;
            case t_const: code = buildConst(block, false); break;
            
            case t_id: {
                Token idtoken = tk;
                tk = scanner->getNext();
                
                if (tk.type == t_assign || tk.type == t_lbracket || tk.type == t_dot) {
                    scanner->rewind(tk);
                    scanner->rewind(idtoken);
                    code = buildVariableAssign(block, idtoken);
                } else if (tk.type == t_lparen) {
                    code = buildFunctionCallStmt(block, idtoken);
                } else {
                    syntax->addError(0, "Invalid use of identifier.");
                    //token.print();
                    return false;
                }
            } break;
            
            case t_return: code = buildReturn(block); break;
            
            // Handle conditionals
            case t_if: code = buildConditional(block); break;
            case t_elif: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                code = buildConditional(condParent->false_block);
                end = true;
            } break;
            case t_else: {
                std::shared_ptr<AstIfStmt> condParent = std::static_pointer_cast<AstIfStmt>(parent);
                buildBlock(condParent->false_block);
                end = true;
            } break;
            
            // Handle loops
            case t_while: code = buildWhile(block); break;
            case t_break: code = buildLoopCtrl(block, true); break;
            case t_continue: code = buildLoopCtrl(block, false); break;
            
            default: {
                syntax->addError(0, "Invalid token in block.");
                //token.print();
                return false;
            }
        }
        
        if (end) break;
        if (!code) return false;
        tk = scanner->getNext();
    }
    
    return true;
}

// The debug function for the scanner
void Parser::debugScanner() {
    std::cout << "Debugging scanner..." << std::endl;
    
    Token t;
    do {
        t = scanner->getNext();
        t.print();
    } while (t.type != t_eof);
}

//
// Builds a data type from the token stream
//
std::shared_ptr<AstDataType> Parser::buildDataType(bool checkBrackets) {
    Token tk = scanner->getNext();
    std::shared_ptr<AstDataType> dataType = nullptr;
    
    switch (tk.type) {
        case t_bool: dataType = AstBuilder::buildBoolType(); break;
        case t_char: dataType = AstBuilder::buildCharType(); break;
        case t_i8: dataType = AstBuilder::buildInt8Type(); break;
        case t_u8: dataType = AstBuilder::buildInt8Type(true); break;
        case t_i16: dataType = AstBuilder::buildInt16Type(); break;
        case t_u16: dataType = AstBuilder::buildInt16Type(true); break;
        case t_i32: dataType = AstBuilder::buildInt32Type(); break;
        case t_u32: dataType = AstBuilder::buildInt32Type(true); break;
        case t_i64: dataType = AstBuilder::buildInt64Type(); break;
        case t_u64: dataType = AstBuilder::buildInt64Type(true); break;
        case t_string: dataType = AstBuilder::buildStringType(); break;
        
        case t_id: {
            bool isStruct = false;
            for (auto s : tree->structs) {
                if (s->name == tk.id_val) {
                    isStruct = true;
                    break;
                }
            }
                
            if (isStruct) {
                dataType = AstBuilder::buildStructType(tk.id_val);
            }
        } break;
        
        default: {}
    }

    if (checkBrackets) {
        tk = scanner->getNext();
        if (tk.type == t_lbracket) {
            tk = scanner->getNext();
            if (tk.type != t_rbracket) {
                syntax->addError(0, "Invalid pointer type.");
                return nullptr;
            }
            
            dataType = AstBuilder::buildPointerType(dataType);
        } else {
            scanner->rewind(tk);
        }
    }
    
    return dataType;
}


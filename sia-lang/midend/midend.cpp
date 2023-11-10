#include <memory>
#include <iostream>

#include <ast/ast_builder.hpp>

#include "midend.hpp"

Midend::Midend(std::shared_ptr<AstTree> tree) {
    this->parse_tree = tree;
    this->tree = std::make_shared<AstTree>(parse_tree->file);
}

void Midend::run() {
    it_process_block(parse_tree->block, tree->block);
}

void Midend::it_process_block(std::shared_ptr<AstBlock> &block, std::shared_ptr<AstBlock> &new_block) {
    new_block->mergeSymbols(block);

    for (auto const &stmt : block->block) {
        switch (stmt->type) {
            // Function call statements
            case V_AstType::FuncCallStmt: {
                auto fc = std::static_pointer_cast<AstFuncCallStmt>(stmt);
                if (fc->name == "print") {
                    process_print(fc, new_block);
                } else {
                    new_block->addStatement(fc);
                }
            } break;
        
            // Block statements have to be processed separately
            case V_AstType::Func: {
                auto func = std::static_pointer_cast<AstFunction>(stmt);
                auto func2 = std::make_shared<AstFunction>(func->name, func->data_type);
                func2->args = func->args;
                it_process_block(func->block, func2->block);
                new_block->addStatement(func2);
            } break;
        
            // By default, add the statement to the new block
            default: {
                new_block->addStatement(stmt);
            }
        }
    }
}

//
// Converts the builtin-print to a printf call
// This is similar to the one in the other compilers, but here we convert to printf rather than
// a built-in library call.
//
void Midend::process_print(std::shared_ptr<AstFuncCallStmt> &call, std::shared_ptr<AstBlock> &block) {
    auto args = call->expression;
    call->name = "printf";
    std::string fmt = "";
    
    for (auto expr : std::static_pointer_cast<AstExprList>(args)->list) {
        switch (expr->type) {
            case V_AstType::CharL: fmt += "%c"; break;
            case V_AstType::I8L:
            case V_AstType::I16L:
            case V_AstType::I32L:
            case V_AstType::I64L: fmt += "%d"; break;
            case V_AstType::StringL: fmt += "%s"; break;
            
            case V_AstType::ArrayAccess:
            case V_AstType::StructAccess:
            case V_AstType::ID: {
                std::shared_ptr<AstDataType> dtype;
                if (expr->type == V_AstType::ArrayAccess) {
                    std::string name = std::static_pointer_cast<AstArrayAccess>(expr)->value;
                    dtype = block->getDataType(name);
                    dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                } else if (expr->type == V_AstType::StructAccess) {
                    auto sa = std::static_pointer_cast<AstStructAccess>(expr);
                    auto sa_type = std::static_pointer_cast<AstStructType>(block->getDataType(sa->var));
                    for (auto str : tree->structs) {
                        if (str->name == sa_type->name) {
                            for (auto v_item : str->items) {
                                if (v_item.name == sa->member) {
                                    dtype = v_item.type;
                                }
                            }
                        }
                    }
                    if (dtype->type == V_AstType::Ptr) {
                        dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                    }
                } else {
                    std::string name = std::static_pointer_cast<AstID>(expr)->value;
                    dtype = block->getDataType(name);
                    if (dtype->type == V_AstType::Ptr) {
                        dtype = std::static_pointer_cast<AstPointerType>(dtype)->base_type;
                    }
                }
                
                switch (dtype->type) {
                    case V_AstType::Bool: fmt += "%i"; break;
                    case V_AstType::Char: fmt += "%c"; break;
                    case V_AstType::Int8:
                    case V_AstType::Int16:
                    case V_AstType::Int32:
                    case V_AstType::Int64: fmt += "%d"; break;
                    case V_AstType::String: fmt += "%s"; break;
                    default: {}
                }
            } break;
            
            default: {}
        }
    }
    
    // Add the end, print a newline
    fmt += "\n";
    
    // Add the format
    auto args2 = std::static_pointer_cast<AstExprList>(args);
    std::shared_ptr<AstString> fmt_str = std::make_shared<AstString>(fmt);
    args2->list.insert(args2->list.begin(), fmt_str);
    
    // Add it back to the block
    block->addStatement(call);
}


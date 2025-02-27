#include <iostream>

#include <ast/ast.hpp>
#include <ast/ast_builder.hpp>

#include "interpreter.hpp"

//
// For running functions
//
vm_arg_list AstInterpreter::run_function(std::shared_ptr<AstFunction> func, std::vector<vm_arg_list> args) {   
    auto ctx = std::make_shared<IntrContext>();
    ctx->func_type = func->data_type;
    
    // Merge arguments into the symbol table
    for (int i = 0; i<func->args.size(); i++) {
        auto arg = func->args[i];
        // Arrays
        if (arg.type->type == V_AstType::Ptr) {
            auto ptr = std::static_pointer_cast<AstPointerType>(arg.type);
            ctx->type_map[arg.name] = ptr->base_type;
            
            if (is_int_type(ptr->base_type)) {
                ctx->iarray_map[arg.name] = *std::get_if<std::vector<uint64_t>>(&args[i]);
            } else if (is_float_type(ptr->base_type)) {
            
            } else if (is_string_type(ptr->base_type)) {
                ctx->sarray_map[arg.name] = *std::get_if<std::vector<std::string>>(&args[i]);
            }
            
        // Scalar variables
        } else {
            ctx->type_map[arg.name] = arg.type;
            
            if (is_int_type(arg.type)) {
                ctx->ivar_map[arg.name] = *std::get_if<uint64_t>(&args[i]);
            } else if (is_float_type(arg.type)) {
            
            } else if (is_string_type(arg.type)) {
                ctx->svar_map[arg.name] = *std::get_if<std::string>(&args[i]);
            }
        }
    }
    
    // Run the block
    run_block(ctx, func->block);
    
    // At the end, check the stack
    if (is_int_type(func->data_type)) {
        if (func->data_type->type == V_AstType::Ptr) {
            std::string name = ctx->sstack.top();
            return ctx->iarray_map[name];
        }
        if (ctx->istack.empty()) return (uint64_t)0;
        return ctx->istack.top();
    } else if (is_float_type(func->data_type)) {
    
    } else if (is_string_type(func->data_type)) {
        std::string value = "";
        if (func->data_type->type == V_AstType::Ptr) {
            std::string name = ctx->sstack.top();
            return ctx->sarray_map[name];
        }
        if (!ctx->sstack.empty()) {
            value = ctx->sstack.top();
        }
        return value;
    }
    
    return (uint64_t)0;
}

vm_arg_list AstInterpreter::call_function(std::shared_ptr<IntrContext> ctx, std::string name, std::shared_ptr<AstExprList> args) {
    // Handle the "length" call for arrays and strings
    if (name == "length") {
        auto arg1 = args->list[0];
        if (arg1->type == V_AstType::ID) {
            auto id = std::static_pointer_cast<AstID>(arg1);
            if (is_int_array(ctx, id->value)) {
                return (uint64_t)ctx->iarray_map[id->value].size();
            } else if (is_float_array(ctx, id->value)) {
            
            } else if (is_string_array(ctx, id->value)) {
                return (uint64_t)ctx->sarray_map[id->value].size();
            } else if (ctx->type_map[id->value]->type == V_AstType::String) {
                return (uint64_t)ctx->svar_map[id->value].length();
            }
        } else if (arg1->type == V_AstType::StringL) {
            auto s = std::static_pointer_cast<AstString>(arg1);
            return (uint64_t)s->value.length();
        }
    }
    
    // Otherwise, pull from the table
    auto func = function_map[name];
    std::vector<vm_arg_list> addrs;
    
    // TODO: Check type
    for (int i = 0; i<args->list.size(); i++) {
        auto arg = args->list[i];
        auto data_type = func->args[i].type;
        
        // Pointers
        if (data_type->type == V_AstType::Ptr) {
            auto base_type = std::static_pointer_cast<AstPointerType>(data_type)->base_type;
            auto id = std::static_pointer_cast<AstID>(arg);
            if (is_int_type(base_type)) {
                addrs.push_back(ctx->iarray_map[id->value]);
            } else if (is_float_type(base_type)) {
            
            } else if (is_string_type(base_type)) {
                addrs.push_back(ctx->sarray_map[id->value]);
            }
            
        // Everything else
        } else {
            if (is_int_type(data_type)) {
                run_iexpression(ctx, arg);
                uint64_t value = ctx->istack.top();
                ctx->istack.pop();
                addrs.push_back(value);
            } else if (is_float_type(data_type)) {
            
            } else if (is_string_type(data_type)) {
                run_sexpression(ctx, arg);
                std::string value = ctx->sstack.top();
                ctx->sstack.pop();
                addrs.push_back(value);
            }
        }
    }
    
    // Run it
    return run_function(func, addrs);
}

//
// Runs the builtin print call
//
void AstInterpreter::run_print(std::shared_ptr<IntrContext> ctx, std::shared_ptr<AstExprList> args) {
    for (auto const &arg : args->list) {
        switch (arg->type) {
            // Print a string literal
            case V_AstType::StringL: {
                auto s = std::static_pointer_cast<AstString>(arg);
                std::cout << s->value;
            } break;
            
            // Print a character literal
            case V_AstType::CharL: {
                auto c = std::static_pointer_cast<AstChar>(arg);
                std::cout << c->value;
            } break;
            
            // Print an integer literal
            case V_AstType::IntL: {
                auto i = std::static_pointer_cast<AstInt>(arg);
                std::cout << i->value;
            } break;
            
            // Identifier
            // TODO: Eventually clean this up
            case V_AstType::ID: {
                auto id = std::static_pointer_cast<AstID>(arg);
                auto data_type = ctx->type_map[id->value];
                
                // Integers
                if (is_int_type(data_type)) {
                    if (is_int_array(ctx, id->value)) {
                        auto array = ctx->iarray_map[id->value];
                        std::cout << "[";
                        for (int i = 0; i<array.size(); i++) {
                            std::cout << array[i];
                            if (i+1 < array.size()) std::cout << ", ";
                        }
                        std::cout << "]";
                    } else {
                        std::cout << ctx->ivar_map[id->value];
                    }
                
                // Floatss
                } else if (is_float_type(data_type)) {
                
                // Strings
                } else if (is_string_type(data_type)) {
                    if (is_string_array(ctx, id->value)) {
                        auto array = ctx->sarray_map[id->value];
                        std::cout << "[";
                        for (int i = 0; i<array.size(); i++) {
                            std::cout << "\"" << array[i] << "\"";
                            if (i+1 < array.size()) std::cout << ", ";
                        }
                        std::cout << "]";
                    } else {
                        std::cout << ctx->svar_map[id->value];
                    }
                }
            } break;
            
            // Array access
            case V_AstType::ArrayAccess: {
                auto acc = std::static_pointer_cast<AstArrayAccess>(arg);
                run_iexpression(ctx, acc->index);
                int idx = ctx->istack.top();
                ctx->istack.pop();
                
                if (is_int_array(ctx, acc->value)) {
                    std::cout << ctx->iarray_map[acc->value][idx];
                } else if (is_float_array(ctx, acc->value)) {
                
                } else if (is_string_array(ctx, acc->value)) {
                    std::cout << ctx->sarray_map[acc->value][idx];
                } else if (ctx->type_map[acc->value]->type == V_AstType::String) {
                    std::cout << ctx->svar_map[acc->value][idx];
                }
            } break;
            
            // Function call expression
            // TODO: Type checking
            case V_AstType::FuncCallExpr: {
                auto fc = std::static_pointer_cast<AstFuncCallExpr>(arg);
                auto value = call_function(ctx, fc->name, std::static_pointer_cast<AstExprList>(fc->args));
                
                // The length function
                if (fc->name == "length") {
                    std::cout << *std::get_if<uint64_t>(&value);
                    
                // All other functions
                } else {
                    auto func_type = function_map[fc->name]->data_type;
                    if (is_int_type(func_type)) {
                        std::cout << *std::get_if<uint64_t>(&value);
                    } else if (is_float_type(func_type)) {
                    
                    } else if (is_string_type(func_type)) {
                        std::cout << *std::get_if<std::string>(&value);
                    }
                }
            } break;
            
            // Print a binary operation
            case V_AstType::Add:
            case V_AstType::Sub:
            case V_AstType::Mul:
            case V_AstType::Div:
            case V_AstType::Mod:
            case V_AstType::And:
            case V_AstType::Or:
            case V_AstType::Xor:
            case V_AstType::Lsh:
            case V_AstType::Rsh:
            case V_AstType::EQ:
            case V_AstType::NEQ:
            case V_AstType::GT:
            case V_AstType::LT:
            case V_AstType::GTE:
            case V_AstType::LTE:
            {
                auto data_type = interpret_type(ctx, arg);
                if (data_type == nullptr) {
                    std::cout << "[ERR:<UNK_TYPE>]";
                    break;
                }
                
                run_expression(ctx, arg, data_type);
                if (data_type->type == V_AstType::Int32) {
                    std::cout << ctx->istack.top();
                    ctx->istack.pop();
                }
                // TODO: Other types here
            } break;
            
            default: {
                std::cout << "[ERR:INVALID_EXPR]: ";
                arg->print();
            }
        }
    }
    
    std::cout << std::endl;
}


#pragma once

#include <string>
#include <fstream>
#include <stack>

//
// Token definitions
//
enum token {
    t_none,
    t_eof,
    
    // Keywords
    t_func,
    t_return,
    t_of,
    t_end,
    t_def,
    t_extern,
    
    // Datatype keywords
    t_string,
    
    // Symbols
    t_dot,
    t_lcbrace,
    t_rcbrace,
    t_numsym,
    t_colon,
    
    // Literals
    t_id,
    t_int_literal,
    t_string_literal,
};

//
// The lexical analyzer
//
struct Lex {
    explicit Lex(std::string input);
    token get_next();
    bool is_symbol(char c);
    token get_symbol(char c);
    bool is_int();
    void print(token t);
    
    // Accessible member variables
    std::string value = "";
    std::stack<token> stack;
    
private:
    std::ifstream reader;
    std::string buffer = "";
};


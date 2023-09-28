#include <iostream>
#include <cctype>

#include "lex.hpp"

//
// Initialize the lexer
//
Lex::Lex(std::string input) {
    reader = std::ifstream(input);
}

//
// Get the next token in the stream
//
token Lex::get_next() {
    if (!stack.empty()) {
        auto t = stack.top();
        stack.pop();
        return t;
    }
    
    if (reader.eof()) {
        return t_eof;
    }
    
    while (!reader.eof()) {
        char c = reader.get();
        if (c == ' ' || is_symbol(c)) {
            if (is_symbol(c)) {
                token t = get_symbol(c);
                if (buffer.length() > 0) {
                    stack.push(t);
                } else {
                    return t;
                }
            }
            
            if (buffer.length() == 0) continue;
            
            token t = t_none;
            if (buffer == "return") t = t_return;
            else if (is_int()) {
                t = t_int_literal;
                value = buffer;
            } else {
                t = t_id;
                value = buffer;
            }
            
            buffer = "";
            return t;
        } else {
            buffer += c;
        }
    }
    
    return t_eof;
}

//
// Check and see if a character is a symbol or part of one
//
bool Lex::is_symbol(char c) {
    switch (c) {
        case '\n': return true;
        
        default: {}
    }
    
    return false;
}

//
// Returns the symbol for the equivalent token
//
token Lex::get_symbol(char c) {
    switch (c) {
        case '\n': return t_nl;
        
        default: {}
    }
    
    return t_none;
}

//
// Check to see if a given literal is an integer
//
bool Lex::is_int() {
    for (char c : buffer) {
        if (!isdigit(c)) return false;
    }
    return true;
}

//
// Our lexical debug function
//
void Lex::print(token t) {
    switch (t) {
        case t_eof: std::cout << "EOF" << std::endl; break;
    
        // Keywords
        case t_return: std::cout << "RETURN" << std::endl; break;

        // Symbols
        case t_nl: std::cout << "NL" << std::endl; break;

        // Literals
        case t_id: std::cout << "ID(" << value << ")" << std::endl; break;
        case t_int_literal: std::cout << "VAL(" << value << ")" << std::endl; break;
        
        default: std::cout << "???" << std::endl;
    }
}


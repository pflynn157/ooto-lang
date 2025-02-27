#pragma once

#include <string>
#include <fstream>
#include <stack>

#include <parser/base_lex.hpp>

//
// Represents token data
//
enum token {
    t_eof = 0,
    t_none,
    
	t_extern,
	t_func,
	t_struct,
	t_end,
	t_return,
	t_var,
	t_array,
	t_const,
	t_bool,
	t_char,
	t_string,
	t_i8,
	t_u8,
	t_i16,
	t_u16,
	t_i32,
	t_u32,
	t_i64,
	t_u64,
	t_if,
	t_elif,
	t_else,
	t_while,
	t_is,
	t_then,
	t_do,
	t_break,
	t_continue,
	t_import,
	t_true,
	t_false,
	t_lgand,
	t_lgor,
	t_sizeof,
	t_for,
	t_forall,
	t_in,
	t_step,
	t_repeat,
	t_enum,
	t_class,
	t_extends,
	t_float,
	t_double,
    
	t_dot,
	t_semicolon,
	t_comma,
	t_lparen,
	t_rparen,
	t_lbracket,
	t_rbracket,
	t_plus,
	t_minus,
	t_mul,
	t_div,
	t_mod,
	t_and,
	t_or,
	t_xor,
	t_colon,
	t_gt,
	t_gte,
	t_lt,
	t_lte,
	t_eq,
	t_neq,
	t_assign,
	t_arrow,
	t_range,
	t_scope,
	t_annot,
    
    t_id,
    t_int_literal,
    t_string_literal,
    t_char_literal,
    t_float_literal,
};

//
// The lexical analyzer
//
struct Lex : BaseLex {
    explicit Lex(std::string input);
    void unget(int t) override;
    int get_next() override;
    void debug_token(int t) override;
    
    std::string get_raw_buffer();
private:
    std::ifstream reader;
    std::string buffer = "";
    std::string raw_buffer = "";
    std::stack<token> token_stack;
    
    // Internal functions
    bool is_symbol(char c);
    token get_symbol(char c);
    bool is_integer();
    bool is_hex();
    bool is_float();
};


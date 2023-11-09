#pragma once

#include <string>
#include <memory>

#include <parser/base_parser.hpp>
#include <ast/ast.hpp>

#include <lex/lex.hpp>

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    bool parse() override;
    
    // parser.cpp
    void consume_token(token t, std::string message);
    std::string generate_name(std::string base);
    std::shared_ptr<AstDataType> get_data_type();
    
    // function.cpp
    void parse_function();
    
private:
    std::unique_ptr<Lex> lex;
    std::string module_name = "";
};


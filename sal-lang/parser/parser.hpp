#pragma once

#include <string>
#include <memory>

#include <parser/base_parser.hpp>
#include <lex/lex.hpp>

class Parser : public BaseParser {
public:
    explicit Parser(std::string input);
    bool parse();
    
    bool parse_block(std::shared_ptr<AstBlock> block);
    bool parse_return(std::shared_ptr<AstBlock> block);
    std::shared_ptr<AstExpression> parse_expression(std::shared_ptr<AstBlock> block, token stop = t_nl);
    
    std::unique_ptr<Lex> lex;
};


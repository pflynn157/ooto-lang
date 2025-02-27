//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <map>
#include <memory>

#include <ast/ast.hpp>

#include <Java/JavaBuilder.hpp>

std::string GetClassName(std::string input);

class Compiler {
public:
    explicit Compiler(std::string className);
    void Build(std::shared_ptr<AstTree> tree);
    void Write();
protected:
    void BuildFunction(std::shared_ptr<AstStatement> GS);
    void BuildStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    
    void BuildVarDec(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildVarAssign(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildFuncCallStatement(std::shared_ptr<AstStatement> stmt, std::shared_ptr<JavaFunction> function);
    void BuildExpr(std::shared_ptr<AstExpression> expr, std::shared_ptr<JavaFunction> function, std::shared_ptr<AstDataType> dataType = nullptr);
    
    std::string GetTypeForExpr(std::shared_ptr<AstExpression> expr);
private:
    std::string className;
    std::shared_ptr<JavaClassBuilder> builder;
    std::map<std::string, std::shared_ptr<JavaFunction>> funcMap;
    
    int aCount = 1;
    std::map<std::string, int> objMap;
    std::map<std::string, std::string> objTypeMap;
    
    int iCount = 1;
    std::map<std::string, int> intMap;
};

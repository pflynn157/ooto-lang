//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#pragma once

#include <string>
#include <memory>

#include <ast/ast.hpp>

namespace AstBuilder {

//
// The builders for data types
//
std::shared_ptr<AstDataType> buildVoidType();
std::shared_ptr<AstDataType> buildBoolType();
std::shared_ptr<AstDataType> buildCharType();
std::shared_ptr<AstDataType> buildInt8Type(bool isUnsigned = false);
std::shared_ptr<AstDataType> buildInt16Type(bool isUnsigned = false);
std::shared_ptr<AstDataType> buildInt32Type(bool isUnsigned = false);
std::shared_ptr<AstDataType> buildInt64Type(bool isUnsigned = false);
std::shared_ptr<AstDataType> buildFloat32Type();
std::shared_ptr<AstDataType> buildFloat64Type();
std::shared_ptr<AstDataType> buildStringType();
std::shared_ptr<AstPointerType> buildPointerType(std::shared_ptr<AstDataType> base);
std::shared_ptr<AstPointerType> buildInt32PointerType();
std::shared_ptr<AstStructType> buildStructType(std::string name);
std::shared_ptr<AstObjectType> buildObjectType(std::string name);

}


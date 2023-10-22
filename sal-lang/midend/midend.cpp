//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <memory>

#include "midend.hpp"
#include "ast_processor.hpp"

Midend::Midend(std::shared_ptr<AstTree> tree) {
    this->tree = tree;
}

void Midend::run() {
    std::unique_ptr<AstProcessor> proc = std::make_unique<AstProcessor>(this->tree);
    proc->run();
    this->tree = proc->tree;
}


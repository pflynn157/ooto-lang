//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include <iostream>
#include <string>

#include <libelf/elf.hpp>
#include "symbol.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "Error: No input file." << std::endl;
        return 1;
    }
    
    std::string input = argv[1];
    std::string output = argv[2];

    Elf64File *file = new Elf64File(output);
    
    // Test out our symbol parser
    SymbolParser symparse(input);
    Symbols *symbols = symparse.getSymbols();
    
    /*for (auto const &x : symbols->locations) {
        std::cout << x.first << " -> " << x.second << std::endl;
    }
    
    for (auto sym : symbols->global) {
        std::cout << "GLOBAL: " << sym << std::endl;
    }*/
    
    symparse.processSymbols(file);
    
    Parser parser(input, file, symbols);
    parser.build();
    
    // Write out the file
    file->write();
    
    return 0;
}


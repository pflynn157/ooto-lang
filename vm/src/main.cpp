//
// This software is licensed under BSD0 (public domain).
// Therefore, this software belongs to humanity.
// See COPYING for more info.
//
#include "vm.hpp"

int main() {
    VM vm;
    vm.load("first.bin", true);
    vm.run(true);
    return 0;
}


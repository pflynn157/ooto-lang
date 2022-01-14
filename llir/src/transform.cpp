#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include <llir.hpp>

namespace LLIR {

// The maps
std::vector<std::string> memList;
std::map<std::string, int> regMap;
int regCount = 0;

Operand *checkOperand(Operand *input) {
    if (input->getType() != OpType::Reg) {
        return input;
    }
    
    Reg *reg = static_cast<Reg *>(input);
    if (std::find(memList.begin(), memList.end(), reg->getName()) != memList.end()) {
        Mem *mem = new Mem(reg->getName());
        return mem;
    }
    
    if (regMap.find(reg->getName()) != regMap.end()) {
        HReg *reg2 = new HReg(regMap[reg->getName()]);
        return reg2;
    }
    
    return input;
}

// By default, all operands in LLIR are virtual registers, which are naturally not
// suitable to hardware transformation
//
// This pass translates all virtual registers to hardware registers and memory operands
// as appropriate.
//
// The algorithm is very simple. Basically, we look for "allocas", and assign memory operands to
// the destination, and replace everywhere else.
//
void Module::transform() {
    for (Function *func : functions) {
        memList.clear();
        regMap.clear();
        regCount = 0;
        
        for (int i = 0; i<func->getBlockCount(); i++) {
            Block *block = func->getBlock(i);
            for (int j = 0; j<block->getInstrCount(); j++) {
                Instruction *instr = block->getInstruction(j);
                switch (instr->getType()) {
                    case InstrType::Alloca: {
                        Reg *reg = static_cast<Reg *>(instr->getDest());
                        memList.push_back(reg->getName());
                        
                        Mem *mem = new Mem(reg->getName());
                        instr->setDest(mem);
                        continue;
                    }
                    
                    // In order to keep from too many registers being used, we should always
                    // go back to earlier registers once we hit a store instruction
                    case InstrType::Store: {
                        regCount = 0;
                    } break;
                    
                    case InstrType::Load:
                    case InstrType::Add:
                    case InstrType::Sub:
                    case InstrType::SMul:
                    case InstrType::UMul:
                    case InstrType::SDiv:
                    case InstrType::UDiv: {
                        Reg *reg = static_cast<Reg *>(instr->getDest());
                        regMap[reg->getName()] = regCount;
                        ++regCount;
                        
                        HReg *reg2 = new HReg(regCount - 1);
                        instr->setDest(reg2);
                    } break;
                }
                
                // Otherwise, check and switch the operands
                if (instr->getOperand1()) {
                    Operand *op = checkOperand(instr->getOperand1());
                    instr->setOperand1(op);
                }
                
                if (instr->getOperand2()) {
                    Operand *op = checkOperand(instr->getOperand2());
                    instr->setOperand2(op);
                }
                
                if (instr->getOperand3()) {
                    Operand *op = checkOperand(instr->getOperand3());
                    instr->setOperand3(op);
                }
            }
        }
    }
}

} // end LLIR

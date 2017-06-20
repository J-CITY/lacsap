#include "parser.h"

//push eax
//pop eax
bool tryOpt1(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == in2->_type && in2->_type == AsmOperationType::cmd1) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_push &&
            ((AsmCmd*)in2)->operation == AsmOperation::asm_pop) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg &&
                ((AsmCmd*)in2)->left->_type == AsmOperationType::reg &&
                ((AsmReg*)((AsmCmd*)in1)->left)->name == ((AsmReg*)((AsmCmd*)in2)->left)->name) {
                if(((AsmCmd*)in1)->b1 == false && ((AsmCmd*)in2)->b1 == false) {
                    //std::cout << pos << ": ";in1->print();
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//mov eax, 1
//push eax
bool tryOpt2(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd2 && in2->_type == AsmOperationType::cmd1) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_mov &&
            ((AsmCmd*)in2)->operation == AsmOperation::asm_push) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg &&
                ((AsmCmd*)in2)->left->_type == AsmOperationType::reg &&
                ((AsmReg*)((AsmCmd*)in1)->left)->name == ((AsmReg*)((AsmCmd*)in2)->left)->name) {
                if(((AsmCmd*)in1)->right->_type == AsmOperationType::imn &&
                   ((AsmCmd*)in1)->b2 == false &&
                   ((AsmCmd*)in1)->b1 == false &&
                   ((AsmCmd*)in2)->b1 == false ) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2, new AsmCmd(AsmOperation::asm_push, ((AsmCmd*)in1)->right));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//push eax
//pop ebx
bool tryOpt3(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == in2->_type && in2->_type == AsmOperationType::cmd1) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_push &&
            ((AsmCmd*)in2)->operation == AsmOperation::asm_pop) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg &&
                ((AsmCmd*)in2)->left->_type == AsmOperationType::reg) {
                if(((AsmCmd*)in1)->b1 == false && ((AsmCmd*)in2)->b1 == false) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2,
                                       new AsmCmd(AsmOperation::asm_mov,
                                                  ((AsmCmd*)in2)->left,
                                                  ((AsmCmd*)in1)->left));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//mov eax, 1
//add eax, ebx
bool tryOpt4(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == in2->_type && in2->_type == AsmOperationType::cmd2) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_mov &&
            (
             ((AsmCmd*)in2)->operation == AsmOperation::asm_add ||
             ((AsmCmd*)in2)->operation == AsmOperation::asm_sub ||
             ((AsmCmd*)in2)->operation == AsmOperation::asm_imul ||
             ((AsmCmd*)in2)->operation == AsmOperation::asm_xor ||
             ((AsmCmd*)in2)->operation == AsmOperation::asm_or ||
             ((AsmCmd*)in2)->operation == AsmOperation::asm_and
             )) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmCmd*)in2)->b1 == false) {

                if(((AsmCmd*)in2)->right->_type == AsmOperationType::reg &&
                   ((AsmReg*)((AsmCmd*)in1)->left)->name == ((AsmReg*)((AsmCmd*)in2)->right)->name) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2,
                                       new AsmCmd(((AsmCmd*)in2)->operation,
                                                  ((AsmCmd*)in2)->left,
                                                  ((AsmCmd*)in1)->right, ((AsmCmd*)in2)->t1, ((AsmCmd*)in2)->b1,
                                                  ((AsmCmd*)in2)->s1, ((AsmCmd*)in2)->s1str));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//mov eax, qword [v]
//push eax
bool tryOpt5(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd2 && in2->_type == AsmOperationType::cmd1) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_mov &&
             ((AsmCmd*)in2)->operation == AsmOperation::asm_push) {

            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in2)->left->_type == AsmOperationType::reg && ((AsmCmd*)in2)->b1 == false &&
               ((AsmReg*)((AsmCmd*)in1)->left)->name == ((AsmReg*)((AsmCmd*)in2)->left)->name) {

                if(
                   ((AsmCmd*)in1)->right->_type == AsmOperationType::ind &&
                   ((AsmCmd*)in1)->b2 == true &&
                   ((AsmCmd*)in1)->t2 != AsmSizeof::s_def
                   ) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2,
                                       new AsmCmd(AsmOperation::asm_push,
                                                  ((AsmCmd*)in1)->right, AsmSizeof::s_dq, true, ((AsmCmd*)in1)->s2, ((AsmCmd*)in1)->s2str));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//push 1
//pop eax
bool tryOpt6(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd1 && in2->_type == AsmOperationType::cmd1) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_push &&
             ((AsmCmd*)in2)->operation == AsmOperation::asm_pop) {

            if(((AsmCmd*)in1)->left->_type == AsmOperationType::imn && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in2)->left->_type == AsmOperationType::reg) {
                if(((AsmImn*)((AsmCmd*)in1)->left)->var == "0" && ((AsmCmd*)in2)->b1 == false) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2,
                                       new AsmCmd(AsmOperation::asm_xor,
                                                  ((AsmCmd*)in2)->left, ((AsmCmd*)in2)->left));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                } else if(((AsmCmd*)in2)->b1 == false) {
                    ASM.asmcode.insert(ASM.asmcode.begin()+pos+2,
                                       new AsmCmd(AsmOperation::asm_mov,
                                                  ((AsmCmd*)in2)->left, ((AsmCmd*)in1)->left, ((AsmCmd*)in2)->t1, ((AsmCmd*)in2)->b1, ((AsmCmd*)in2)->s1, ((AsmCmd*)in2)->s1str));
                    ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+2);
                    return true;
                }
            }
        }
    }
    return false;
}

//mov rax, 0
bool tryOpt7(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd2) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_mov) {

            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmImn*)((AsmCmd*)in1)->right)->var == "0") {
                ASM.asmcode.insert(ASM.asmcode.begin()+pos+1,
                                    new AsmCmd(AsmOperation::asm_xor,
                                                ((AsmCmd*)in1)->left, ((AsmCmd*)in1)->left));
                ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+1);
                return true;
            }
        }
    }
    return false;
}

//imul rax, 0
//imul rax, 1
bool tryOpt8(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd2) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_add || ((AsmCmd*)in1)->operation == AsmOperation::asm_sub) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmImn*)((AsmCmd*)in1)->right)->var == "0") {
                ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+1);
                return true;
            }
        } else if(((AsmCmd*)in1)->operation == AsmOperation::asm_imul) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmImn*)((AsmCmd*)in1)->right)->var == "0") {
                ASM.asmcode.insert(ASM.asmcode.begin()+pos+1,
                                    new AsmCmd(AsmOperation::asm_xor,
                                                ((AsmCmd*)in1)->left, ((AsmCmd*)in1)->left));
                ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+1);
                return true;
            } else if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmImn*)((AsmCmd*)in1)->right)->var == "1") {
                ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+1);
                return true;
            }
        }
    }
    return false;
}

//cmp rax, 0
bool tryOpt9(AsmCode *in1, AsmCode *in2, int pos) {
    if(in1->_type == AsmOperationType::cmd2) {
        if(((AsmCmd*)in1)->operation == AsmOperation::asm_cmp) {
            if(((AsmCmd*)in1)->left->_type == AsmOperationType::reg && ((AsmCmd*)in1)->b1 == false &&
               ((AsmCmd*)in1)->right->_type == AsmOperationType::imn && ((AsmImn*)((AsmCmd*)in1)->right)->var == "0") {
                ASM.asmcode.insert(ASM.asmcode.begin()+pos+1,
                                    new AsmCmd(AsmOperation::asm_test,
                                                ((AsmCmd*)in1)->left, ((AsmCmd*)in1)->left));
                ASM.asmcode.erase(ASM.asmcode.begin()+pos, ASM.asmcode.begin()+pos+1);
                return true;
            }
        }
    }
    return false;
}

void Parser::optimization() {
    int i = 0;
    while(i < ASM.asmcode.size()-1) {
        if(tryOpt1(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt2(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt3(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt4(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt5(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt6(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt7(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt8(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        } else if(tryOpt9(ASM.asmcode[i], ASM.asmcode[i+1], i)) {
            continue;
        }
        i++;
    }
}

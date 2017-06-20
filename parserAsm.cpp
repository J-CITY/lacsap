#include "parser.h"

int pushFuncParam(bool ifFunc, std::string name, Descriptor *descr, int _mov) {
    int mov = 0;
    AsmImn* shift = nullptr;
    int popSz = 0;

    if(descr->type != DescriptorTypes::arrays &&
        descr->type != DescriptorTypes::records) {
            popSz++;
            if(!ifFunc) {
                shift = new AsmImn(std::to_string(_mov));
                ASM.Add(AsmOperation::asm_push, new AsmInd("v_"+name), AsmSizeof::s_dq, true, shift);
            } else {
                if(symbolStack->get()->findInTable(name) != nullptr) {
                    mov = symbolStack->get()->movInTable(name);
                    //ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp-"+std::to_string((mov+1+_mov)*8), AsmSizeof::s_def, true));
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr,"-"+std::to_string((mov+1+_mov)*8));
                } else if(symbolStack->get()->parent->findInTable(name) != nullptr) {
                    mov = symbolStack->get()->parent->movInTable(name);
                    //ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp+8+"+std::to_string(mov+1+_mov*8), AsmSizeof::s_def, true));
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr,"+"+std::to_string(mov+1+_mov*8));
                } else if(baseTable->findInTable(name) != nullptr) {
                    shift = new AsmImn(std::to_string(_mov));
                    ASM.Add(AsmOperation::asm_push, new AsmInd("v_"+name), AsmSizeof::s_dq, true, shift);
                }
                ASM.Add(AsmOperation::asm_push, rbx);
            }
    } else if(descr->type == DescriptorTypes::arrays) {
        DescriptorArray* darr = (DescriptorArray*)(descr);
        for(int i = 0; i < darr->indices.size(); ++i) {
            int _max = std::stoi(((ExpressionNode*)(((DescriptorLimited*)(darr->indices[i]))->_max))->nameLex);
            int _min = std::stoi(((ExpressionNode*)(((DescriptorLimited*)(darr->indices[i]))->_min))->nameLex);
            for(int j = 0; j < _max - _min+1; ++j) {
                popSz += pushFuncParam(ifFunc, name, darr->baseType, j*(std::stoi(getSize("", darr->baseType))));
            }
        }
    } else if(descr->type == DescriptorTypes::records) {
        DescriptorRecord* drec = (DescriptorRecord*)(descr);
        int movInRec=0;
        for(int i = 0; i < drec->rName.size(); ++i) {
            popSz += pushFuncParam(ifFunc, name, drec->rName[i]->type, _mov+movInRec);
            movInRec += (std::stoi(getSize("", drec->rName[i]->type)));
        }

    }

    return popSz;
}


std::string genAsmDerective(Descriptor *type) {
    switch(type->type) {
    case DescriptorTypes::scalarInt:
        return "dq";
    case DescriptorTypes::scalarFloat:
        return "dq";
    case DescriptorTypes::scalarChar:
        return "dq";
    case DescriptorTypes::scalarBoolean:
        return "dq";
    case DescriptorTypes::pointer:
        return genAsmDerective(((DescriptorPointers*)type)->baseType);
    case DescriptorTypes::records:
        return "dq";
    case DescriptorTypes::arrays:
        return genAsmDerective(((DescriptorArray*)type)->baseType);
    default:
        break;
    }
    return "";
}


void generateVars(SymbolTable *symt, bool ifFunc) {

    for(int i = 0; i < symt->symbolsvec.size(); ++i) {
        if(ifFunc) {
            int _sz = std::stoi(getSize("", symt->symbolsvec[i]->type));
            //for(int i = 0; i < result.size(); ++i) {
                if(((SymbolVar*)symt->symbolsvec[i])->mot == MethodOfTransmission::paramvar) {
                    ASM.Add(AsmOperation::asm_push, new AsmImn("0"), AsmSizeof::s_def, false);
                } else {
                    for(int j = 0; j < _sz; ++j) {
                        ASM.Add(AsmOperation::asm_push, new AsmImn("0"), AsmSizeof::s_def, false);
                    }
                }
            //}
        }
    }

	if(!ifFunc)
	for(unsigned int i = 0; i < symt->symbolsvec.size(); ++i) {
        if(symt->symbolsvec[i]->_class != SymbolTypes::func &&
           symt->symbolsvec[i]->_class != SymbolTypes::proc &&
           symt->symbolsvec[i]->_class != SymbolTypes::consts) {
            //std::cout << symt->symbolsvec[i]->lex.lexem << "\n";
            std::string _t = genAsmDerective(symt->symbolsvec[i]->type);
            if(symt->symbolsvec[i]->type->type == DescriptorTypes::arrays && !((DescriptorArray*)symt->symbolsvec[i]->type)->isOpen) {
                _t = "v_" + symt->symbolsvec[i]->lex.lexem + ": " + " times "
                    + getSize(symt->symbolsvec[i]->lex.lexem, symt->symbolsvec[i]->type) +" "+ _t + " " + "0";
                ASMOther.Add(_t);
            } else if(symt->symbolsvec[i]->type->type == DescriptorTypes::records) {
                _t = "v_" + symt->symbolsvec[i]->lex.lexem + ": " + " times "
                    + getSize(symt->symbolsvec[i]->lex.lexem, symt->symbolsvec[i]->type) +" "+ _t + " " + "0";
                ASMOther.Add(_t);
            } else {
                _t = "v_" + symt->symbolsvec[i]->lex.lexem + ": " + _t + " " + "0";
                ASMOther.Add(_t);
            }
        }
	}

}

void BinOperationNode::generateAsm(bool ifFunc, bool needAsm) {
    if(nameLex == ":=") {
        if(((ExpressionNode*)children[0])->nameLex == "@") {
            children[0]->generateAsm(ifFunc, needAsm);
            ((VarNode*)children[0]->children[0])->generateAsmIdent(ifFunc, needAsm, false);///ifFunc
        } else if(((ExpressionNode*)children[0])->nameLex == "^") {
            //children[0]->generateAsm(ifFunc, needAsm);
            ((VarNode*)children[0]->children[0])->generateAsmIdent(ifFunc, needAsm, false);///ifFunc
        } else {
            ((VarNode*)children[0])->generateAsmIdent(ifFunc, needAsm, false);///ifFunc
        }
        AsmOperand* shift = nullptr;
        if(((ExpressionNode*)children[0])->nameLex == "^" || ((ExpressionNode*)children[0])->nameLex == "@") {//need before expression
            if(___shift != 0) {
                ASM.Add(AsmOperation::asm_pop, r13);
                ___shift--;
                while(___shift != 0) {
                    ASM.Add(AsmOperation::asm_pop, r13);
                    ___shift--;
                    ASM.Add(AsmOperation::asm_add, r13, r14);
                }
                shift = r13;
            }
        } else if(___shift != 0) {
            ASM.Add(AsmOperation::asm_pop, r13);
            ___shift--;
            while(___shift != 0) {
                ASM.Add(AsmOperation::asm_pop, r14);
                ___shift--;
                ASM.Add(AsmOperation::asm_add, r13, r14);
            }
            shift = r13;
        }
        shiftFor = shift;

        //children[1]->generateAsm();//expr
        //std::cout << ((ExpressionNode*)children[1])->nameLex << "\n";
        generateAsmCode(children[1], ifFunc, needAsm, false);
        //std::cout << ifFunc << " !!!\n";
        if(!ifFunc)
            genAsmCodeForIdent(children[0], children[1], shift);
        else
            genAsmCodeForIdentFunc(children[0], children[1], shift);

    } else
    if(convertType->type == DescriptorTypes::scalarInt) {

        if(id == (int)Symbols::sys_div || id == (int)Symbols::sys_mod) {
            ASM.Add(AsmOperation::asm_pop, rax);
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_mov, r8, rax);
            ASM.Add(AsmOperation::asm_mov, rax, rbx);
            ASM.Add(AsmOperation::asm_xor, rdx, rdx);
            ASM.Add(AsmOperation::asm_idiv, r8);
            if(id == (int)Symbols::sys_div) {
                ASM.Add(AsmOperation::asm_push, rax);
            } else {
                ASM.Add(AsmOperation::asm_push, rdx);
            }
        } else {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_pop, rax);
            if(id == (int)Symbols::sys_shl || id == (int)Symbols::sys_shr) {
                ASM.Add(AsmOperation::asm_mov, rcx, rbx);
                ASM.Add(getAsmOper(id, 0), rax, new AsmImn("cl"));

            } else {
                ASM.Add(getAsmOper(id, 0), rax, rbx);
            }
            ASM.Add(AsmOperation::asm_push, rax);
        }
    } else
    if(convertType->type == DescriptorTypes::scalarFloat) {
        ASM.Add(AsmOperation::asm_pop, rbx);
        if(((ExpressionNode*)children[1])->convertType->type == DescriptorTypes::scalarInt) {
            ASM.Add(AsmOperation::asm_cvtsi2sd, xmm1, rbx);
        } else {
            ASM.Add(AsmOperation::asm_movq, xmm1, rbx);
        }
        ASM.Add(AsmOperation::asm_pop, rax);
        if(((ExpressionNode*)children[0])->convertType->type == DescriptorTypes::scalarInt) {
            ASM.Add(AsmOperation::asm_cvtsi2sd, xmm0, rax);
        } else {
            ASM.Add(AsmOperation::asm_movq, xmm0, rax);
        }
        ASM.Add(getAsmOper(id, 1), xmm0, xmm1);
        //asmConstants.Add("8");
        //ASM.Add(AsmOperation::asm_sub, rsp, new AsmInd("_8", AsmSizeof::s_dq, true));
        ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("8"));
        ASM.Add(AsmOperation::asm_movq, rsp, xmm0, AsmSizeof::s_dq, true);
    } else
    if(convertType->type == DescriptorTypes::scalarBoolean) {
        if(id == (int)Symbols::sys_and) {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_pop, rax);
            ASM.Add(AsmOperation::asm_test, rax, rax);
            ASM.Add(AsmOperation::asm_jz, new AsmMark("CMP"+std::to_string(boolOperCounter), false));
            ASM.Add(AsmOperation::asm_push, rbx);
            ASM.Add(AsmOperation::asm_jmp, new AsmMark("_CMP"+std::to_string(boolOperCounter), false));
            ASM.Add("CMP"+std::to_string(boolOperCounter)+":");
            //asmConstants.Add("0");
            //ASM.Add(AsmOperation::asm_push, new AsmInd("_0", AsmSizeof::s_dq, true));
            ASM.Add(AsmOperation::asm_push, new AsmImn("0"));
            ASM.Add("_CMP"+std::to_string(boolOperCounter)+":");
            boolOperCounter++;
        } else if(id == (int)Symbols::sys_or) {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_pop, rax);
            ASM.Add(AsmOperation::asm_test, rax, rax);
            ASM.Add(AsmOperation::asm_jnz, new AsmMark("CMP"+std::to_string(boolOperCounter), false));
            ASM.Add(AsmOperation::asm_push, rbx);
            ASM.Add(AsmOperation::asm_jmp, new AsmMark("_CMP"+std::to_string(boolOperCounter), false));
            ASM.Add("CMP"+std::to_string(boolOperCounter)+":");
            //asmConstants.Add("1");
            //ASM.Add(AsmOperation::asm_push, new AsmInd("_1", AsmSizeof::s_dq, true));
            ASM.Add(AsmOperation::asm_push, new AsmImn("1"));
            ASM.Add("_CMP"+std::to_string(boolOperCounter)+":");
            boolOperCounter++;
        } else if(id == (int)Symbols::sys_xor) {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_pop, rax);
            ASM.Add(AsmOperation::asm_xor, rax, rbx);
            ASM.Add(AsmOperation::asm_push, rax);
        } else {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_pop, rax);
            ASM.Add(AsmOperation::asm_cmp, rax, rbx);
            ASM.Add(getAsmOper(id, 0), new AsmMark("CMP"+std::to_string(boolOperCounter), false));
            //asmConstants.Add("0");
            //ASM.Add(AsmOperation::asm_push, new AsmInd("_0", AsmSizeof::s_dq, true));
            ASM.Add(AsmOperation::asm_push, new AsmImn("0"));

            ASM.Add(AsmOperation::asm_jmp, new AsmMark("_CMP"+std::to_string(boolOperCounter), false));
            ASM.Add("CMP"+std::to_string(boolOperCounter)+":");
            //asmConstants.Add("1");
            //ASM.Add(AsmOperation::asm_push, new AsmInd("_1", AsmSizeof::s_dq, true));
            ASM.Add(AsmOperation::asm_push, new AsmImn("1"));
            ASM.Add("_CMP"+std::to_string(boolOperCounter)+":");
            boolOperCounter++;
        }
    }

}

void UnarOperationNode::generateAsm(bool ifFunc, bool needAsm) {
    if(needAsm) {
    AsmOperand* shift = nullptr;

    if(nameLex == "^" || nameLex == "@") {
        if(___shift != 0) {
            ASM.Add(AsmOperation::asm_pop, rax);
            ___shift--;
            while(___shift != 0) {
                ASM.Add(AsmOperation::asm_pop, rbx);
                ___shift--;
                ASM.Add(AsmOperation::asm_add, rax, rbx);
            }
            shift = rax;
        }
    } else if(___shift != 0) {
        ASM.Add(AsmOperation::asm_pop, rax);
        ___shift--;
        while(___shift != 0) {
            ASM.Add(AsmOperation::asm_pop, rbx);
            ___shift--;
            ASM.Add(AsmOperation::asm_add, rax, rbx);
        }
        shift = rax;
    }

    int mov = 0;
    if(nameLex == "@") {
        if(ifFunc) {
            if(symbolStack->get()->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0])->nameLex);
                if(((SymbolVar*)(((VarNode*)(children[0]))->type))->mot == MethodOfTransmission::paramvar) {
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift,"-"+std::to_string(mov*8));
                } else {
                    ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift,"-"+std::to_string(mov*8));
                }
            } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0])->nameLex, false);
                if(shift != nullptr)
                    ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                if(((SymbolVar*)(((VarNode*)(children[0]))->type))->mot == MethodOfTransmission::paramvar) {
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "+"+std::to_string((mov+1)*8));
                } else {
                    ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift, "+"+std::to_string((mov+1)*8));
                }
            } else if(baseTable->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)children[0])->nameLex, AsmSizeof::s_def, true, shift));
            }
        } else {
            ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)children[0])->nameLex),AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift);
        }
        ASM.Add(AsmOperation::asm_push, rbx);
    }
    }

    if(convertType->type == DescriptorTypes::scalarBoolean) {
        ASM.Add(AsmOperation::asm_pop, rbx);
        ASM.Add(getAsmOper(id,0), rbx);
        ASM.Add(AsmOperation::asm_push, rbx);

    }
    if(convertType->type == DescriptorTypes::scalarInt) {
        ASM.Add(AsmOperation::asm_xor, rax, rax);
        ASM.Add(AsmOperation::asm_pop, rbx);
        if(getAsmOper(id,0) == AsmOperation::asm_not) {
            ASM.Add(getAsmOper(id,0), rbx);
            ASM.Add(AsmOperation::asm_push, rbx);
        } else {
            ASM.Add(getAsmOper(id,0), rax, rbx);
            ASM.Add(AsmOperation::asm_push, rax);
        }
    }
    if(convertType->type == DescriptorTypes::scalarFloat) {
        ASM.Add(AsmOperation::asm_pop, rbx);
        if(convertType->type == DescriptorTypes::scalarInt) {
            ASM.Add(AsmOperation::asm_cvtsi2sd, xmm1, rbx);
        } else {
            ASM.Add(AsmOperation::asm_movq, xmm1, rbx);
        }
        ASM.Add(AsmOperation::asm_xor, rax, rax);
        ASM.Add(AsmOperation::asm_cvtsi2sd, xmm0, rax);

        ASM.Add(getAsmOper(id, 1), xmm0, xmm1);
        //asmConstants.Add("8");
        //ASM.Add(AsmOperation::asm_sub, rsp, new AsmInd("_8", AsmSizeof::s_dq, true));
        ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("8"));
        ASM.Add(AsmOperation::asm_movq, rsp, xmm0, AsmSizeof::s_dq, true);
    }

}



void ForActionNode::generateAsm(bool ifFunc, bool needAsm) {
    //std::cout << ((ExpressionNode*)children[0])->nameLex <<((ExpressionNode*)children[0])->nameNode<< "!!\n";
    ((BinOperationNode*)children[0])->generateAsm(ifFunc, needAsm);
    AsmOperand* shift = nullptr;
    shift = ((BinOperationNode*)children[0])->shiftFor;
    //AsmOperand* shift = ((VarNode*)children[0])->generateAsmFactor(ifFunc, needAsm, false);
    //generateAsmCode(children[0], ifFunc, needAsm, false);
    generateAsmCode(children[1], ifFunc, needAsm, false);


    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;

    ASM.Add(AsmOperation::asm_jmp, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
    ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
    //asmConstants.Add("1");
    //ASM.Add(AsmOperation::asm_push, new AsmInd("_1"), AsmSizeof::s_dq, true);
    ASM.Add(AsmOperation::asm_push, new AsmImn("1"));
    ASM.Add(AsmOperation::asm_pop, rbx);
    if(nameNode == "to") {
        if(ifFunc) {
            int mov = 0;
            if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_add, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true, shift);
            }
        } else {
            ASM.Add(AsmOperation::asm_add, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
        }
    } else {
        if(ifFunc) {
            int mov = 0;
            if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_sub, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
            }
        } else {
            ASM.Add(AsmOperation::asm_sub, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
        }
    }
    ASM.Add(AsmOperation::asm_pop, r12);

    if(ifFunc) {
        int mov = 0;
        if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr,"-"+std::to_string(mov*8));
        } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
        } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true);
        }
    } else {
        ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true);
    }
    ASM.Add(AsmOperation::asm_push, r12);
    if(nameNode == "to") {
        ASM.Add(AsmOperation::asm_jle, new AsmMark("BREAK"+std::to_string(_boolOperCounter), false));
    } else {
        ASM.Add(AsmOperation::asm_jge, new AsmMark("BREAK"+std::to_string(_boolOperCounter), false));
    }
    ASM.Add("FOR"+std::to_string(_boolOperCounter)+":");

    generateAsmCode(children[2], ifFunc, needAsm, false);
    if(children[2]->nameNode == "identifier" && ((VarNode*)children[2])->type->_class != SymbolTypes::proc) {
        ASM.Add(AsmOperation::asm_pop, rax);
    }

    //asmConstants.Add("1");
    //ASM.Add(AsmOperation::asm_push, new AsmInd("1"), AsmSizeof::s_dq, true);
    ASM.Add(AsmOperation::asm_push, new AsmImn("1"));
    ASM.Add(AsmOperation::asm_pop, rbx);
    if(nameNode == "to") {
        if(ifFunc) {

            int mov = 0;
            if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_add, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
            }
        } else {
            ASM.Add(AsmOperation::asm_add, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
        }
    } else {
        if(ifFunc) {
            int mov = 0;
            if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr,"-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
            } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_sub, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
            }
        } else {
            ASM.Add(AsmOperation::asm_sub, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), rbx, AsmSizeof::s_dq, true);
        }
    }
    ASM.Add(AsmOperation::asm_pop, r12);
    if(ifFunc) {
        int mov = 0;
        if(symbolStack->get()->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8));
        } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0]->children[0])->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
        } else if(baseTable->findInTable(((ExpressionNode*)children[0]->children[0])->nameLex) != nullptr) {
            ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_"+((ExpressionNode*)children[0]->children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true);
        }
    } else {
        ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_" + ((ExpressionNode*)children[0]->children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true);
    }
    ASM.Add(AsmOperation::asm_push, r12);

    if(nameNode == "to") {
        ASM.Add(AsmOperation::asm_jge, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
    } else {
        ASM.Add(AsmOperation::asm_jle, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
    }
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
    ASM.Add(AsmOperation::asm_pop, r12);
    stackBreak.pop();
    ((BinOperationNode*)children[0])->shiftFor = nullptr;
}

void VarNode::func(TreeNode* node, bool ifFunc, bool needAsm, bool ifRead) {
if(nameLex == "ORD") {
    generateAsmCode(children[0], ifFunc, needAsm, false);
}
if(nameLex == "nil") {
    if(needAsm) {
        asmConstants.Add(nameLex);
    }
}
for(int k = 0; k < node->children.size(); ++k) {
    if(node->children.size() != 0 && ((ActionNode*)node->children[k])->nameLex == "[]" && needAsm) {
        for(int i = 0; i < ((DescriptorArray*)(type->type))->indices.size(); ++i) {
            generateAsmCode(node->children[k]->children[i], ifFunc, needAsm, ifRead);
        }
        //std::cout << needAsm << "\n";
        if(needAsm) {
            ASM.Add(AsmOperation::asm_mov, rax, new AsmImn("0"));
            for(int i = 0; i < ((DescriptorArray*)(type->type))->indices.size(); ++i) {
                ASM.Add(AsmOperation::asm_pop, rbx);
                if(!((DescriptorArray*)(type->type))->isOpen) {
                    ASM.Add(AsmOperation::asm_sub, rbx,
                            new AsmImn(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)(type->type))->indices[i])->_min)->nameLex));
                }
                if(i > 0 && !((DescriptorArray*)(type->type))->isOpen) {
                    int dim = std::stoi(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)(type->type))->indices[i-1])->_max)->nameLex) -
                        std::stoi(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)(type->type))->indices[i-1])->_min)->nameLex);
                    ASM.Add(AsmOperation::asm_imul, rbx, new AsmImn(std::to_string(dim+1)));
                } else if(i > 0 && ((DescriptorArray*)(type->type))->isOpen) {

                    std::string szname = ((ExpressionNode*)node)->nameLex+"size"+std::to_string(i-1);
                    if((SymbolVar*)(symbolStack->get()->parent->findInTable(szname)) != nullptr) {
                        int mov = symbolStack->get()->parent->movInTable(szname);
                        //std::cout << (mov+1)*8 << "  " << szname  <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
                        ASM.Add(AsmOperation::asm_imul, rdx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
                        //ASM.Add(AsmOperation::asm_imul, rbx, rax);
                    }


                }
                ASM.Add(AsmOperation::asm_add, rax, rbx);
            }
            ASM.Add(AsmOperation::asm_imul, rax, new AsmImn(getSize("", ((DescriptorArray*)(type->type))->baseType)));
            ASM.Add(AsmOperation::asm_push, rax);
            ___shift++;
        }
    }

    if(node->children.size() != 0 && ((ActionNode*)node->children[k])->nameLex == "." && needAsm) {
        Descriptor *drec;
        if(((VarNode*)node)->type->type->type == DescriptorTypes::records) {
            drec = ((VarNode*)node)->type->type;
        }
        if(((VarNode*)node)->type->type->type == DescriptorTypes::arrays &&
            ((DescriptorArray*)(type->type))->baseType->type == DescriptorTypes::records) {
            drec = ((DescriptorArray*)(((VarNode*)node)->type->type))->baseType;
        }


        ASM.Add(AsmOperation::asm_mov, rax, new AsmImn("0"));
        for(unsigned int i = 0; i < ((DescriptorRecord*)(drec))->rName.size(); ++i) {
            if( ((ExpressionNode*) ((ActionNode*)node->children[k])->children[0])->nameLex == ((DescriptorRecord*)(drec))->rName[i]->lex.lexem) {
                break;
            }
            std::string _s;
            //if(isArr && ((DescriptorArray*)(type->type))->baseType->type == DescriptorTypes::records) {
                _s = getSize("", ((DescriptorRecord*)(drec))->rName[i]->type);
            //} else
            //    _s = getSize("", ((DescriptorRecord*)(drec))->rName[i]->type);
            ASM.Add(AsmOperation::asm_add, rax, new AsmImn(_s));

        }
        ASM.Add(AsmOperation::asm_push, rax);
        ___shift++;
        //std::cout << ((ExpressionNode*)node->children[k]->children[0])->nameLex << "!!!!\n";
        func(((ActionNode*)node->children[k])->children[0], ifFunc, needAsm, ifRead);
    }
    else
    if(((VarNode*)node)->type->_class == SymbolTypes::func || ((VarNode*)node)->type->_class == SymbolTypes::proc) {
        if(((VarNode*)node)->type->_class == SymbolTypes::func) {
            if(needAsm)
                ASM.Add(AsmOperation::asm_push, new AsmImn("0"), AsmSizeof::s_def, false);
        }
        if(needAsm) {
        unsigned int inputParam = 1;
        if(((VarNode*)node)->type->_class == SymbolTypes::proc)
            inputParam = 0;
        int popSz = 0;
        for(int i = 0; i < node->children[k]->children.size(); ++i) {
            if(((SymbolVar*)(((SymbolFunc*)(((VarNode*)node)->type))->inputParam->symbolsvec[inputParam]))->mot == MethodOfTransmission::paramval)
                mot = 1;
            else
                mot = -1;

            generateAsmCode(node->children[k]->children[i], ifFunc, needAsm, ifRead);

            if((((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::arrays ||
                ((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::records) && mot == 1) {
                if(((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::arrays &&
                    ((DescriptorArray*)((SymbolVar*)(((SymbolFunc*)(((VarNode*)node)->type))->inputParam->symbolsvec[inputParam]))->type)->isOpen) {

                        AsmOperand* shift = nullptr;
                        if(ifFunc) {
                            int mov = 0;
                            SymbolVar *svar = nullptr;
                            if((svar = (SymbolVar*)(symbolStack->get()->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                                mov = symbolStack->get()->movInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex);
                                //-
                                ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr, "-"+std::to_string((mov)*8));
                                //ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_def, true));

                            } else if((svar = (SymbolVar*)(symbolStack->get()->parent->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                                mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex);
                                if(shift != nullptr)
                                    ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                                ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr, "+"+std::to_string((mov+1)*8));

                                //ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_def, true));
                            } else if((svar = (SymbolVar*)(baseTable->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                                ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)node->children[k]->children[i])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true);
                            }
                        } else {
                            ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)node->children[k]->children[i])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true);
                        }
                        mot = 0;
                        ASM.Add(AsmOperation::asm_push, rbx);
                        //add arr size
                        for(int y = 0; y < ((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices.size(); ++y) {
                            ASM.Add(AsmOperation::asm_mov, rax,
                                    new AsmImn(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices[i])->_max)->nameLex));
                            ASM.Add(AsmOperation::asm_mov, rbx,
                                    new AsmImn(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices[i])->_min)->nameLex));
                            ASM.Add(AsmOperation::asm_sub, rax, rbx);
                            ASM.Add(AsmOperation::asm_add, rax, new AsmImn("1"));
                            ASM.Add(AsmOperation::asm_push, rax);
                        }

                } else {
                    popSz--;
                    popSz += pushFuncParam(ifFunc, ((ExpressionNode*)node->children[k]->children[i])->nameLex, ((ExpressionNode*)node->children[k]->children[i])->convertType, 0);
                }
            } else if((((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::arrays ||
                ((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::records) && mot == -1) {
                AsmOperand* shift = nullptr;

                if(ifFunc) {
                    int mov = 0;
                    SymbolVar *svar = nullptr;
                    if((svar = (SymbolVar*)(symbolStack->get()->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                        mov = symbolStack->get()->movInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex);
                        //-
                        ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr, "-"+std::to_string((mov)*8));
                        //ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_def, true));

                    } else if((svar = (SymbolVar*)(symbolStack->get()->parent->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                        mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex);
                        if(shift != nullptr)
                            ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                        ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, nullptr, "+"+std::to_string((mov+1)*8));

                        //ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_def, true));
                    } else if((svar = (SymbolVar*)(baseTable->findInTable(((ExpressionNode*)node->children[k]->children[i])->nameLex))) != nullptr) {
                        ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)node->children[k]->children[i])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true);
                    }
                } else {
                    ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+((ExpressionNode*)node->children[k]->children[i])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true);
                }
                mot = 0;
                ASM.Add(AsmOperation::asm_push, rbx);
                if(((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::arrays &&
                    ((DescriptorArray*)((SymbolVar*)(((SymbolFunc*)(((VarNode*)node)->type))->inputParam->symbolsvec[inputParam]))->type)->isOpen) {
                    for(int y = 0; y < ((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices.size(); ++y) {
                        ASM.Add(AsmOperation::asm_mov, rax,
                                new AsmImn(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices[i])->_max)->nameLex));
                        ASM.Add(AsmOperation::asm_mov, rbx,
                                new AsmImn(((ExpressionNode*)((DescriptorLimited*)((DescriptorArray*)((VarNode*)node->children[k]->children[i])->type->type)->indices[i])->_min)->nameLex));
                        ASM.Add(AsmOperation::asm_sub, rax, rbx);
                        ASM.Add(AsmOperation::asm_add, rax, new AsmImn("1"));
                        ASM.Add(AsmOperation::asm_push, rax);
                    }
                }
            }
            if(((ExpressionNode*)node->children[k]->children[i])->convertType->type == DescriptorTypes::arrays) {
                if(((DescriptorArray*)((SymbolVar*)(((SymbolFunc*)(((VarNode*)node)->type))->inputParam->symbolsvec[inputParam]))->type)->isOpen) {
                    inputParam += ((DescriptorArray*)((SymbolVar*)(((SymbolFunc*)(((VarNode*)node)->type))->inputParam->symbolsvec[inputParam]))->type)->indices.size();
                }
			}
            inputParam++;
        }
        ASM.Add("call " + nameLex);
        int _to = 0;
        if(((VarNode*)node)->type->_class == SymbolTypes::proc) {
            _to = inputParam + popSz;
        } else {
            _to = inputParam + popSz - 1;
        }
        for(int m = 0; m < _to; ++m) {
            ASM.Add(AsmOperation::asm_pop, rbx);
        }
        }
    }
    }

}

void VarNode::generateAsmIdent(bool ifFunc, bool needAsm, bool ifRead) {
    //        std::cout << "!!!!!"<<nameLex<<"\n";

    func(this, ifFunc, needAsm, ifRead);

}

AsmOperand* VarNode::generateAsmFactor(bool ifFunc, bool needAsm, bool ifRead, bool atOrArror) {
    //std::cout << nameLex << "\n";
    if(nameNode == "RANDOM") {
        ASM.Add("CPUID");
        ASM.Add("RDTSC");
        ASM.Add(AsmOperation::asm_push, rax);
        return nullptr;
    }
    if(nameNode == "GETCH") {
        generateAsmCode(children[0], ifFunc, needAsm, ifRead);
        ASM.Add(AsmOperation::asm_pop, rcx);
        ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("16"));
        ASM.Add("call GetAsyncKeyState");
        ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("16"));
        ASM.Add("cmp eax, 0");
        ASM.Add("jnz _GTCH"+std::to_string(intGetch));
        ASM.Add(AsmOperation::asm_push, new AsmImn("0"));
        ASM.Add("jmp __GTCH"+std::to_string(intGetch));
        ASM.Add("_GTCH"+std::to_string(intGetch)+":");
        ASM.Add(AsmOperation::asm_push, new AsmImn("1"));
        ASM.Add("__GTCH"+std::to_string(intGetch)+":");
        intGetch++;
        return nullptr;
    }
    generateAsmIdent(ifFunc, needAsm, ifRead);
    if(id == ((int)Symbols::ident) && ((VarNode*)this)->type->_class == SymbolTypes::proc) {
        return nullptr;
    }
    AsmOperand* shift = nullptr;
    if(needAsm) {
        if(___shift != 0) {
            ASM.Add(AsmOperation::asm_pop, rax);
            ___shift--;
            while(___shift != 0) {
                ASM.Add(AsmOperation::asm_pop, rbx);
                ___shift--;
                ASM.Add(AsmOperation::asm_add, rax, rbx);
            }
            shift = rax;
        }
        if(atOrArror) {
            shift = rax;
        }
    }
    int mov = 0;
    if(atOrArror) {

    } else
    if(convertType->type == DescriptorTypes::pointer) {
        if(needAsm) {
            if(nameLex == "nil") {
                ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);
            } else {
                if(ifFunc) {
                    if(symbolStack->get()->findInTable(nameLex) != nullptr) {
                        mov = symbolStack->get()->movInTable(nameLex);
                        //-
                        ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "-"+std::to_string(mov*8));
                    } else if(symbolStack->get()->parent->findInTable(nameLex) != nullptr) {
                        mov = symbolStack->get()->parent->movInTable(nameLex, false);
                        if(shift != nullptr)
                            ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));

                        ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "+"+std::to_string((mov+1)*8));
                        //ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp+8+"+std::to_string((mov+1)*8), AsmSizeof::s_dq, true, shift));
                    } else if(baseTable->findInTable(nameLex) != nullptr) {
                        ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);
                    }
                } else {
                    ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);//dq true
                }
            }
            ASM.Add(AsmOperation::asm_push, rbx);
        }

    }
    if(atOrArror) {

    } else
    if(convertType->type == DescriptorTypes::scalarInt ||
            convertType->type == DescriptorTypes::scalarFloat ||
            convertType->type == DescriptorTypes::scalarChar ||
            convertType->type == DescriptorTypes::scalarBoolean &&
            (nameLex != "true" && nameLex != "false" && nameLex != "nil")) {

            //std::cout << "%%%\n";
            bool _mot = false;
            if((((Symbol*)(type))->_class != SymbolTypes::func && ((Symbol*)(type))->_class != SymbolTypes::proc)) {
                SymbolVar *svar = nullptr;
                if(ifFunc) {
                    if((svar = (SymbolVar*)(symbolStack->get()->findInTable(nameLex))) != nullptr) {
                        mov = symbolStack->get()->movInTable(nameLex);
                        if(mot == -1) {
                            ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift, "-"+std::to_string(mov*8));
                        } else {
                            if(ifRead) {
                                ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift, "-"+std::to_string((mov)*8));
                            } else if( (((SymbolVar*)(type))->mot) == MethodOfTransmission::paramvar ) {
                                ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8));//shift
                                if(shift != nullptr)
                                    ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                                ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true, shift, "");
                                _mot = true;
                            } else
                                ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "-"+std::to_string(mov*8));
                        }
                    } else if((svar = (SymbolVar*)(symbolStack->get()->parent->findInTable(nameLex))) != nullptr) {
                        mov = symbolStack->get()->parent->movInTable(nameLex);
                        if(shift != nullptr)
                            ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                        if(mot == -1) {
                            ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift, "+"+std::to_string((mov+1)*8));
                        } else {
                            if(ifRead) {
                                ASM.Add(AsmOperation::asm_lea, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift, "+"+std::to_string((mov+1)*8));
                            } else if( (((SymbolVar*)(type))->mot) == MethodOfTransmission::paramvar ) {
                                ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));//shift
                                if(shift != nullptr)
                                    ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                                _mot = true;
                                ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true, shift, "");
                            } else
                                ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "+"+std::to_string((mov+1)*8));
                        }
                    } else if((svar = (SymbolVar*)(baseTable->findInTable(nameLex))) != nullptr) {
                        if(mot == -1) {
                            ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift);
                        } else {
                            if(ifRead) {
                                ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift);
                            } else
                                ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);
                        }
                    }
                } else {
                    //std::cout << "%%%\n";
                    if(mot == -1 && !ifFunc) {
                        ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift);
                    } else {
                        if(ifRead) {
                            ASM.Add(AsmOperation::asm_lea, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_def, true, shift);
                        } else
                            ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);
                    }
                }
                if(svar != nullptr) {
                    mot = (int)(svar->mot);
                    mot = (mot == 0) ? (1) : (-1);
                }
                if(mot == -1 && ifFunc && !_mot) {
                    ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true);
                } else if(_mot){
                    //ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true, shift, "");
                } else {
                    ASM.Add(AsmOperation::asm_push, rbx);
                }
                mot = 0;
            }

        }
        return shift;
}

void BlockActionNode::generateAsm(bool ifFunc, bool needAsm) {

    if(!ifFunc) {
        ASM.Add("section", ".text");
        ASM.Add("global", "CMAIN");

    }

    for(int i = 0; i < children.size(); ++i) {
        generateAsmCode(children[i], ifFunc, needAsm, false);
        if(children[i]->nameNode == "identifier" && ((VarNode*)children[i])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    }
    if(!ifFunc) {
        ASM.Add(AsmOperation::asm_ret);
    }
}

void BeginActionNode::generateAsm(bool ifFunc, bool needAsm) {
    if(!ifFunc && !beginMain) {
        beginMain = true;
        ASM.Add("section", ".text");
        ASM.Add("mark", "CMAIN");
    }
    for(int i = 0; i < children.size(); ++i) {
        generateAsmCode(children[i], ifFunc, needAsm, false);
        //std::cout << children[i]->nameNode << "\n";
        if(children[i]->nameNode == "identifier" && ((VarNode*)children[i])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    }
}

void IfActionNode::generateAsm(bool ifFunc, bool needAsm) {
    for(int i = 0; i < children.size(); ++i) {
        generateAsmCode(children[i], ifFunc, needAsm, false);
        if(children[i]->nameNode == "identifier" && ((VarNode*)children[i])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    }
}

void ElseActionNode::generateAsm(bool ifFunc, bool needAsm) {
    for(int i = 0; i < children.size(); ++i) {
        generateAsmCode(children[i], ifFunc, needAsm, false);
        if(children[i]->nameNode == "identifier" && ((VarNode*)children[i])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    }
}

void WhileActionNode::generateAsm(bool ifFunc, bool needAsm) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;
    ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("WHILE"+std::to_string(_boolOperCounter)+":");
    generateAsmCode(children[0], ifFunc, needAsm, false);
    ASM.Add(AsmOperation::asm_pop, r12);
    ASM.Add(AsmOperation::asm_cmp, r12, new AsmImn("1"));
    ASM.Add(AsmOperation::asm_jnz, new AsmMark("_WHILE"+std::to_string(_boolOperCounter), false));
    generateAsmCode(children[1], ifFunc, needAsm, false);
    if(children[1]->id == (int)(Symbols::ident)) {
        if(children[1]->nameNode == "identifier" && ((VarNode*)children[1])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    }
    ASM.Add(AsmOperation::asm_jmp, new AsmMark("WHILE"+std::to_string(_boolOperCounter), false));
    ASM.Add("_WHILE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
    stackBreak.pop();
}

void RepeatActionNode::generateAsm(bool ifFunc, bool needAsm) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;
    ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("UNTIL"+std::to_string(_boolOperCounter)+":");

    generateAsmCode(children[0], ifFunc, needAsm, false);
    if(children[0]->nameNode == "identifier" && ((VarNode*)children[0])->type->_class != SymbolTypes::proc) {
            ASM.Add(AsmOperation::asm_pop, rax);
        }
    generateAsmCode(children[1], ifFunc, needAsm, false);

    ASM.Add(AsmOperation::asm_pop, rax);
    ASM.Add(AsmOperation::asm_cmp, rax, new AsmImn("0"));
    ASM.Add(AsmOperation::asm_jz, new AsmMark("UNTIL"+std::to_string(_boolOperCounter), false));
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
    stackBreak.pop();
}












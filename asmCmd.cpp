#include "asmCmd.h"

std::string asmOperationToStr(AsmOperation op) {
    switch(op) {
    case AsmOperation::asm_push:
        return "push";
    case AsmOperation::asm_pop:
        return "pop";
    case AsmOperation::asm_add:
        return "add";
    case AsmOperation::asm_not:
        return "not";
    case AsmOperation::asm_addsd:
        return "addsd";
    case AsmOperation::asm_imul:
        return "imul";
    case AsmOperation::asm_mulsd:
        return "mulsd";
    case AsmOperation::asm_sub:
        return "sub";
    case AsmOperation::asm_subsd:
        return "subsd";
    case AsmOperation::asm_ret:
        return "ret";
    case AsmOperation::asm_call:
        return "call";
    case AsmOperation::asm_mov:
        return "mov";
    case AsmOperation::asm_movq:
        return "movq";
    case AsmOperation::asm_cvtsi2sd:
        return "cvtsi2sd";
    case AsmOperation::asm_xor:
        return "xor";
    case AsmOperation::asm_or:
        return "or";
    case AsmOperation::asm_and:
        return "and";
    case AsmOperation::asm_divsd:
        return "divsd";
    case AsmOperation::asm_idiv:
        return "idiv";
    case AsmOperation::asm_sar:
        return "sar";
    case AsmOperation::asm_sal:
        return "sal";
    case AsmOperation::asm_shr:
        return "shr";
    case AsmOperation::asm_shl:
        return "shl";
    case AsmOperation::asm_lea:
        return "lea";
    case AsmOperation::asm_cmp:
        return "cmp";
    case AsmOperation::asm_jmp:
        return "jmp";
    case AsmOperation::asm_jz:
        return "jz";
    case AsmOperation::asm_jnz:
        return "jnz";
    case AsmOperation::asm_jc:
        return "jc";
    case AsmOperation::asm_jnc:
        return "jnc";
    case AsmOperation::asm_ja:
        return "ja";
    case AsmOperation::asm_jna:
        return "jna";

    case AsmOperation::asm_jg:
        return "jg";
    case AsmOperation::asm_jge:
        return "jge";
    case AsmOperation::asm_jl:
        return "jl";
    case AsmOperation::asm_jle:
        return "jle";
    case AsmOperation::asm_test:
        return "test";
    default:
        return "default";
    }
}

AsmLine::AsmLine(std::string _cmd) {
    _type = AsmOperationType::line;
    cmd = _cmd;
}

void AsmLine::print() {
    std::cout << cmd << "\n";
}

AsmSection::AsmSection(std::string _n) {
    _type = AsmOperationType::section;
    name = _n;
}

void AsmSection::print() {
    std::cout << "section " << name << "\n";
}

AsmGlobal::AsmGlobal(std::string _n) {
    _type = AsmOperationType::global;
    name = _n;
}

void AsmGlobal::print() {
    std::cout << "global " << name << "\n";
}

AsmFormat::AsmFormat() {
    _type = AsmOperationType::format;
}

void AsmFormat::Add(std::string s) {
    formats.push_back(s);
}

int AsmFormat::getSize() {
    return formats.size();
}

void AsmFormat::print() {
    if(formats.size() > 0)
        std::cout << "section .data \n";
    for(unsigned int i = 0; i < formats.size(); ++i) {
        std::cout << "format" + std::to_string(i) + ": db " + "\"" +formats[i] + "\"" + ", 0\n";
    }
    std::cout << "_hOut : dq 0\n";
}

AsmConstant::AsmConstant() {
    _type = AsmOperationType::constant;
}

void AsmConstant::Add(std::string name, std::string val) {
    formats.insert ( std::pair<std::string, std::string>(val,"_"+name) );
}

void AsmConstant::Add(std::string s) {
    if(s == "true") {
        formats.insert ( std::pair<std::string, std::string>("1","_"+s) );
    } else if(s == "false") {
        formats.insert ( std::pair<std::string, std::string>("0","_"+s) );
    } else if(s == "nil") {
        formats.insert ( std::pair<std::string, std::string>("0","_"+s) );
    } else
        formats.insert ( std::pair<std::string, std::string>(s,"_"+s) );
}

std::string AsmConstant::Find(std::string v) {
    if(v == "true") {
        return formats["1"];
    } else if(v == "false" || v == "nil") {
        return formats["0"];
    }
    return formats[v];
}

int AsmConstant::getSize() {
    return formats.size();
}

void AsmConstant::print() {
    if(formats.size() > 0)
        std::cout << "section .data \n";
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        std::cout << (*it).second << ":" << " dq " <<  (*it).first << "\n";
    }
}

AsmReg::AsmReg(std::string _n) {
    name = _n;
    _type = AsmOperationType::reg;
}

void AsmReg::print() {
    if(type == AsmSizeof::s_dw) {
        std::cout << " dword ";
    } else if(type == AsmSizeof::s_dq) {
        std::cout << " qword ";
    }
    if(rb) {
        std::cout << "[" << name << "]";
    } else {
        std::cout << name;
    }
}

AsmMark::AsmMark(std::string _n, bool _b = true) {
    colon = _b;
    name = _n;
    _type = AsmOperationType::mark;
}

void AsmMark::print() {
    std::cout << name;
    if(colon)
        std::cout << ":" << "\n";
    else
        std::cout << "\n";
}

AsmImn::AsmImn(std::string _var) {
    var = _var;
    _type = AsmOperationType::imn;
}

void AsmImn::print() {
    std::cout << " " << var;
}

AsmInd::AsmInd(std::string _var, AsmSizeof _t, bool _rb, AsmOperand* _sh) {
    var = _var;
    type = _t;
    rb = _rb;
    shift = _sh;
    _type = AsmOperationType::ind;
}

void AsmInd::print() {
    if(type == AsmSizeof::s_dw) {
        std::cout << " dword ";
    } else if(type == AsmSizeof::s_dq) {
        std::cout << " qword ";
    }
    if(rb) {
        std::cout << "[" << var;
        if(shift != nullptr) {
            std::cout << "+";
            shift->print();
            std::cout << "*8";
        }
        std::cout << "]";
    } else {
        std::cout << var;
    }

}

void AsmCmd::print() {
    std::cout << asmOperationToStr(operation) << " ";
    if(left != nullptr) {
        if(t1 == AsmSizeof::s_db) {
            std::cout << " dword ";
        } else if(t1 == AsmSizeof::s_dq) {
            std::cout << " qword ";
        }
        if(b1) {
            std::cout << "[";
            left->print();
            std::cout << s1str;
            if(s1 != nullptr) {
                std::cout << "+";
                s1->print();
                std::cout << "*8";
            }
            std::cout << "]";
        } else {
            left->print();
        }
    }
    if(right != nullptr) {
        std::cout << ", ";
        if(t2 == AsmSizeof::s_db) {
            std::cout << " dword ";
        } else if(t2 == AsmSizeof::s_dq) {
            std::cout << " qword ";
        }
        if(b2) {
            std::cout << "[";
            right->print();
            std::cout << s2str;
            if(s2 != nullptr) {
                std::cout << "+";
                s2->print();
                std::cout << "*8";
            }
            std::cout << "]";
        } else {
            right->print();
        }

    }
    std::cout << "\n";
}

void AsmProgram::delLast() {
    asmcode.erase(asmcode.begin() + asmcode.size()-1);
}

void AsmProgram::Add(AsmOperation _op, int pos) {
    if(pos != 0) {
        std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
        asmcode.insert(nth, new AsmCmd(_op));
        return;
    }
    asmcode.push_back(new AsmCmd(_op));
}

void AsmProgram::Add(AsmOperation _op, AsmOperand *_l,
            AsmSizeof _t1, bool _b1, AsmOperand* _s1, std::string _s1str, int pos) {
    if(pos != 0) {
        std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
        asmcode.insert(nth, new AsmCmd(_op, _l, _t1, _b1, _s1, _s1str));
        return;
    }
    asmcode.push_back(new AsmCmd(_op, _l, _t1, _b1, _s1, _s1str));
}

void AsmProgram::Add(AsmOperation _op, AsmOperand *_l, AsmOperand *_r,
            AsmSizeof _t1, bool _b1, AsmOperand *_s1, std::string _s1str,
            AsmSizeof _t2, bool _b2, AsmOperand *_s2, std::string _s2str, int pos) {
    if(pos != 0) {
        std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
        asmcode.insert(nth, new AsmCmd(_op, _l, _r, _t1, _b1, _s1, _s1str, _t2, _b2, _s2, _s2str));
        return;
    }
    asmcode.push_back(new AsmCmd(_op, _l, _r, _t1, _b1, _s1, _s1str, _t2, _b2, _s2, _s2str));
}

void AsmProgram::Add(std::string _cmd, std::string _do) {
    if(_cmd == "section") {
        asmcode.push_back(new AsmSection(_do));
    } else if(_cmd == "global") {
        asmcode.push_back(new AsmGlobal(_do));
    } else if(_cmd == "mark") {
        asmcode.push_back(new AsmMark(_do));
    }
}

void AsmProgram::Add(std::string _cmd, int pos) {
    asmcode.push_back(new AsmLine(_cmd));
}

void AsmProgram::Add(std::string _n, std::string _t, int _v, int pos) {
    //asmcode.push_back(new AsmLine(_cmd));
}

void AsmProgram::print() {
    for(unsigned int i = 0; i < asmcode.size(); ++i) {
        asmcode[i]->print();
    }
}


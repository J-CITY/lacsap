#ifndef ASMCMD_H_INCLUDED
#define ASMCMD_H_INCLUDED
#include <iostream>
#include <vector>
#include <map>

enum class AsmOperation {
    asm_ret,//0

    asm_push,//1
    asm_pop,
    asm_call,
    asm_jmp,
    asm_jz,
    asm_jnz,
    asm_jc,
    asm_jnc,
    asm_ja,
    asm_jna,
    asm_jg,
    asm_jge,
    asm_jl,
    asm_jle,

    asm_add,//2
    asm_addsd,
    asm_divsd,
    asm_idiv,
    asm_sub,
    asm_subsd,
    asm_imul,
    asm_mulsd,
    asm_and,
    asm_or,
    asm_xor,
    asm_mov,
    asm_movq,
    asm_cvtsi2sd,
    asm_sar,
    asm_sal,
    asm_lea,
    asm_cmp,
    asm_test,
    asm_not,
    asm_shr,
    asm_shl

};

enum class AsmOperationType {
    line,
    section,
    global,
    format,
    constant,
    reg,
    mark,
    imn,
    ind,
    cmd0,
    cmd1,
    cmd2
};

class AsmCode {
public:
    AsmOperationType _type;
    AsmCode(){}
    virtual void print() = 0;
};

class AsmLine:public AsmCode {
public:
    std::string cmd = "";
    AsmLine(std::string _cmd) {
        _type = AsmOperationType::line;
        cmd = _cmd;
    }
    void print() {
        std::cout << cmd << "\n";
    }
};

class AsmSection:public AsmCode {
public:
    std::string name = "";
    AsmSection(std::string _n) {
        _type = AsmOperationType::section;
        name = _n;
    }
    void print() {
        std::cout << "section " << name << "\n";
    }
};

class AsmGlobal:public AsmCode {
public:
    std::string name = "";
    AsmGlobal(std::string _n) {
        _type = AsmOperationType::global;
        name = _n;
    }
    void print() {
        std::cout << "global " << name << "\n";
    }
};


class AsmFormat:public AsmCode {
public:
    std::vector<std::string> formats;
    AsmFormat() {
        _type = AsmOperationType::format;
    }
    void Add(std::string s) {
        formats.push_back(s);
    }
    int getSize() {
        return formats.size();
    }
    void print() {
        if(formats.size() > 0)
            std::cout << "section .data \n";
        for(unsigned int i = 0; i < formats.size(); ++i) {
            std::cout << "format" + std::to_string(i) + ": db " + "\"" +formats[i] + "\"" + ", 0\n";
        }
        std::cout << "_hOut : dq 0\n";
    }
};

class AsmConstant:public AsmCode {
public:
    std::map<std::string, std::string> formats;
    AsmConstant() {
        _type = AsmOperationType::constant;
    }
    void Add(std::string name, std::string val) {
        formats.insert ( std::pair<std::string, std::string>(val,"_"+name) );
    }
    void Add(std::string s) {
        if(s == "true") {
            formats.insert ( std::pair<std::string, std::string>("1","_"+s) );
        } else if(s == "false") {
            formats.insert ( std::pair<std::string, std::string>("0","_"+s) );
        } else if(s == "nil") {
            formats.insert ( std::pair<std::string, std::string>("0","_"+s) );
        } else
            formats.insert ( std::pair<std::string, std::string>(s,"_"+s) );
    }
    std::string Find(std::string v) {
        if(v == "true") {
            return formats["1"];
        } else if(v == "false" || v == "nil") {
            return formats["0"];
        }
        return formats[v];
    }
    int getSize() {
        return formats.size();
    }
    void print() {
        if(formats.size() > 0)
            std::cout << "section .data \n";
        for (auto it = formats.begin(); it != formats.end(); ++it) {
            std::cout << (*it).second << ":" << " dq " <<  (*it).first << "\n";
        }
    }
};

class AsmOperand:public AsmCode {
public:
    void print() {}
};

enum class AsmSizeof {
    s_def,
    s_db,
    s_dw,
    s_dd,
    s_dq

};

class AsmReg:public AsmOperand {
public:
    AsmSizeof type; //dword qword
    bool rb = false;
    std::string name;
    AsmReg(std::string _n) {
        name = _n;
        _type = AsmOperationType::reg;
    }
    void print() {
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
};

class AsmMark:public AsmOperand {
public:
    std::string name = "";
    bool colon = true;
    AsmMark(std::string _n, bool _b = true) {
        colon = _b;
        name = _n;
        _type = AsmOperationType::mark;
    }
    void print() {
        std::cout << name;
        if(colon)
            std::cout << ":" << "\n";
        else
            std::cout << "\n";
    }
};

class AsmImn:public AsmOperand {
public:
    std::string var;
    AsmImn(std::string _var) {
        var = _var;
        _type = AsmOperationType::imn;
    }
    void print() {
        std::cout << " " << var;
    }
};

class AsmOffset:public AsmOperand {
public:
    void print() {}
};

class AsmInd:public AsmOperand {
public:
    AsmSizeof type; //dword qword
    bool rb = false;
    AsmOperand* shift = nullptr;
    std::string var;
    AsmInd(std::string _var, AsmSizeof _t=AsmSizeof::s_def, bool _rb=false, AsmOperand* _sh = nullptr) {
        var = _var;
        type = _t;
        rb = _rb;
        shift = _sh;
        _type = AsmOperationType::ind;
    }
    void print() {
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
};

std::string asmOperationToStr(AsmOperation op);

class AsmCmd:public AsmCode {
public:
    AsmOperation operation;
    AsmOperand *left = nullptr;
    AsmOperand *right = nullptr;
    AsmOperand *s1 = nullptr, *s2 = nullptr;
    std::string s1str = "", s2str = "";
    AsmSizeof t1=AsmSizeof::s_def, t2=AsmSizeof::s_def;
    bool b1=0, b2=0;

    AsmCmd(){}

    AsmCmd(AsmOperation _op) {
        operation = _op;
        _type = AsmOperationType::cmd0;
    }

    AsmCmd(AsmOperation _op, AsmOperand *_l,
           AsmSizeof _t1=AsmSizeof::s_def, bool _b1=false, AsmOperand *_s1 = nullptr, std::string _s1str = "") {
        operation = _op;
        left = _l;
        t1 = _t1;
        b1 = _b1;
        s1 = _s1;
        s1str = _s1str;
        _type = AsmOperationType::cmd1;
    }

    AsmCmd(AsmOperation _op, AsmOperand *_l, AsmOperand *_r,
           AsmSizeof _t1=AsmSizeof::s_def, bool _b1=false, AsmOperand *_s1 = nullptr, std::string _s1str = "",
           AsmSizeof _t2=AsmSizeof::s_def, bool _b2=false, AsmOperand *_s2 = nullptr, std::string _s2str = "") {
        operation = _op;
        left = _l;
        right = _r;
        t1=_t1;
        t2=_t2;
        b1=_b1;
        b2=_b2;
        s1 = _s1;
        s2 = _s2;
        s1str = _s1str;
        s2str = _s2str;
        _type = AsmOperationType::cmd2;
    }
    void print() {
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

};

class AsmProgram {
public:
    std::vector<AsmCode*> asmcode;

    AsmProgram() {}

    void delLast() {
        asmcode.erase(asmcode.begin() + asmcode.size()-1);
    }

    void Add(AsmOperation _op, int pos = 0) {
        if(pos != 0) {
            std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
            asmcode.insert(nth, new AsmCmd(_op));
            return;
        }
        asmcode.push_back(new AsmCmd(_op));
    }

    void Add(AsmOperation _op, AsmOperand *_l,
             AsmSizeof _t1=AsmSizeof::s_def, bool _b1=false, AsmOperand* _s1 = nullptr, std::string _s1str = "", int pos = 0) {
        if(pos != 0) {
            std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
            asmcode.insert(nth, new AsmCmd(_op, _l, _t1, _b1, _s1, _s1str));
            return;
        }
        asmcode.push_back(new AsmCmd(_op, _l, _t1, _b1, _s1, _s1str));
    }

    void Add(AsmOperation _op, AsmOperand *_l, AsmOperand *_r,
             AsmSizeof _t1=AsmSizeof::s_def, bool _b1=false, AsmOperand *_s1 = nullptr, std::string _s1str = "",
             AsmSizeof _t2=AsmSizeof::s_def, bool _b2=false, AsmOperand *_s2 = nullptr, std::string _s2str = "", int pos = 0) {
        if(pos != 0) {
            std::vector<AsmCode*>::iterator nth = asmcode.begin() + pos;
            asmcode.insert(nth, new AsmCmd(_op, _l, _r, _t1, _b1, _s1, _s1str, _t2, _b2, _s2, _s2str));
            return;
        }
        asmcode.push_back(new AsmCmd(_op, _l, _r, _t1, _b1, _s1, _s1str, _t2, _b2, _s2, _s2str));
    }

    void Add(std::string _cmd, std::string _do) {
        if(_cmd == "section") {
            asmcode.push_back(new AsmSection(_do));
        } else if(_cmd == "global") {
            asmcode.push_back(new AsmGlobal(_do));
        } else if(_cmd == "mark") {
            asmcode.push_back(new AsmMark(_do));
        }
    }

    void Add(std::string _cmd, int pos = 0) {
        asmcode.push_back(new AsmLine(_cmd));

    }

    void Add(std::string _n, std::string _t, int _v, int pos = 0) {
        //asmcode.push_back(new AsmLine(_cmd));

    }

    void print() {
        for(unsigned int i = 0; i < asmcode.size(); ++i) {
            //    std::cout << i << ": ";
            asmcode[i]->print();
        }
    }

};


#endif // ASMCMD_H_INCLUDED

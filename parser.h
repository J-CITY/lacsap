#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
#include <algorithm>
#include "scanner.h"
#include "asmCmd.h"
#include <stack>

AsmOperation getAsmOper(int in, int type);
enum class DescriptorTypes {
	scalarInt,
	scalarFloat,
	scalarChar,
	scalarBoolean,
	arrays,
	records,
	references,
	pointer,
	limited,
	enums
};

enum class SymbolTypes {
	types,
	consts,
	vars,
	func,
	proc
};

enum class MethodOfTransmission {
	paramval,
	paramvar
};

class TreeNode {
public:
    //std::string typenode = "";

	int typeForPrint=0;
	std::string nameNode = "";
	int id;
	TreeNode(){}
	std::vector<TreeNode*> children;
	virtual void print(int l) = 0;
	virtual ~TreeNode() {};
	virtual void generateAsm(bool ifFunc, bool needAsm) {};

	//bool ifFunc = false;
	//bool needAsm = true;
};

class Descriptor {
public:
    std::string name="";
	Descriptor(){}
	DescriptorTypes type;
	virtual void print(int l) = 0;
	virtual ~Descriptor(){}
};

class Symbol {
public:
	SymbolTypes _class;
	Descriptor *type;
	Lexem lex;
	virtual ~Symbol(){
	}
};

extern std::stack<int> stackBreak;
extern std::stack<int> stackReturn;
extern int intGetch;
extern int sizeReturns;
extern AsmReg *rax;
extern AsmReg *rdx;
extern AsmReg *rcx;
extern AsmReg *rbx;
extern AsmReg *rsp;
extern AsmReg *rbp;
extern AsmReg *rsi;
extern AsmReg *r8 ;
extern AsmReg *r12;//for while
extern AsmReg *r13;//for array
extern AsmReg *r14;//for array
extern AsmReg *xmm0;
extern AsmReg *xmm1;
extern AsmReg *xmm2;
extern AsmFormat asmFormat;
extern AsmConstant asmConstants;
extern int boolOperCounter;
extern int ___shift;
extern AsmProgram ASM;
extern AsmProgram ASMOther;
std::string getSize(std::string name, Descriptor* descr);

std::string SymbolTypeToStr(int st);
void printTree(TreeNode *root, int l, int rot);

class DescriptorScalarInt: public Descriptor {
public:
	DescriptorScalarInt() {
		type = DescriptorTypes::scalarInt;
	}
	void print(int l) {
		std::cout << "integer";
	}
	~DescriptorScalarInt() {}
};

class DescriptorScalarFloat: public Descriptor {
public:
	DescriptorScalarFloat() {
		type = DescriptorTypes::scalarFloat;
	}
	void print(int l) {
		std::cout << "real";
	}
	~DescriptorScalarFloat() {}
};

class DescriptorScalarChar: public Descriptor {
public:
	DescriptorScalarChar() {
		type = DescriptorTypes::scalarChar;
	}
	void print(int l) {
		std::cout << "char";
	}
	~DescriptorScalarChar() {}
};

class SymbolConst:public Symbol {
public:
	TreeNode *expr;
	SymbolConst(Lexem _lex, TreeNode *_e, Descriptor * _type) : Symbol() {
		type = _type;
		_class = SymbolTypes::consts;
		lex = _lex;
		expr = _e;
	}
	~SymbolConst() {
		delete expr;
	}
};

class DescriptorScalarBoolean: public Descriptor {
public:
	SymbolConst *_true;
	SymbolConst *_false;
	DescriptorScalarBoolean() {
		type = DescriptorTypes::scalarBoolean;
	}
	void print(int l) {
		std::cout << "boolean " << "{" << _true->lex.lexem << " " << _false->lex.lexem << "}";
	}
	~DescriptorScalarBoolean() {
	}
};

class DescriptorLimited: public Descriptor {
public:
	TreeNode *_min, *_max;
	Descriptor *baseType;
	DescriptorLimited() {
		type = DescriptorTypes::limited;
	}
	void print(int l);
public:
	~DescriptorLimited() {
	    delete _min;
	    delete _max;
	}
};

class DescriptorArray: public Descriptor {
public:
	Descriptor *baseType;
	bool isOpen = false;
	std::vector<Descriptor*> indices;
	DescriptorArray() {
		type = DescriptorTypes::arrays;
	}
	void print(int l);
	~DescriptorArray() {}
};

class DescriptorRecord: public Descriptor {
public:
	std::vector<Symbol*> rName;
	DescriptorRecord() {
		type = DescriptorTypes::records;
	}
	void print(int l);
	~DescriptorRecord() {}
};

class DescriptorReferences: public Descriptor {
public:
	Descriptor *baseType;
	DescriptorReferences() {
		type = DescriptorTypes::references;
	}
	void print(int l) {
		std::cout << "reference\n";
		baseType->print(l);
	}
	~DescriptorReferences() {}
};

class DescriptorPointers: public Descriptor {
public:
	Descriptor *baseType;
	DescriptorPointers() {
		type = DescriptorTypes::pointer;
	}
	void print(int l) {
		std::cout << "pointer ";
		if(baseType != nullptr) {
            baseType->print(l);
		} else {
            std::cout << "nil ";
        }
	}
	~DescriptorPointers() {}
};

class SymbolVar:public Symbol {
public:
	MethodOfTransmission mot = MethodOfTransmission::paramval;
	SymbolVar() : Symbol() {
		_class = SymbolTypes::vars;
	}
	~SymbolVar() {}
};

class SymbolType:public Symbol {
public:
	SymbolType() : Symbol() {
		_class = SymbolTypes::types;
	}
	~SymbolType() {}
};

class SymbolTable {
public:
	SymbolTable *parent = nullptr;
	std::vector<Symbol*> symbolsvec;
	std::map<std::string, Symbol*> symbolsmap;
	std::vector<Descriptor*> descriptors;

	Symbol* findInTable(std::string id) {
		decltype(symbolsmap)::iterator it = symbolsmap.find(id);
		if(it != symbolsmap.end()) {
			return it->second;
		} else {
			return nullptr;
		}
	}

    int movInTable(std::string id, bool param = true) {
        int mov = 0;
        int tableSz = 0;

        for(auto i = 0; i < symbolsvec.size(); ++i) {
            if(((SymbolVar*)(symbolsvec[i]))->mot == MethodOfTransmission::paramvar) {
                tableSz++;
            } else
                tableSz += std::stoi(getSize("", symbolsvec[i]->type));
        }

        for(auto i = 0; i < symbolsvec.size(); ++i) {
            if(symbolsvec[i]->lex.lexem == id) {
                if(!param)
                    return mov;
                else
                    return tableSz - mov;
            } else {
                if(((SymbolVar*)(symbolsvec[i]))->mot == MethodOfTransmission::paramvar) {
                    mov++;
                } else
                    mov += std::stoi(getSize("", symbolsvec[i]->type));
            }
        }
        return -1;
	}

    void addFront(Symbol *s) {
    	symbolsvec.insert(symbolsvec.begin(), s);
		symbolsmap.insert(std::pair<std::string, Symbol*>(s->lex.lexem,
			              symbolsvec[0]));
    }

	void add(Symbol *s) {
		symbolsvec.push_back(s);
		symbolsmap.insert(std::pair<std::string, Symbol*>(s->lex.lexem,
			              symbolsvec[symbolsvec.size()-1]));
	}
	void print(int l);
	~SymbolTable() {
	    delete parent;
	    for(unsigned int i = 0; i < symbolsvec.size(); ++i) {
            delete symbolsvec[i];
	    }
	    for(unsigned int i = 0; i < descriptors.size(); ++i) {
            delete descriptors[i];
	    }
	}
};

class SymbolProc:public Symbol {
public:
	SymbolTable *localParam;
	SymbolTable *inputParam;
	TreeNode *block;
	SymbolProc() : Symbol() {
 		inputParam = new SymbolTable();
 		localParam = new SymbolTable();
 		_class = SymbolTypes::proc;
	}
public:
	~SymbolProc() {
		delete localParam;
		delete inputParam;
	}
};

class SymbolFunc:public SymbolProc {
public:
	SymbolType *returnParam;
	SymbolFunc() : SymbolProc() {
		_class = SymbolTypes::func;
	}
	~SymbolFunc() {
	    delete localParam;
		delete inputParam;
	}
};

class SymbolStack {
public:
	SymbolTable *symbolTable;
	void pop() {
		symbolTable = symbolTable->parent;
	}
	SymbolTable *get() {
		return symbolTable;
	}
	void push(SymbolTable *in) {
		in->parent = symbolTable;
		symbolTable = in;
	}
	SymbolStack() {
		symbolTable = nullptr;
	}
	Symbol* findInTables(std::string id) {
		SymbolTable *st = symbolTable;
		Symbol *it = st->findInTable(id);
		while(true) {
			if(it != nullptr) {
				return it;
			} else {
				if(st->parent != nullptr) {
					st = st->parent;
					it = st->findInTable(id);
				} else {
					return nullptr;
				}
			}
		}
	}
	/*int movInTables(std::string id) {
        int mov = 0;
        for(auto i = 0; i < symbolsvec.size(); ++i) {
            if(symbolsvec[i]->lex == id) {
                return mov;
            } else {
                mov += std::stoi(getSize(symbolsvec[i].type));
            }
        }
        return -1;
	}*/
	~SymbolStack() {
		delete symbolTable;
	}
};

void generateVars(SymbolTable *symt, bool ifFunc);
std::string genAsmDerective(Descriptor *type);

void genAsmCodeForIdent(TreeNode *ident, TreeNode *expr, AsmOperand* shift);
void genAsmCodeForIdentFunc(TreeNode *ident, TreeNode *expr, AsmOperand* shift);
void generateAsmCode(TreeNode *root, bool ifFunc, bool needAsm, bool ifRead);
int pushFuncParam(bool ifFunc, std::string name, Descriptor *descr, int _mov);
extern int mot;//-1 var; 1 val; 0 def;
extern SymbolTable *baseTable;
extern SymbolTable *mainTable;
extern SymbolStack *Stack;
extern SymbolStack *symbolStack;



class StringNode:public TreeNode {
public:
	std::string nameLex;
	StringNode(std::string _chN, Lexem lex) {
		nameNode = _chN;
		nameLex = lex.lexem;
		id = lex.val;

	}
	void print(int l) {
		std::cout << nameLex << ":" << nameNode;
	}
	void generateAsm(bool ifFunc, bool needAsm) {

	}
	~StringNode() {}
};

class ExpressionNode:public TreeNode {
public:
	Descriptor *convertType;

	int operation_type;
	std::string nameLex;
	ExpressionNode(std::string _chN, Lexem lex) {

		nameNode = _chN;
		operation_type = lex.type;
		nameLex = lex.lexem;
		id = lex.val;

		typeForPrint = 1;
	}
	virtual void print(int l) {
		std::cout << nameLex << ":" << nameNode <<"{";
		if(convertType != nullptr) {
			convertType->print(l);
		}
		std::cout << "}";
	}
	~ExpressionNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
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
        if(nameLex == "^") {
            if(ifFunc) {
                if(symbolStack->get()->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                    mov = symbolStack->get()->movInTable(((ExpressionNode*)children[0])->nameLex);
                    //-
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "-"+std::to_string(mov*8));
                    //ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
                    ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true);
                } else if(symbolStack->get()->parent->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                    mov = symbolStack->get()->parent->movInTable(((ExpressionNode*)children[0])->nameLex);
                    if(shift != nullptr)
                        ASM.Add(AsmOperation::asm_imul, shift, new AsmImn("-1"));
                    ASM.Add(AsmOperation::asm_mov, rbx, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "+"+std::to_string((mov+1)*8));
                    //ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
                    ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true);
                } else if(baseTable->findInTable(((ExpressionNode*)children[0])->nameLex) != nullptr) {
                    ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+((ExpressionNode*)children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);
                    ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true);
                }
            } else {
                ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("v_"+((ExpressionNode*)children[0])->nameLex), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift);//dq true
                ASM.Add(AsmOperation::asm_push, rbx, AsmSizeof::s_dq, true);
            }

        }
	    }
	}
};

class ActionNode:public TreeNode {
public:
	std::string nameLex = "";
	ActionNode(std::string in) {
		nameNode = in;
		nameLex = in;
	}
	void print(int l) {
		std::cout << nameNode;
	}
	void generateAsm(bool ifFunc, bool needAsm) {
	    if(nameLex == "SLEEP") {
            generateAsmCode(children[0], ifFunc, needAsm, false);
            ASM.Add(AsmOperation::asm_pop, rcx);
            ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("80"));
            ASM.Add("call Sleep");
            ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("80"));
	    } else
	    if(nameLex == "INITSCR") {
            //generateAsmCode(children[0], ifFunc, needAsm, false);
            //asmConstants.Add("hOut", "0");
            ASM.Add(AsmOperation::asm_mov, rcx, new AsmImn("-11"));
            ASM.Add("call GetStdHandle");
            ASM.Add("mov qword [_hOut], rax");
	    } else
	    if(nameLex == "GOTOXY") {
            generateAsmCode(children[1], ifFunc, needAsm, false);
            generateAsmCode(children[0], ifFunc, needAsm, false);
            ASM.Add(AsmOperation::asm_pop, rdx);
            ASM.Add(AsmOperation::asm_shl, rdx, new AsmImn("16"));
            ASM.Add(AsmOperation::asm_pop, rbx);
            ASM.Add(AsmOperation::asm_add, rdx, rbx);
            ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("_hOut"), AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr);
            ASM.Add("call SetConsoleCursorPosition");
	    } else
        if(nameLex == "WRITE") {
            std::string format = "";
			for(int i = 0; i < children.size(); ++i) {
                if(children[i]->id == (int)Symbols::sys_string) {
                    format += ((StringNode*)children[i])->nameLex;

                    ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize())), AsmSizeof::s_def, false);
                    asmFormat.Add(format);
                    ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("40"));
                    ASM.Add(AsmOperation::asm_call, new AsmInd("printf", AsmSizeof::s_def, false));
                    ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("40"));
                    format = "";
                    continue;
                } else {
                    generateAsmCode(children[i], ifFunc, needAsm, false);
                    AsmOperand* shift = nullptr;
                    ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize())), AsmSizeof::s_def, false);

                    if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarInt) {
                        format += "%d";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarFloat) {
                        format += "%f";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarBoolean) {
                        format += "%d";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarChar) {
                        format += "%c";
                    } else {
                        format += "%d";
                    }
                    ASM.Add(AsmOperation::asm_pop, rdx);
                    asmFormat.Add(format);
                    ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("40"));
                    ASM.Add(AsmOperation::asm_call, new AsmInd("printf", AsmSizeof::s_def, false));
                    ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("40"));
                    format = "";
                }
			}
        } else if(nameLex == "READ") {
                std::string format = "";
                for(int i = 0; i < children.size(); ++i) {
                    generateAsmCode(children[i], ifFunc, needAsm, true);

                    ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize())), AsmSizeof::s_def, false);
                    if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarInt) {
                        format += "%d";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarFloat) {
                        format += "%lf";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarBoolean) {
                        format += "%d";
                    } else if(((ExpressionNode*)children[i])->convertType->type == DescriptorTypes::scalarChar) {
                        format += "%c";
                    } else {
                        format += "%d";
                    }
                    ASM.Add(AsmOperation::asm_pop, rdx);
                    asmFormat.Add(format);
                    ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("40"));
                    ASM.Add(AsmOperation::asm_call, new AsmInd("scanf", AsmSizeof::s_def, false));
                    ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("40"));
                    format = "";
                }

			} else if(nameLex == "BRANCH") {
                generateAsmCode(children[0], ifFunc, needAsm, false);
                int _boolOperCounter = boolOperCounter;
                boolOperCounter++;
                ASM.Add(AsmOperation::asm_pop, rax);
                ASM.Add(AsmOperation::asm_cmp, rax, new AsmImn("1"));
                ASM.Add(AsmOperation::asm_jnz, new AsmMark("IFELSE"+std::to_string(_boolOperCounter), false));
                generateAsmCode(children[1], ifFunc, needAsm, false);
                ASM.Add(AsmOperation::asm_jmp, new AsmMark("_IFELSE"+std::to_string(_boolOperCounter), false));
                ASM.Add("IFELSE"+std::to_string(_boolOperCounter)+":");
                if(children.size() == 3) {
                    generateAsmCode(children[2], ifFunc, needAsm, false);
                }
                ASM.Add("_IFELSE"+std::to_string(_boolOperCounter)+":");
			} else if(nameLex == "return") {
                ASM.Add(AsmOperation::asm_jmp, new AsmMark("RETURN"+std::to_string(stackReturn.size()), false));
			}
	}
	~ActionNode(){}
};

class BlockActionNode:public ActionNode {
public:
	BlockActionNode(std::string in):ActionNode(in) {}
	~BlockActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

extern bool beginMain;
class BeginActionNode:public ActionNode {
public:
	BeginActionNode(std::string in):ActionNode(in) {}
	~BeginActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class AssignActionNode:public ActionNode {
public:
	AssignActionNode(std::string in):ActionNode(in) {}
	~AssignActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {

	}
};

class IfActionNode:public ActionNode {
public:
	IfActionNode(std::string in):ActionNode(in) {}
	~IfActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class ElseActionNode:public ActionNode {
public:
	ElseActionNode(std::string in):ActionNode(in) {}
	~ElseActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class ForActionNode:public ActionNode {
public:
	ForActionNode(std::string in):ActionNode(in) {}
	~ForActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class WhileActionNode:public ActionNode {
public:
	WhileActionNode(std::string in):ActionNode(in) {}
	~WhileActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class RepeatActionNode:public ActionNode {
public:
	RepeatActionNode(std::string in):ActionNode(in) {}
	~RepeatActionNode() {}
    void generateAsm(bool ifFunc, bool needAsm);
};

class BreakActionNode:public ActionNode {
public:
	BreakActionNode(std::string in):ActionNode(in){}
	~BreakActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
        ASM.Add(AsmOperation::asm_jmp, new AsmMark("BREAK"+std::to_string(stackBreak.top()), false));
	}
};

class ContinueActionNode:public ActionNode {
public:
	ContinueActionNode(std::string in):ActionNode(in) {}
	~ContinueActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
        ASM.Add(AsmOperation::asm_jmp, new AsmMark("CONTINUE"+std::to_string(stackBreak.top()), false));
	}
};

class FreeActionNode:public ActionNode {
public:
	FreeActionNode(std::string in):ActionNode(in) {}
	~FreeActionNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {

	}
};

class LiteralNode:public ExpressionNode {
public:
	Descriptor *type;
	LiteralNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~LiteralNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
	}
};


class CharLiteralNode:public LiteralNode {
public:
	CharLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~CharLiteralNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
        ASM.Add(AsmOperation::asm_push, new AsmImn(std::to_string(nameLex[1])));
	}
};

class VarNode:public ExpressionNode {
public:
	Symbol *type;
	bool ifRead = false;
	VarNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {
		typeForPrint = 2;
	}
    void func(TreeNode* node, bool ifFunc, bool needAsm, bool ifRead);
	void generateAsmIdent(bool ifFunc, bool needAsm, bool ifRead = false);

	AsmOperand*  generateAsmFactor(bool ifFunc, bool needAsm, bool ifRead = false, bool atOrArror = false);

	void generateAsm(bool ifFunc, bool needAsm) {
	}

	~VarNode() {}
};

class ProgramNode:public VarNode {
public:
	SymbolFunc *type = new SymbolFunc();
	std::string nameLex = "";

	ProgramNode(Lexem lex) : VarNode("PROGRAM", lex) {
		typeForPrint = 2;
		nameNode = "PROGRAM";
		nameLex = lex.lexem;
		id = lex.val;
	}

	void print(int l) {
		std::cout << nameNode << " : " << nameLex << "\n";
		type->localParam->print(l);
	}
	~ProgramNode() {
		//delete type;
	}
	void generateAsm(bool ifFunc, bool needAsm) {
	    //if(!ifFunc)
        ASMOther.Add("section", ".data");
	    generateVars(type->localParam, false);
        generateAsmCode(children[0], ifFunc, needAsm, false);
	}
};

class ProcNode:public VarNode {
public:
	SymbolProc *type = nullptr;
	std::string nameLex = "";

	ProcNode(Lexem lex) : VarNode("PROC", lex) {
		typeForPrint = 2;
		nameNode = "PROC";
		nameLex = lex.lexem;
		id = lex.val;
	}

	void print(int l);
	~ProcNode() {
	}
	void generateAsm(bool ifFunc, bool needAsm) {
	    //local
	    //input
	    symbolStack->push(mainTable);
	    symbolStack->push(type->inputParam);
	    symbolStack->push(type->localParam);

        stackReturn.push(stackReturn.size()+1);
        sizeReturns++;

        ASMOther.Add("section", ".text");
		ASM.Add(type->lex.lexem+":");
		ASM.Add("push rbp");
		ASM.Add("mov rbp, rsp");
	    generateVars(type->localParam, true);

		generateAsmCode(type->block, true, needAsm, false);

        ASM.Add("RETURN"+std::to_string(sizeReturns)+":");

		int _add = 0;
		for(int i = 0; i < type->localParam->symbolsvec.size(); ++i) {
            _add += 8;
            //ASM.Add(AsmOperation::asm_pop, rbx);
		}
		ASM.Add(AsmOperation::asm_add, rsp, new AsmImn(std::to_string(_add)));

        ASM.Add("mov rsp, rbp");
        ASM.Add("pop rbp");
		ASM.Add("ret");

		stackReturn.pop();

		symbolStack->pop();
		symbolStack->pop();
		symbolStack->pop();
	}
};

class FuncNode:public VarNode {
public:
	SymbolFunc *type = nullptr;
	std::string nameLex = "";

	FuncNode(Lexem lex) : VarNode("FUNC", lex) {
		typeForPrint = 2;
		nameNode = "FUNC";
		nameLex = lex.lexem;
		id = lex.val;
	}

	void print(int l);
	~FuncNode() {
	}
    void generateAsm(bool ifFunc, bool needAsm) {
        symbolStack->push(mainTable);
        symbolStack->push(type->inputParam);
        symbolStack->push(type->localParam);

        stackReturn.push(stackReturn.size()+1);
        sizeReturns++;


        ASMOther.Add("section", ".text");
		ASM.Add(type->lex.lexem+":");
		ASM.Add("push rbp");
		ASM.Add("mov rbp, rsp");
        generateVars(type->localParam, true);
		generateAsmCode(type->block, true, needAsm, false);


		ASM.Add("RETURN"+std::to_string(sizeReturns)+":");

		int _add = 0;
		for(int i = 0; i < type->localParam->symbolsvec.size(); ++i) {
            _add += 8;
            //ASM.Add(AsmOperation::asm_pop, rbx);
		}
		ASM.Add(AsmOperation::asm_add, rsp, new AsmImn(std::to_string(_add)));
        ASM.Add("mov rsp, rbp");
        ASM.Add("pop rbp");
		ASM.Add("ret");

        stackReturn.pop();

		symbolStack->pop();
		symbolStack->pop();
		symbolStack->pop();
	}
};

class BinOperationNode:public ExpressionNode {
public:
    AsmOperand* shiftFor = nullptr;
	BinOperationNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~BinOperationNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class UnarOperationNode:public ExpressionNode {
public:
	UnarOperationNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~UnarOperationNode() {}
	void generateAsm(bool ifFunc, bool needAsm);
};

class BoolLiteralNode:public LiteralNode {
public:
	BoolLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~BoolLiteralNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
        asmConstants.Add(nameLex);
        ASM.Add(AsmOperation::asm_push, new AsmInd(asmConstants.Find(nameLex)), AsmSizeof::s_dq, true);
	}
};

class IntLiteralNode:public LiteralNode {
public:
	IntLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~IntLiteralNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {
        ASM.Add(AsmOperation::asm_push, new AsmImn(nameLex));
	}
};

class FloatLiteralNode:public LiteralNode {
public:
	FloatLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex){}
	void generateAsm(bool ifFunc, bool needAsm) {
	    std::string _nameLex = nameLex;
	    std::size_t found = nameLex.find("+");
        if (found!=std::string::npos) {
            _nameLex.replace(found, 1, "p");
        }
        found = nameLex.find("-");
        if (found!=std::string::npos) {
            _nameLex.replace(found, 1, "p");
        }
        asmConstants.Add(_nameLex, nameLex);
        ASM.Add(AsmOperation::asm_push, new AsmInd(asmConstants.Find(nameLex)), AsmSizeof::s_dq, true);
	}
};



class RecordLiteralNode:public LiteralNode {
public:
	RecordLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex){}
	~RecordLiteralNode() {}
	void generateAsm(bool ifFunc, bool needAsm) {

	}
};

Descriptor *castSecontToFirst(Descriptor *d1, Descriptor *d2);
Descriptor *cast(Descriptor *d1, Descriptor *d2);



class Parser {
public:
	Error error;
	Parser(std::string _filePath);
	~Parser();
	void Parse();
	void print();
	void optimization();
	void printAsm() {
        ASMOther.print();
        ASM.print();
        asmFormat.print();
        asmConstants.print();
	}
private:

    ExpressionNode *Calculate(ExpressionNode * expr);

	Scanner scanner;
	ProgramNode *root = nullptr;
	bool inCycle = false;

	ExpressionNode *getIdent(bool ifFunc, Descriptor **recordDescr, bool ifRecord, bool needAsm);
	ExpressionNode *factor(bool ifFunc, bool needAsm, bool ifRead);
	ExpressionNode *term(bool ifFunc, bool needAsm);
	ExpressionNode *expression(bool ifFunc, bool needAsm);
	ExpressionNode *condition(bool ifFunc, bool needAsm);

	TreeNode *statement(bool ifFunc);
	TreeNode *statementIf(bool ifFunc);
	TreeNode *statementWhile(bool ifFunc);
	TreeNode *statementFor(bool ifFunc);
	TreeNode *statementUntil(bool ifFunc);

	std::vector<SymbolVar*> getVarBlock(int ifFunc, SymbolTable *st, bool forFunc, bool ifRecord);
	TreeNode *block(bool ifFunc, SymbolTable* st);
	void program();

	Descriptor *getType(SymbolTable *st, bool forFunc);
	Descriptor *getArrLimit();

	bool expect(Symbols symacc);
	bool accept(Symbols symacc);

	ExpressionNode *getOrd(bool ifFunc, bool needAsm);
	ExpressionNode *getChr(bool ifFunc, bool needAsm);

};

#endif // PARSER_H_INCLUDED

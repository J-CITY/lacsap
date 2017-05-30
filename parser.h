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
	int typeForPrint=0;
	std::string nameNode = "";
	int id;
	TreeNode(){}
	std::vector<TreeNode*> children;
	virtual void print(int l) = 0;
	virtual ~TreeNode() {};
	virtual void generateAsm() {};
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
	void generateAsm() {

	}
	~StringNode() {}
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
	void generateAsm() {
        if(nameLex == "write") {

        }
	}
	~ActionNode(){}
};

class BlockActionNode:public ActionNode {
public:
	BlockActionNode(std::string in):ActionNode(in) {}
	~BlockActionNode() {}
	void generateAsm() {

	}
};

class AssignActionNode:public ActionNode {
public:
	AssignActionNode(std::string in):ActionNode(in) {}
	~AssignActionNode() {}
	void generateAsm() {

	}
};

class IfActionNode:public ActionNode {
public:
	IfActionNode(std::string in):ActionNode(in) {}
	~IfActionNode() {}
	void generateAsm() {

	}
};

class ElseActionNode:public ActionNode {
public:
	ElseActionNode(std::string in):ActionNode(in) {}
	~ElseActionNode() {}
	void generateAsm() {

	}
};

class ForActionNode:public ActionNode {
public:
	ForActionNode(std::string in):ActionNode(in) {}
	~ForActionNode() {}
	void generateAsm() {

	}
};

class WhileActionNode:public ActionNode {
public:
	WhileActionNode(std::string in):ActionNode(in) {}
	~WhileActionNode() {}
	void generateAsm() {

	}
};

class RepeatActionNode:public ActionNode {
public:
	RepeatActionNode(std::string in):ActionNode(in) {}
	~RepeatActionNode() {}
    void generateAsm() {

	}
};

class BreakActionNode:public ActionNode {
public:
	BreakActionNode(std::string in):ActionNode(in){}
	~BreakActionNode() {}
	void generateAsm() {

	}
};

class ContinueActionNode:public ActionNode {
public:
	ContinueActionNode(std::string in):ActionNode(in) {}
	~ContinueActionNode() {}
	void generateAsm() {

	}
};

class FreeActionNode:public ActionNode {
public:
	FreeActionNode(std::string in):ActionNode(in) {}
	~FreeActionNode() {}
	void generateAsm() {

	}
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
	void generateAsm() {

	}
};

class VarNode:public ExpressionNode {
public:
	Symbol *type;
	VarNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {
		typeForPrint = 2;
	}
	void generateAsm() {
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
	void generateAsm() {

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
	void generateAsm(AsmProgram *ASM, bool rOrl) {

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
    void generateAsm(AsmProgram *ASM, bool rOrl) {

	}
};

class BinOperationNode:public ExpressionNode {
public:
	BinOperationNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~BinOperationNode() {}
	void generateAsm() {
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
		}
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
		}
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
};

class UnarOperationNode:public ExpressionNode {
public:
	UnarOperationNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~UnarOperationNode() {}
	void generateAsm() {
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
};

class LiteralNode:public ExpressionNode {
public:
	Descriptor *type;
	LiteralNode(std::string _chN, Lexem lex):ExpressionNode(_chN, lex) {}
	~LiteralNode() {}
	void generateAsm() {

	}
};

class BoolLiteralNode:public LiteralNode {
public:
	BoolLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~BoolLiteralNode() {}
	void generateAsm() {

	}
};

class IntLiteralNode:public LiteralNode {
public:
	IntLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~IntLiteralNode() {}
	void generateAsm() {
        ASM.Add(AsmOperation::asm_push, new AsmImn(nameLex));
	}
};

class FloatLiteralNode:public LiteralNode {
public:
	FloatLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex){}
	void generateAsm() {
        asmConstants.Add(nameLex);
        ASM.Add(AsmOperation::asm_push, new AsmInd(asmConstants.Find(nameLex), AsmSizeof::s_dq, true));
	}
};

class CharLiteralNode:public LiteralNode {
public:
	CharLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex) {}
	~CharLiteralNode() {}
	void generateAsm() {
        ASM.Add(AsmOperation::asm_push, new AsmImn(std::to_string(nameLex[1])));
	}
};

class RecordLiteralNode:public LiteralNode {
public:
	RecordLiteralNode(std::string _chN, Lexem lex):LiteralNode(_chN, lex){}
	~RecordLiteralNode() {}
	void generateAsm() {

	}
};

Descriptor *castSecontToFirst(Descriptor *d1, Descriptor *d2);
Descriptor *cast(Descriptor *d1, Descriptor *d2);

class Parser {
public:

	SymbolStack *Stack;
	SymbolStack *symbolStack;
	Error error;
	Parser(std::string _filePath);
	~Parser();
	void Parse();
	void print();
	void printAsm() {
        ASMOther.print();
        ASM.print();
        asmFormat.print();
        asmConstants.print();
	}
private:
    int mot = 0;//-1 var; 1 val; 0 def;

    void genAsmCodeForIdentFunc(ExpressionNode *ident, ExpressionNode *expr, AsmOperand* shift);
    void genAsmCodeForIdent(ExpressionNode *ident, ExpressionNode *expr, AsmOperand* shift);

    ExpressionNode *Calculate(ExpressionNode * expr);

    SymbolTable *baseTable = nullptr;

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

	int pushFuncParam(bool ifFunc, std::string name, Descriptor *descr, int _mov = 0) {
	    int mov = 0;
        AsmImn* shift = nullptr;
        int popSz = 0;

	    if(descr->type != DescriptorTypes::arrays &&
            descr->type != DescriptorTypes::records) {
                popSz++;
                if(!ifFunc) {
                    shift = new AsmImn(std::to_string(_mov));
                    ASM.Add(AsmOperation::asm_push, new AsmInd("v_"+name, AsmSizeof::s_dq, true, shift));
                } else {
                    if(symbolStack->get()->findInTable(name) != nullptr) {
                        mov = symbolStack->get()->movInTable(name);
                        ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp-"+std::to_string((mov+1+_mov)*8), AsmSizeof::s_def, true));
                    } else if(symbolStack->get()->parent->findInTable(name) != nullptr) {
                        mov = symbolStack->get()->parent->movInTable(name);
                        ASM.Add(AsmOperation::asm_mov, rbx, new AsmInd("rbp+8+"+std::to_string(mov+1+_mov*8), AsmSizeof::s_def, true));
                    } else if(baseTable->findInTable(name) != nullptr) {
                        shift = new AsmImn(std::to_string(_mov));
                        ASM.Add(AsmOperation::asm_push, new AsmInd("v_"+name, AsmSizeof::s_dq, true, shift));
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
};

#endif // PARSER_H_INCLUDED

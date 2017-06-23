#include "parser.h"

AsmReg *rax = new AsmReg("rax");
AsmReg *rdx = new AsmReg("rdx");
AsmReg *rcx = new AsmReg("rcx");
AsmReg *rbx = new AsmReg("rbx");
AsmReg *rsp = new AsmReg("rsp");
AsmReg *rbp = new AsmReg("rbp");
AsmReg *rsi = new AsmReg("rsi");
AsmReg *r8 = new AsmReg("r8");
AsmReg *r12 = new AsmReg("r12");//for array
AsmReg *r13 = new AsmReg("r13");//for array
AsmReg *r14 = new AsmReg("r14");//for array
AsmReg *xmm0 = new AsmReg("xmm0");
AsmReg *xmm1 = new AsmReg("xmm1");
AsmReg *xmm2 = new AsmReg("xmm2");
AsmFormat asmFormat;
AsmConstant asmConstants;
int boolOperCounter = 0;
int ___shift = 0;
AsmProgram ASM;
AsmProgram ASMOther;
std::stack<int> stackBreak;
std::stack<int> stackReturn;
int intGetch = 0;
bool beginMain = false;

int mot = 0;
SymbolTable *baseTable = nullptr;
SymbolTable *mainTable = nullptr;
SymbolStack *Stack = nullptr;
SymbolStack *symbolStack = nullptr;
int sizeReturns = 0;

Parser::Parser(std::string _filePath) {
	scanner.open(_filePath);
	scanner.read();
}

void Parser::Parse() {
	program();
	boolOperCounter = 0;
}

void Parser::program() {
	symbolStack = new SymbolStack();

	baseTable = new SymbolTable();
	Descriptor *intType = new DescriptorScalarInt();
	Descriptor *realType = new DescriptorScalarFloat();
	Descriptor *charType = new DescriptorScalarChar();
	Descriptor *booleanType = new DescriptorScalarBoolean();
	baseTable->descriptors.push_back(intType);
	baseTable->descriptors.push_back(realType);
	baseTable->descriptors.push_back(charType);
	baseTable->descriptors.push_back(booleanType);

	SymbolType *st = new SymbolType();

	TextPos pos;
	Lexem lex;
	lex.lexem = "integer";
	lex.val = (int)Symbols::ident;
	lex.type = identifier;
	lex.pos = pos;
	st->lex = lex;
	st->type = intType;
	baseTable->add(st);

	st = new SymbolType();
	lex.lexem = "real";
	st->lex = lex;
	st->type = realType;
	baseTable->add(st);

	st = new SymbolType();
	lex.lexem = "char";
	st->lex = lex;
	st->type = charType;
	baseTable->add(st);

	Lexem maxlex((std::string)"32768", (TextPos)pos, (int)Symbols::intc, (int)literalint);
	Lexem maxlexName((std::string)"maxint", (TextPos)pos, (int)Symbols::ident, (int)identifier);
	IntLiteralNode *cst = new IntLiteralNode((std::string)"32768", maxlex);
	baseTable->add(new SymbolConst(maxlexName, cst, intType));

	Lexem falselex((std::string)"0", (TextPos)pos, (int)Symbols::intc, (int)literalint);
	Lexem falselexName((std::string)"false", (TextPos)pos, (int)Symbols::sys_false, (int)keyword);
	cst = new IntLiteralNode((std::string)"0", falselex);
	SymbolConst *_false = new SymbolConst(falselexName, cst, intType);
	baseTable->add(_false);

	Lexem truelex((std::string)"1", (TextPos)pos, (int)Symbols::intc, (int)literalint);
	Lexem truelexName((std::string)"true", (TextPos)pos, (int)Symbols::sys_true, (int)keyword);
	cst = new IntLiteralNode((std::string)"1", truelex);
	SymbolConst *_true = new SymbolConst(truelexName, cst, intType);
	baseTable->add(_true);
	symbolStack->push(baseTable);

	st = new SymbolType();
	lex.lexem = "boolean";
	st->lex = lex;
	st->type = booleanType;
	((DescriptorScalarBoolean *)(st->type))->_true = _true;
	((DescriptorScalarBoolean *)(st->type))->_false = _false;
	baseTable->add(st);

	SymbolVar *nil = new SymbolVar();
	DescriptorPointers *nilDescriptor = new DescriptorPointers();
	nilDescriptor->baseType = nullptr;
	Lexem nilLexem((std::string)"nil", (TextPos)pos, (int)Symbols::sys_nil, (int)keyword);
	nil->type = nilDescriptor;
	nil->lex = nilLexem;
	baseTable->add(nil);

    ASMOther.Add("%include \"io64.inc\"", 0);
    ASMOther.Add("extern GetAsyncKeyState", 0);
    ASMOther.Add("extern Sleep", 0);
    ASMOther.Add("extern GetStdHandle", 0);
    ASMOther.Add("extern SetConsoleCursorPosition", 0);

	expect(Symbols::sys_program);
	scanner.getNextLexem();

	if(expect(Symbols::ident)) {
		root = new ProgramNode(scanner.getLexem());
		scanner.getNextLexem();
	}
	expect(Symbols::semicolon);
	scanner.getNextLexem();

	mainTable = new SymbolTable();
	TreeNode *blockNode;
	blockNode = block(false, mainTable);

	root->type->localParam = mainTable;
	root->children.push_back(blockNode);
	expect(Symbols::point);

	generateAsmCode(root, false, true, false);
}

TreeNode *Parser::statementIf(bool ifFunc) {
    TreeNode *br = new ActionNode("BRANCH");
    scanner.getNextLexem();
    ExpressionNode *expr = condition(ifFunc, true);
    if(expr->convertType->type != DescriptorTypes::scalarBoolean) {
        //ERROR must be int
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
    }
    expect(Symbols::sys_then);
    scanner.getNextLexem();

    IfActionNode *ifbody = new IfActionNode("IF_BODY");
    if(accept(Symbols::semicolon)) {
        ifbody->children.push_back(new FreeActionNode("FREE_NODE"));
    } else {
        ifbody->children.push_back(statement(ifFunc));
    }
    br->children.push_back(expr);
    br->children.push_back(ifbody);
    if(accept(Symbols::semicolon)) {
        scanner.getNextLexem();
    }
    if(accept(Symbols::sys_else)) {
        scanner.getNextLexem();
        ElseActionNode *elsebody = new ElseActionNode("ELSE_BODY");

        if(accept(Symbols::semicolon)) {
            elsebody->children.push_back(new FreeActionNode("FREE_NODE"));
        } else {
            elsebody->children.push_back(statement(ifFunc));
        }
        br->children.push_back(elsebody);
        if(accept(Symbols::semicolon)) {

        }
    } else {
        scanner.getPrevLexem();
    }
    return br;
}

TreeNode *Parser::statementWhile(bool ifFunc) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;

    WhileActionNode *wh = new WhileActionNode(scanner.getLexem().lexem);
    scanner.getNextLexem();

    ExpressionNode *expr = condition(ifFunc, true);
    if(expr->convertType->type != DescriptorTypes::scalarBoolean) {
        //ERROR must be int
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
    }
    expect(Symbols::sys_do);
    scanner.getNextLexem();
    wh->children.push_back(expr);

    if(accept(Symbols::semicolon)) {
        wh->children.push_back(new FreeActionNode("FREE_NODE"));
        //scanner.getNextLexem();
    } else {
        wh->children.push_back(statement(ifFunc));
    }
    stackBreak.pop();
    return wh;
}

TreeNode *Parser::statementRepeat(bool ifFunc) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;

    RepeatActionNode *rt = new RepeatActionNode(scanner.getLexem().lexem);
    scanner.getNextLexem();
    if(accept(Symbols::semicolon)) {
        rt->children.push_back(new FreeActionNode("FREE_NODE"));
    } else {
        rt->children.push_back(statement(ifFunc));
    }
    if(accept(Symbols::semicolon)) {
        scanner.getNextLexem();
    }
    expect(Symbols::sys_until);
    scanner.getNextLexem();
    ExpressionNode *expr = condition(ifFunc, true);
    stackBreak.pop();

    if(expr->convertType->type != DescriptorTypes::scalarBoolean) {
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }
    rt->children.push_back(expr);
    return rt;
}

TreeNode *Parser::statementFor(bool ifFunc) {
    bool flag = false;
	ForActionNode *fr = new ForActionNode(scanner.getLexem().lexem);
	scanner.getNextLexem();

	expect(Symbols::ident);
	ExpressionNode *ident = getIdent(ifFunc, nullptr, false, true);

	if(ident->convertType->type != DescriptorTypes::scalarInt) {
		//ERROR must be int
		error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
	}
	if(ident->nameLex == "nil" || ident->nameLex == "true" || ident->nameLex == "false") {
        error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
	}
	if(ident->nameLex == "^" || ident->nameLex == "@") {

	} else if(((VarNode*)ident)->type->_class != SymbolTypes::vars) {
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expect var.");
    }
	expect(Symbols::assign);

	if(scanner.getPrevLexem().val == (int)Symbols::rpar) {
		error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
	} else {
		scanner.getNextLexem();
	}
	BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
	scanner.getNextLexem();
	ExpressionNode *expr0 = expression(ifFunc, true);
	if(expr0->convertType->type != DescriptorTypes::scalarInt) {
		//ERROR must be int
		error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
	}
	eq->children.push_back(ident);
	eq->children.push_back(expr0);
	eq->convertType = expr0->convertType;

	if(accept(Symbols::sys_to) || accept(Symbols::sys_downto)) {
		if(accept(Symbols::sys_to)) {
			fr->nameNode = "to";
		} else {
			fr->nameNode = "downto";
		}
		scanner.getNextLexem();
	} else {
		error.printError(Errors::ERROR_TO_DOWNTO, scanner.getLexem().pos, "");
	}
	ExpressionNode *expr1 = expression(ifFunc, true);

	if(expr1->convertType->type != DescriptorTypes::scalarInt) {
		//ERROR must be int
		error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
	}
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;
	/*if(fr->nameNode == "to") {
    } else {
    }*/

	expect(Symbols::sys_do);
	scanner.getNextLexem();
	fr->children.push_back(eq);
	fr->children.push_back(expr1);
	if(accept(Symbols::semicolon)) {
		fr->children.push_back(new FreeActionNode("FREE_NODE"));
	} else {
		fr->children.push_back(statement(ifFunc));
	}
    stackBreak.pop();
	return fr;
}

TreeNode *Parser::statementSleep(bool ifFunc) {
    ActionNode *slp = new ActionNode("SLEEP");
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();
    TreeNode *val;
    val = condition(ifFunc, true);
    if(((ExpressionNode*)val)->convertType->type != DescriptorTypes::scalarInt) {
        //ERROR
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }
    expect(Symbols::rpar);
    scanner.getNextLexem();
    slp->children.push_back(val);
    return slp;
}

TreeNode *Parser::statementInitscr(bool ifFunc) {
    ActionNode *iscr = new ActionNode("INITSCR");
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();
    expect(Symbols::rpar);
    scanner.getNextLexem();
    return iscr;
}

TreeNode *Parser::statementGotoxy(bool ifFunc) {
    ActionNode *gtxy = new ActionNode("GOTOXY");
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();

    TreeNode *val1, *val2;
    val1 = condition(ifFunc, true);
    if(((ExpressionNode*)val1)->convertType->type != DescriptorTypes::scalarInt) {
        //ERROR
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }
    expect(Symbols::comma);
    scanner.getNextLexem();
    val2 = condition(ifFunc, true);
    if(((ExpressionNode*)val2)->convertType->type != DescriptorTypes::scalarInt) {
        //ERROR
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }
    expect(Symbols::rpar);
    scanner.getNextLexem();
    gtxy->children.push_back(val1);
    gtxy->children.push_back(val2);
    return gtxy;
}

TreeNode *Parser::statementWrite(bool ifFunc) {
    ActionNode *wr = new ActionNode("WRITE");
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();
    TreeNode *ident;
    while(!accept(Symbols::rpar)) {
        if(accept(Symbols::comma)) {
            scanner.getNextLexem();
            continue;
        }
        if(scanner.getLexem().val == (int)Symbols::sys_string) {
            ident = new StringNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
            wr->children.push_back(ident);
            scanner.getNextLexem();
            continue;
        }
        if(!accept(Symbols::sys_string)) {
            ident = condition(ifFunc, true);
            AsmOperand* shift = nullptr;
            if( ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarChar &&
                ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarInt &&
                ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarFloat &&
                ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarBoolean &&
                ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::pointer) {
                //ERROR
                error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
            } else {
                wr->children.push_back(ident);
            }
        } else {
            error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
        }
    }
    expect(Symbols::rpar);
    scanner.getNextLexem();
    return wr;
}

TreeNode *Parser::statementRead(bool ifFunc) {
    ActionNode *rd = new ActionNode("READ");
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();
    ExpressionNode *ident;
    int i = 0;
    std::string format = "";
    while(!accept(Symbols::rpar)) {
        if(i != 0) {
            expect(Symbols::comma);
            scanner.getNextLexem();
        }
        expect(Symbols::ident);
        ident = factor(ifFunc, true, true);
        if(ident->id != (int)Symbols::ident && ident->id != (int)Symbols::at && ident->id != (int)Symbols::arrow) {
            error.printError(Errors::ERROR_BAD_SYMBOL, scanner.getLexem().pos, "");
        }
        if(ident->convertType->type != DescriptorTypes::scalarChar &&
            ident->convertType->type != DescriptorTypes::scalarInt &&
            ident->convertType->type != DescriptorTypes::scalarFloat &&
            ident->convertType->type != DescriptorTypes::scalarBoolean) {
            //ERROR
            error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
        } else {
            rd->children.push_back(ident);
            ++i;
        }
    }
    expect(Symbols::rpar);
    scanner.getNextLexem();
    return rd;
}


TreeNode *Parser::statement(bool ifFunc) {
	if(scanner.isLastLexem()) {
		return nullptr;
	}
	if (accept(Symbols::ident) || accept(Symbols::at)) {
		std::string lex = scanner.getLexem().lexem;
		std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
		if(lex == "sleep") {
            return statementSleep(ifFunc);
		} else if(lex == "initscr") {
            return statementInitscr(ifFunc);
		} else if(lex == "gotoxy") {
            return statementGotoxy(ifFunc);
		} else if(lex == "write") {
			return statementWrite(ifFunc);
		} else if(lex == "read") {
			return statementRead(ifFunc);
		} else if(lex == "ord") {
			return getOrd(ifFunc, true);
		} else if(lex == "chr") {
			return getChr(ifFunc, true);
		}
		ExpressionNode *ident = getIdent(ifFunc, nullptr, false, true);

		if(ident->nameLex == "nil" || ident->nameLex == "true" || ident->nameLex == "false") {
            error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
		}

		if(accept(Symbols::semicolon)) {
			if(scanner.getPrevLexem().val == (int)Symbols::rpar) {
				scanner.getNextLexem();
				return ident;
			} else {
				error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
			}
		}
		if(ident->nameLex == "^" || ident->nameLex == "@") {

		} else if(((VarNode*)ident)->type->_class != SymbolTypes::vars) {
            error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expect var.");
        }

        if((ident->id == (int)Symbols::ident && ((VarNode*)ident)->type->_class != SymbolTypes::proc) || ident->id != (int)Symbols::ident) {
		expect(Symbols::assign);

		if(scanner.getPrevLexem().val == (int)Symbols::rpar) {
			error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
		} else {
			scanner.getNextLexem();
		}

		BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();		ExpressionNode *expr = condition(ifFunc, true);//expression
		eq->children.push_back(ident);
		eq->children.push_back(expr);
		eq->convertType = castSecontToFirst(((ExpressionNode*)ident)->convertType, expr->convertType);
		if(eq->convertType == nullptr) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if(((ExpressionNode*)ident)->convertType->type != eq->convertType->type) {
            //ERROR in types
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if( ident->nameLex == "@") {//
            ident = ((ExpressionNode*)ident->children[0]);
		}
        return eq;
        } else {
            return ident;
        }
	} else if (accept(Symbols::sys_begin)) {
		BeginActionNode *ident = new BeginActionNode(scanner.getLexem().lexem);

		scanner.getNextLexem();
		if(accept(Symbols::semicolon)) {
			ident->children.push_back(new FreeActionNode("FREE_NODE"));
		} else {
			scanner.getPrevLexem();
		}

		do {
			scanner.getNextLexem();
			if(accept(Symbols::sys_end))
				break;
			ident->children.push_back(statement(ifFunc));

		} while (accept(Symbols::semicolon));

		expect(Symbols::sys_end);
		scanner.getNextLexem();
		return ident;
	} else if (accept(Symbols::sys_if)) {
		return statementIf(ifFunc);
	} else if (accept(Symbols::sys_while)) {
		return statementWhile(ifFunc);
	} else if (accept(Symbols::sys_for)) {
		return statementFor(ifFunc);
	} else if (accept(Symbols::sys_repeat)) {
        return statementRepeat(ifFunc);
	} else if(accept(Symbols::sys_return)) {
		ActionNode *ret = new ActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		return ret;
	} else if(accept(Symbols::sys_break)) {
		if(stackBreak.size() == 0) {
			error.printError(Errors::ERROR_BREAK, scanner.getLexem().pos, "");
		}
		BreakActionNode *br = new BreakActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		return br;
	} else if(accept(Symbols::sys_continue)) {
		if(stackBreak.size() == 0) {
			error.printError(Errors::ERROR_BREAK, scanner.getLexem().pos, "");
		}
		ContinueActionNode *cn = new ContinueActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		return cn;
	} else {
		error.printError(Errors::ERROR_SYNTAX_ERROR, scanner.getLexem().pos, "");
	}
	return nullptr;
}

bool Parser::accept(Symbols symacc) {
	int symbol = scanner.getLexem().val;
	if (symbol == (int)symacc) {
		return true;
	}
	return false;
}

bool Parser::expect(Symbols symacc) {
	if (accept(symacc))
		return true;
	error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
	return false;
}

Parser::~Parser() {
	 delete root;
}



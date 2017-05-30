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

Parser::Parser(std::string _filePath) {
	scanner.open(_filePath);
	scanner.read();
	//scanner.printLexems();
}

void Parser::Parse() {
	program();
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

    ASMOther.Add("%include \"io64.inc\"");

	expect(Symbols::sys_program);
	scanner.getNextLexem();

	if(expect(Symbols::ident)) {
		root = new ProgramNode(scanner.getLexem());
		scanner.getNextLexem();
	}
	expect(Symbols::semicolon);
	scanner.getNextLexem();

	SymbolTable *stb = new SymbolTable();
	TreeNode *blockNode;
	blockNode = block(false, stb);

	root->type->localParam = stb;
	root->children.push_back(blockNode);

	expect(Symbols::point);
}

void Parser::genAsmCodeForIdentFunc(ExpressionNode *ident, ExpressionNode *expr, AsmOperand* shift) {
    int mov = 0;
    if(ident->convertType->type == DescriptorTypes::scalarFloat ||
        (ident->convertType->type == DescriptorTypes::pointer &&
        ((DescriptorPointers*)ident->convertType)->baseType->type == DescriptorTypes::scalarFloat )) {
        if(expr->convertType->type == DescriptorTypes::scalarFloat) {
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                //-
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift,"-"+std::to_string((mov)*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift,"+"+std::to_string((mov+1)*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
            }
        } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                //-
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift,"-"+std::to_string((mov)*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift,"+"+std::to_string((mov+1)*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
            }
            ASM.Add(AsmOperation::asm_cvtsi2sd, xmm0, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift,"+"+std::to_string((mov+1)*8));
            //ASM.Add(AsmOperation::asm_cvtsi2sd, xmm0, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            //asmConstants.Add("8");
            ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("8"));
            ASM.Add(AsmOperation::asm_movq, rsp, xmm0, AsmSizeof::s_dq, true);
            ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift, "+"+std::to_string((mov+1)*8));
            //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
        }
    } else {
        SymbolVar *svar = nullptr;
        if( (svar = (SymbolVar*)(symbolStack->get()->findInTable(ident->nameLex))) != nullptr) {
            mov = symbolStack->get()->movInTable(ident->nameLex);
            //-
            if(svar->mot == MethodOfTransmission::paramvar) {
                mov = 1;
                //mov rax, qword [rbp+8+8]
                //pop qword [rax]
                ASM.Add(AsmOperation::asm_mov, rax, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, shift, "-"+std::to_string(mov*8));
                //ASM.Add(AsmOperation::asm_mov, rax, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
                ASM.Add(AsmOperation::asm_pop, rax, AsmSizeof::s_dq, true, shift);
            } else {
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift, "-"+std::to_string(mov*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            }
        } else if((svar = (SymbolVar*)(symbolStack->get()->parent->findInTable(ident->nameLex))) != nullptr) {
            mov = symbolStack->get()->parent->movInTable(ident->nameLex);
            if(svar->mot == MethodOfTransmission::paramvar) {
                //mov rax, qword [rbp+8+8]
                //pop qword [rax]
                mov = 1;
                //nullptr
                ASM.Add(AsmOperation::asm_mov, rax, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
                //ASM.Add(AsmOperation::asm_mov, rax, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true));
                ASM.Add(AsmOperation::asm_pop, rax, AsmSizeof::s_dq, true, shift);
            } else {
                ASM.Add(AsmOperation::asm_pop, rbp, AsmSizeof::s_dq, true, shift,"+"+std::to_string((mov+1)*8));
                //ASM.Add(AsmOperation::asm_pop, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift));
            }
        } else if((svar = (SymbolVar*)(baseTable->findInTable(ident->nameLex))) != nullptr) {
            if(svar->mot == MethodOfTransmission::paramvar) {
                //mov rax, qword [rbp+8+8]
                //pop qword [rax]
                mov = 1;
                ASM.Add(AsmOperation::asm_mov, rax, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true));
                ASM.Add(AsmOperation::asm_pop, rax, AsmSizeof::s_dq, true, shift);
            } else {
                ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
            }
        }
    }
}

void Parser::genAsmCodeForIdent(ExpressionNode *ident, ExpressionNode *expr, AsmOperand* shift) {
    if(ident->nameLex == "^") {
        ASM.Add(AsmOperation::asm_mov, rax, new AsmInd("v_"+((ExpressionNode*)(ident->children[0]))->nameLex, AsmSizeof::s_dq, true, shift));
        ASM.Add(AsmOperation::asm_pop, rax, AsmSizeof::s_dq, true);

    } else
    if(ident->convertType->type == DescriptorTypes::pointer) {
        ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
    } else if(ident->convertType->type == DescriptorTypes::scalarBoolean ||
        (ident->convertType->type == DescriptorTypes::pointer &&
        ((DescriptorPointers*)ident->convertType)->baseType->type == DescriptorTypes::scalarBoolean )) {
        ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
    } else if(ident->convertType->type == DescriptorTypes::scalarChar ||
        (ident->convertType->type == DescriptorTypes::pointer &&
        ((DescriptorPointers*)ident->convertType)->baseType->type == DescriptorTypes::scalarChar )) {
        ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
    } else if(ident->convertType->type == DescriptorTypes::scalarInt ||
        (ident->convertType->type == DescriptorTypes::pointer &&
        ((DescriptorPointers*)ident->convertType)->baseType->type == DescriptorTypes::scalarInt )) {
        ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
    } else if(ident->convertType->type == DescriptorTypes::scalarFloat ||
        (ident->convertType->type == DescriptorTypes::pointer &&
        ((DescriptorPointers*)ident->convertType)->baseType->type == DescriptorTypes::scalarFloat )) {
        if(expr->convertType->type == DescriptorTypes::scalarFloat) {
            ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
        } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
            ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
            ASM.Add(AsmOperation::asm_cvtsi2sd, xmm0, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
            //asmConstants.Add("8");
            //ASM.Add(AsmOperation::asm_sub, rsp, new AsmInd("_8", AsmSizeof::s_dq, true));
            ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("8"));
            ASM.Add(AsmOperation::asm_movq, rsp, xmm0, AsmSizeof::s_dq, true);
            ASM.Add(AsmOperation::asm_pop, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift));
        }
    }
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

    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;
    ASM.Add(AsmOperation::asm_pop, rax);
    //asmConstants.Add("1");
    //ASM.Add(AsmOperation::asm_cmp, rax, new AsmInd("_1", AsmSizeof::s_dq, true));
    ASM.Add(AsmOperation::asm_cmp, rax, new AsmImn("1"));
    ASM.Add(AsmOperation::asm_jnz, new AsmMark("IFELSE"+std::to_string(_boolOperCounter), false));

    //scanner.getNextLexem();
    if(accept(Symbols::semicolon)) {
        ifbody->children.push_back(new FreeActionNode("FREE_NODE"));
        //scanner.getNextLexem();
    } else {
        ifbody->children.push_back(statement(ifFunc));
    }
    ASM.Add(AsmOperation::asm_jmp, new AsmMark("_IFELSE"+std::to_string(_boolOperCounter), false));
    ASM.Add("IFELSE"+std::to_string(_boolOperCounter)+":");

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
            //scanner.getNextLexem();
        }
    } else {
        scanner.getPrevLexem();
    }
    ASM.Add("_IFELSE"+std::to_string(_boolOperCounter)+":");
    return br;
}

TreeNode *Parser::statementWhile(bool ifFunc) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;

    ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("WHILE"+std::to_string(_boolOperCounter)+":");

    WhileActionNode *wh = new WhileActionNode(scanner.getLexem().lexem);
    scanner.getNextLexem();


    ExpressionNode *expr = condition(ifFunc, true);
    ASM.Add(AsmOperation::asm_pop, r12);
    //asmConstants.Add("1");
    //ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("_1", AsmSizeof::s_dq, true));
    ASM.Add(AsmOperation::asm_cmp, r12, new AsmImn("1"));
    ASM.Add(AsmOperation::asm_jnz, new AsmMark("_WHILE"+std::to_string(_boolOperCounter), false));

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

    ASM.Add(AsmOperation::asm_jmp, new AsmMark("WHILE"+std::to_string(_boolOperCounter), false));
    ASM.Add("_WHILE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
    stackBreak.pop();
    return wh;
}

TreeNode *Parser::statementUntil(bool ifFunc) {
    stackBreak.push(boolOperCounter);
    int _boolOperCounter = boolOperCounter;
    boolOperCounter++;

    ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
    ASM.Add("UNTIL"+std::to_string(_boolOperCounter)+":");

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

    ASM.Add(AsmOperation::asm_pop, rax);
    //asmConstants.Add("0");
    //ASM.Add(AsmOperation::asm_cmp, rax, new AsmInd("_0", AsmSizeof::s_dq, true));
    ASM.Add(AsmOperation::asm_cmp, rax, new AsmImn("0"));
    ASM.Add(AsmOperation::asm_jz, new AsmMark("UNTIL"+std::to_string(_boolOperCounter), false));
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
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

    AsmOperand* shift = nullptr;
    if(ident->nameLex == "^" || ident->nameLex == "@") {//need before expression
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
       ///////////
	BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
	scanner.getNextLexem();
	ExpressionNode *expr0 = expression(ifFunc, true);
	if(expr0->convertType->type != DescriptorTypes::scalarInt) {
		//ERROR must be int
		error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
	}

	if(!ifFunc)
        genAsmCodeForIdent(ident, expr0, shift);
	else
        genAsmCodeForIdentFunc(ident, expr0, nullptr);
	////////////////
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
	//continue CONTINUE
	//break BREAK
	ASM.Add(AsmOperation::asm_jmp, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
	ASM.Add("CONTINUE"+std::to_string(_boolOperCounter)+":");
	asmConstants.Add("1");
    ASM.Add(AsmOperation::asm_push, new AsmInd("_1", AsmSizeof::s_dq, true));
    ASM.Add(AsmOperation::asm_pop, rbx);
	if(fr->nameNode == "to") {
        if(ifFunc) {
                int mov = 0;
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_add, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift), rbx);
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_add, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true, shift), rbx);
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_add, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true, shift), rbx);
            }
        } else {
            ASM.Add(AsmOperation::asm_add, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true), rbx);
        }
    } else {
        if(ifFunc) {
            int mov = 0;
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_sub, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_sub, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_sub, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true), rbx);
            }
        } else {
            ASM.Add(AsmOperation::asm_sub, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true), rbx);
        }
    }
    ASM.Add(AsmOperation::asm_pop, r12);

    if(ifFunc) {
        int mov = 0;
        if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
            mov = symbolStack->get()->movInTable(ident->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr,"-"+std::to_string(mov*8));
            //ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true));
        } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
            mov = symbolStack->get()->parent->movInTable(ident->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
            //ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true));
        } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
            ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true));
        }
    } else {
        ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true));
    }
    ASM.Add(AsmOperation::asm_push, r12);
    //ASM.Add(AsmOperation::asm_jz, new AsmMark("BREAK"+std::to_string(_boolOperCounter), false));
    if(fr->nameNode == "to") {
        ASM.Add(AsmOperation::asm_jle, new AsmMark("BREAK"+std::to_string(_boolOperCounter), false));
    } else {
        ASM.Add(AsmOperation::asm_jge, new AsmMark("BREAK"+std::to_string(_boolOperCounter), false));
    }
	ASM.Add("FOR"+std::to_string(_boolOperCounter)+":");

	expect(Symbols::sys_do);
	scanner.getNextLexem();
	fr->children.push_back(eq);
	fr->children.push_back(expr1);
	if(accept(Symbols::semicolon)) {
		fr->children.push_back(new FreeActionNode("FREE_NODE"));
	} else {
		fr->children.push_back(statement(ifFunc));
	}

	asmConstants.Add("1");
    ASM.Add(AsmOperation::asm_push, new AsmInd("_1", AsmSizeof::s_dq, true));
    ASM.Add(AsmOperation::asm_pop, rbx);
	if(fr->nameNode == "to") {
        if(ifFunc) {
                int mov = 0;
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_add, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_add, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_add, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_add, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true), rbx);
            }
        } else {
            ASM.Add(AsmOperation::asm_add, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true), rbx);
        }
    } else {
        if(ifFunc) {
            int mov = 0;
            if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr,"-"+std::to_string(mov*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_sub, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
                mov = symbolStack->get()->parent->movInTable(ident->nameLex);
                ASM.Add(AsmOperation::asm_sub, rbp, rbx, AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8), AsmSizeof::s_def, false, nullptr, "");
                //ASM.Add(AsmOperation::asm_sub, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true), rbx);
            } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
                ASM.Add(AsmOperation::asm_sub, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true), rbx);
            }
        } else {
            ASM.Add(AsmOperation::asm_sub, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true), rbx);
        }
    }
    ASM.Add(AsmOperation::asm_pop, r12);
    if(ifFunc) {
        int mov = 0;
        if(symbolStack->get()->findInTable(ident->nameLex) != nullptr) {
            mov = symbolStack->get()->movInTable(ident->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "-"+std::to_string(mov*8));
            //ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("rbp-"+std::to_string(mov*8), AsmSizeof::s_dq, true));
        } else if(symbolStack->get()->parent->findInTable(ident->nameLex) != nullptr) {
            mov = symbolStack->get()->parent->movInTable(ident->nameLex);
            ASM.Add(AsmOperation::asm_cmp, r12, rbp, AsmSizeof::s_def, false, nullptr, "", AsmSizeof::s_dq, true, nullptr, "+"+std::to_string((mov+1)*8));
            //ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("rbp+8+"+std::to_string(mov*8), AsmSizeof::s_dq, true));
        } else if(baseTable->findInTable(ident->nameLex) != nullptr) {
            ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_"+ident->nameLex, AsmSizeof::s_dq, true));
        }
    } else {
        ASM.Add(AsmOperation::asm_cmp, r12, new AsmInd("v_" + ident->nameLex, AsmSizeof::s_dq, true));
    }
    ASM.Add(AsmOperation::asm_push, r12);

    if(fr->nameNode == "to") {
        ASM.Add(AsmOperation::asm_jge, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
    } else {
        ASM.Add(AsmOperation::asm_jle, new AsmMark("FOR"+std::to_string(_boolOperCounter), false));
    }
    ASM.Add("BREAK"+std::to_string(_boolOperCounter)+":");
    ASM.Add(AsmOperation::asm_pop, r12);
    stackBreak.pop();
	return fr;
}

TreeNode *Parser::statement(bool ifFunc) {
	if(scanner.isLastLexem()) {
		return nullptr;
	}
	if (accept(Symbols::ident) || accept(Symbols::at)) {
		std::string lex = scanner.getLexem().lexem;
		std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
		if(lex == "write") {
			ActionNode *wr = new ActionNode("WRITE");
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;

			std::string format = "";
			while(!accept(Symbols::rpar)) {
                if(accept(Symbols::comma)) {
                    scanner.getNextLexem();
                    continue;
                }
                if(scanner.getLexem().val == (int)Symbols::sys_string) {
                    ident = new StringNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
                    format += scanner.getLexem().lexem;
                    wr->children.push_back(ident);
                    scanner.getNextLexem();
                    if(accept(Symbols::rpar)) {
                        ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize()), AsmSizeof::s_def, false));
                        asmFormat.Add(format);
                        ASM.Add(AsmOperation::asm_sub, rsp, new AsmImn("40"));
                        ASM.Add(AsmOperation::asm_call, new AsmInd("printf", AsmSizeof::s_def, false));
                        ASM.Add(AsmOperation::asm_add, rsp, new AsmImn("40"));
                    }
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
                        ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize()), AsmSizeof::s_def, false));
                        if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarInt) {
                            format += "%d";
                        } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarFloat) {
                            format += "%f";
                        } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarBoolean) {
                            format += "%d";
                        } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarChar) {
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
                        wr->children.push_back(ident);
                    }
                } else {
                    error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
                }
			}
			expect(Symbols::rpar);
			scanner.getNextLexem();
			return wr;
		} else if(lex == "read") {
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
				//ident = getIdent(nullptr, false, true);
				ident = factor(ifFunc, true, true);
				if(ident->id != (int)Symbols::ident && ident->id != (int)Symbols::at && ident->id != (int)Symbols::arrow) {
                    error.printError(Errors::ERROR_BAD_SYMBOL, scanner.getLexem().pos, "");
				}
				AsmOperand* shift = nullptr;
                if(ident->nameLex == "^" || ident->nameLex == "@") {//need before expression
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
				if(ident->convertType->type != DescriptorTypes::scalarChar &&
                    ident->convertType->type != DescriptorTypes::scalarInt &&
					ident->convertType->type != DescriptorTypes::scalarFloat &&
					ident->convertType->type != DescriptorTypes::scalarBoolean) {
					//ERROR
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
				} else {
                    ASM.Add(AsmOperation::asm_mov, rcx, new AsmInd("format"+std::to_string(asmFormat.getSize()), AsmSizeof::s_def, false));
                    if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarInt) {
                        format += "%d";
                    } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarFloat) {
                        format += "%lf";
                    } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarBoolean) {
                        format += "%d";
                    } else if(((ExpressionNode*)ident)->convertType->type == DescriptorTypes::scalarChar) {
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
                    rd->children.push_back(ident);
                    ++i;
                }
			}
			expect(Symbols::rpar);
			scanner.getNextLexem();
			return rd;
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
				if((ident->nameLex != "^" && ident->nameLex != "@") && ((VarNode*)ident)->type->_class == SymbolTypes::func) {
                    ASM.Add(AsmOperation::asm_pop, rax);
				} else if((ident->nameLex == "^" || ident->nameLex == "@") && ((VarNode*)(ident->children[0]))->type->_class == SymbolTypes::func) {
                    ASM.Add(AsmOperation::asm_pop, rax);
				}
				//scanner.getNextLexem();
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

        AsmOperand* shift = nullptr;
        if(ident->nameLex == "^" || ident->nameLex == "@") {//need before expression
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

		BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();		ExpressionNode *expr = condition(ifFunc, true);//expression
		eq->children.push_back(ident);
		eq->children.push_back(expr);
		eq->convertType = castSecontToFirst(((ExpressionNode*)ident)->convertType, expr->convertType);
		if(eq->convertType == nullptr) {
			//ERROR
        //std::cout << ident->nameLex << " !!!!!\n" << (int)(expr->convertType->type) <<"\n" <<
         //   (int)(((ExpressionNode*)ident)->convertType);
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if(((ExpressionNode*)ident)->convertType->type != eq->convertType->type) {
            //ERROR in types
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if( ident->nameLex == "@") {//
            ident = ((ExpressionNode*)ident->children[0]);
		}
		if(!ifFunc)
            genAsmCodeForIdent(ident, expr, shift);
		else
            genAsmCodeForIdentFunc(ident, expr, shift);
        return eq;
        } else
            return ident;

	} else if (accept(Symbols::sys_begin)) {
		BlockActionNode *ident = new BlockActionNode(scanner.getLexem().lexem);

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
		//std::cout << "!!!  " <<scanner.getLexem().lexem;
		return ident;
	} else if (accept(Symbols::sys_if)) {
		return statementIf(ifFunc);
	} else if (accept(Symbols::sys_while)) {
		return statementWhile(ifFunc);
	} else if (accept(Symbols::sys_for)) {
		return statementFor(ifFunc);
	} else if (accept(Symbols::sys_repeat)) {
        return statementUntil(ifFunc);
	} else if(accept(Symbols::sys_break)) {
		if(stackBreak.size() == 0) {
			error.printError(Errors::ERROR_BREAK, scanner.getLexem().pos, "");
		}
		BreakActionNode *br = new BreakActionNode(scanner.getLexem().lexem);
		ASM.Add(AsmOperation::asm_jmp, new AsmMark("BREAK"+std::to_string(stackBreak.top()), false));
		scanner.getNextLexem();
		return br;
	} else if(accept(Symbols::sys_continue)) {
		if(stackBreak.size() == 0) {
			error.printError(Errors::ERROR_BREAK, scanner.getLexem().pos, "");
		}
		ContinueActionNode *cn = new ContinueActionNode(scanner.getLexem().lexem);
		ASM.Add(AsmOperation::asm_jmp, new AsmMark("CONTINUE"+std::to_string(stackBreak.top()), false));
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

void Parser::print() {
	baseTable->print(0);
	printTree(root, 0, 0);
}

Parser::~Parser() {
	 delete root;
}



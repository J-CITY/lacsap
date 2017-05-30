#include "parser.h"

std::string SymbolTypeToStr(int st) {
	switch(st) {
	case (int)SymbolTypes::types:
	{
		return "types";
	}
	case (int)SymbolTypes::consts:
	{
		return "consts";
	}
	case (int)SymbolTypes::vars:
	{
		return "vars";
	}
	case (int)SymbolTypes::func:
	{
		return "func";
	}
	case (int)SymbolTypes::proc:
	{
		return "proc";
	}
	}
	return "";
}

void printTree(TreeNode *root, int l, int rot) {
	bool if_print = false;
	if(root->children.size() == 0) {
		if(l != 0) {
			for(int j = 0; j < l-1; ++j) {
				std::cout << "     ";
			}
			if(rot == 0) {
				//std::cout << "\\____";
				std::cout << "\xc0\xc4\xc4\xc4\xc4";
			} else {
				std::cout << "\xda\xc4\xc4\xc4\xc4";
				//std::cout << "/----";
			}
		}
		root->print(l);
		std::cout << std::endl;
	}

	for (unsigned int k = 0; k < root->children.size(); ++ k) {
		if(k < root->children.size()/2 && root->children[k] != nullptr){
			printTree(root->children[k], l+1, 1);
			std::cout << std::endl;
		}
		if(k >= root->children.size()/2 && !if_print) {
			if(l != 0) {
				for(int j = 0; j < l-1; ++j) {
					std::cout << "     ";
				}
				if(rot == 0) {
					//std::cout << "\\____";
					std::cout << "\xc0\xc4\xc4\xc4\xc4";
				} else {
					std::cout << "\xda\xc4\xc4\xc4\xc4";
					//std::cout << "/----";
				}
			}
			root->print(l);
			std::cout << std::endl;
			if_print = true;
		}
		if(k >= root->children.size()/2  && root->children[k] != nullptr) {
			printTree(root->children[k], l+1, 0);
			std::cout << std::endl;
		}
	}
}

Parser::Parser(std::string _filePath) {
	scanner.open(_filePath);
	scanner.read();
	//scanner.printLexems();
}

void Parser::Parse() {
	program();
}

SymbolTable *baseTable = nullptr;
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

TreeNode *Parser::block(bool ifFunc, SymbolTable *st) {
	TreeNode *blockNode = new BlockActionNode("BLOCK");
	symbolStack->push(st);

	while(1){
	if (accept(Symbols::sys_const)) {
		scanner.getNextLexem();
		if(accept(Symbols::ident)) {
			do {
				expect(Symbols::ident);
				Lexem identLex = scanner.getLexem();

				Symbol *identSymbol = nullptr;
				identSymbol = symbolStack->symbolTable->findInTable(identLex.lexem);
				if(identSymbol != nullptr) {
					//ERROR alredy exist
					error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
				}

				scanner.getNextLexem();
				expect(Symbols::equal);
				scanner.getNextLexem();

				Descriptor *type = nullptr;

				Descriptor *d = nullptr;
				TreeNode *expr = condition(&d);
				((ExpressionNode*)expr)->convertType = d;


				if(accept(Symbols::colon)) {
					scanner.getNextLexem();
					expect(Symbols::ident);
					Lexem typeLex = scanner.getLexem();
					Symbol *typeSymbol = symbolStack->findInTables(typeLex.lexem);
					if(typeSymbol == nullptr) {
						//ERROR it is not a type
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
					}
					if(typeSymbol->_class == SymbolTypes::types) {
						type = typeSymbol->type;
					} else {
						//ERROR it is not a type
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
					}
					scanner.getNextLexem();
				} else {
					type = d;
				}
				SymbolConst *ident = new SymbolConst(identLex, expr, type);
				st->add(ident);//add new constant
				expect(Symbols::semicolon);
				scanner.getNextLexem();
			} while (accept(Symbols::ident));
		}
		continue;
	}
	if (accept(Symbols::sys_var)) {
		//bool ifNeedNext = true;
		scanner.getNextLexem();
		do {
			std::vector<SymbolVar*> r = getVarBlock(st, false);
			for(unsigned int i = 0; i < r.size(); ++i) {
				if(st->findInTable(r[i]->lex.lexem) != nullptr) {
					error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
				}
				st->add(r[i]);
			}
			//ifNeedNext = false;
		} while(accept(Symbols::ident));
		continue;
	}
	if (accept(Symbols::sys_type)) {
		Descriptor *type;
		SymbolType *symbolType = nullptr;
		//blockNode->children.push_back(new TypeActionNode(scanner.getLexem().lexem));
		scanner.getNextLexem();
		do {
			expect(Symbols::ident);
			symbolType = new SymbolType();
			symbolType->lex = scanner.getLexem();
			scanner.getNextLexem();
			expect(Symbols::equal);
			scanner.getNextLexem();

			DescriptorArray *arr = nullptr;
			if(accept(Symbols::sys_array)) {
				 arr = new DescriptorArray();
				arr = new DescriptorArray();
				st->descriptors.push_back(arr);
				scanner.getNextLexem();

				expect(Symbols::lbracket);
				scanner.getNextLexem();
				Descriptor *d1 = nullptr, *d2 = nullptr;
				ExpressionNode *from = expression(&d1);
				from->convertType = d1;
				expect(Symbols::twopoints);
				scanner.getNextLexem();
				ExpressionNode *to = expression(&d2);
				to->convertType = d2;
				if(d1->type != DescriptorTypes::scalarInt ||
					d2->type != DescriptorTypes::scalarInt) {
					//ERROR
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
				}

				DescriptorLimited *lim = new DescriptorLimited();
				lim->_min = from;
				lim->_max = to;
				lim->baseType = d1;

				st->descriptors.push_back(lim);
				arr->indices.push_back(lim);
				while(accept(Symbols::comma)) {
					scanner.getNextLexem();
					from = expression(&d1);
					from->convertType = d1;
					expect(Symbols::twopoints);
					scanner.getNextLexem();
					to = expression(&d2);
					to->convertType = d2;
					lim = new DescriptorLimited();
					lim->_min = from;
					lim->_max = to;
					lim->baseType = d1;
					if(d1->type != DescriptorTypes::scalarInt ||
						d2->type != DescriptorTypes::scalarInt) {
						//ERROR
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
					}
					st->descriptors.push_back(lim);
					arr->indices.push_back(lim);
				}
				expect(Symbols::rbracket);
				scanner.getNextLexem();
				expect(Symbols::sys_of);
				scanner.getNextLexem();
			}
			 //TreeNode *type = nullptr;
			 if(accept(Symbols::ident) || accept(Symbols::sys_record)) {
				 if(accept(Symbols::ident)) {
					Symbol *typeSymbol = symbolStack->findInTables(scanner.getLexem().lexem);
					if(typeSymbol == nullptr) {
						//ERROR it is not a type
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
					}
					if(typeSymbol->_class == SymbolTypes::types) {
						type = typeSymbol->type;
					} else {
						//ERROR it is not a type
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
					}
				}
				else {
					//type = new RecordLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
				}
			} else {
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			DescriptorRecord *structRecord = nullptr;
			if(scanner.getLexem().val == (int)Symbols::sys_record) {
				structRecord = new DescriptorRecord();
				st->descriptors.push_back(structRecord);
				//structRecord = new ActionNode("STRUCT");
				//bool ifNeedNextRecord = true;
				scanner.getNextLexem();
				do {
					std::vector<SymbolVar*> r = getVarBlock(st, false);
					for(unsigned int i = 0; i < r.size(); ++i) {
						structRecord->rName.push_back(r[i]);
					}
					//ifNeedNextRecord = false;
				} while(accept(Symbols::ident));
				type = structRecord;

				expect(Symbols::sys_end);
				//scanner.getNextLexem();
			}

			scanner.getNextLexem();
			expect(Symbols::semicolon);
			scanner.getNextLexem();
			if(arr != nullptr) {
				arr->baseType = type;
			} else {}
			if(arr != nullptr) {
				symbolType->type = arr;
			} else {
				symbolType->type = type;
 			}
 			st->add(symbolType);
		 } while (accept(Symbols::ident));
		continue;
	}
	if(accept(Symbols::sys_function)) {
		FuncNode *funcNode;
		SymbolFunc *func = new SymbolFunc();
		func->type = nullptr;
		//st->add(func);
		scanner.getNextLexem();

		if(expect(Symbols::ident)) {
			funcNode = new FuncNode(scanner.getLexem());
			func->lex = scanner.getLexem();
			scanner.getNextLexem();
		}
		st->add(func);
		expect(Symbols::lpar);
        scanner.getNextLexem();
		//func->children.push_back(new ActionNode("IN_PARAM"));
		//bool ifNeedNext = true;
		do {
 			std::vector<SymbolVar*> r = getVarBlock(st,  true);
			for(unsigned int i = 0; i < r.size(); ++i) {
				if(func->inputParam->findInTable(r[i]->lex.lexem) != nullptr) {
					error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
				}
				func->inputParam->add(r[i]);
			}
			if(accept(Symbols::semicolon)) {
				scanner.getNextLexem();
				if(!accept(Symbols::ident) && !accept(Symbols::sys_var)) {
					error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expected identifier.");
				}
			}
			//ifNeedNext = false;
		} while(accept(Symbols::ident) || accept(Symbols::sys_var));
		expect(Symbols::rpar);
		scanner.getNextLexem();
		expect(Symbols::colon);
		scanner.getNextLexem();
		expect(Symbols::ident);
		Symbol *typeSymbol = symbolStack->findInTables(scanner.getLexem().lexem);
		if(typeSymbol == nullptr) {
			//ERROR it is not a type
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
		}
		if(typeSymbol->_class == SymbolTypes::types) {
			func->returnParam = (SymbolType*)typeSymbol;
			func->type = func->returnParam->type;
		} else {
			//ERROR it is not a type
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
		}
		scanner.getNextLexem();
		expect(Symbols::semicolon);
		scanner.getNextLexem();
		symbolStack->push(func->inputParam);

		SymbolTable *st = new SymbolTable();

		SymbolVar *result = new SymbolVar();
		TextPos pos;
		Lexem resultlex("result", pos, (int)Symbols::ident,identifier);
		result->lex = resultlex;
		result->type = typeSymbol->type;
		st->add(result);

		func->block = block(true, st);
		func->localParam = st;
		expect(Symbols::semicolon);
		scanner.getNextLexem();
		funcNode->type = func;
		blockNode->children.push_back(funcNode);
		symbolStack->pop();
		continue;
	 }
	 if(accept(Symbols::sys_procedure)) {
		ProcNode *procNode;
		SymbolProc *proc = new SymbolProc();
		proc->type = nullptr;
		scanner.getNextLexem();

		if(expect(Symbols::ident)) {
			 procNode = new ProcNode(scanner.getLexem());
			 proc->lex = scanner.getLexem();
			 proc->type = nullptr;
			 scanner.getNextLexem();
		}
		st->add(proc);
		expect(Symbols::lpar);
        scanner.getNextLexem();
        //bool ifNeedNext = true;
        do {
            std::vector<SymbolVar*> r = getVarBlock(st, true);
            for(unsigned int i = 0; i < r.size(); ++i) {
                if(proc->inputParam->findInTable(r[i]->lex.lexem) != nullptr) {
                    error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
                }
                proc->inputParam->add(r[i]);
            }
            if(accept(Symbols::semicolon)) {
                scanner.getNextLexem();
                if(!accept(Symbols::ident) && !accept(Symbols::sys_var)) {
                    error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expected identifier.");
                }
            }
            //ifNeedNext = false;
        } while(accept(Symbols::ident) || accept(Symbols::sys_var));
        expect(Symbols::rpar);
        scanner.getNextLexem();
        expect(Symbols::semicolon);
        scanner.getNextLexem();
        symbolStack->push(proc->inputParam);

        SymbolTable *st = new SymbolTable();
        proc->block = block(true, st);
        proc->localParam = st;
        expect(Symbols::semicolon);
        scanner.getNextLexem();
        procNode->type = proc;
        blockNode->children.push_back(procNode);
        symbolStack->pop();
        continue;
    }
    break;
	}
	Descriptor *dres = nullptr;
	blockNode->children.push_back(statement(&dres));
	symbolStack->pop();
	return blockNode;
 }

std::vector<SymbolVar*> Parser::getVarBlock(SymbolTable *st, bool forFunc) {
	bool ifNeedNext = false;
	MethodOfTransmission mot = MethodOfTransmission::paramval;
	Descriptor *type;
	std::vector<Lexem> idents;
	if(forFunc) {
        if(accept(Symbols::sys_var)) {
            mot = MethodOfTransmission::paramvar;
            scanner.getNextLexem();
            //forFunc = false;
        }
    }
	do {
		if(ifNeedNext)
			scanner.getNextLexem();
		ifNeedNext = true;
		//if(forFunc) {
		//	if(accept(Symbols::sys_var)) {
		//		mot = MethodOfTransmission::paramvar;
		//		scanner.getNextLexem();
		//	}
		//	expect(Symbols::ident);
			//if(symbolStack->symbolTable->findInTable(scanner.getLexem().lexem) != nullptr) {
			//	error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
			//}
		//	idents.push_back(scanner.getLexem());
		//} else {
			expect(Symbols::ident);
			//if(symbolStack->symbolTable->findInTable(scanner.getLexem().lexem) != nullptr) {
			//	error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
			//}
			idents.push_back(scanner.getLexem());
		//}
		scanner.getNextLexem();
	} while (accept(Symbols::comma));

	expect(Symbols::colon);
	scanner.getNextLexem();

	DescriptorArray *arr = nullptr;
	if(accept(Symbols::sys_array)) {
		arr = new DescriptorArray();
		st->descriptors.push_back(arr);
		scanner.getNextLexem();

		if(!forFunc && !accept(Symbols::lbracket)) {
			//ERROR must be []
			error.printError(Errors::ERROR_FORGOT_SQUARE_DRACKETS, scanner.getLexem().pos, "");
		}
		if(accept(Symbols::lbracket)) {
			scanner.getNextLexem();
			Descriptor *d1 = nullptr, *d2 = nullptr;
			ExpressionNode *from = expression(&d1);
			from->convertType = d1;
			expect(Symbols::twopoints);
			//ActionNode *range = new ActionNode("range");
			scanner.getNextLexem();
			ExpressionNode *to = expression(&d2);
			to->convertType = d2;
			if(d1->type != DescriptorTypes::scalarInt ||
				d2->type != DescriptorTypes::scalarInt) {
				//ERROR
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
			}

			DescriptorLimited *lim = new DescriptorLimited();
			lim->_min = from;
			lim->_max = to;
			lim->baseType = d1;

			st->descriptors.push_back(lim);
			arr->indices.push_back(lim);

			while(accept(Symbols::comma)) {
				scanner.getNextLexem();
				from = expression(&d1);
				from->convertType = d1;
				expect(Symbols::twopoints);
				//range = new ActionNode("range");
				scanner.getNextLexem();
				to = expression(&d2);
				to->convertType = d2;

				lim = new DescriptorLimited();
				lim->_min = from;
				lim->_max = to;
				if(d1->type != DescriptorTypes::scalarInt ||
					d2->type != DescriptorTypes::scalarInt) {
					//ERROR
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
				}
				lim->baseType = d1;
				st->descriptors.push_back(lim);
				arr->indices.push_back(lim);
			}
			expect(Symbols::rbracket);
			scanner.getNextLexem();
		} else {
			arr->isOpen = true;
		}
		expect(Symbols::sys_of);
		scanner.getNextLexem();
	}
	DescriptorPointers * dp = nullptr;
	if(accept(Symbols::arrow)) {
		dp = new DescriptorPointers();
		scanner.getNextLexem();
	}

	if(accept(Symbols::ident) || accept(Symbols::sys_record)) {
		if(accept(Symbols::ident)) {
			//type = new VarOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
			Symbol *typeSymbol = symbolStack->findInTables(scanner.getLexem().lexem);
			if(typeSymbol == nullptr) {
				//ERROR it is not a type
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
			}
			if(typeSymbol->_class == SymbolTypes::types) {
				type = typeSymbol->type;
			} else {
				//ERROR it is not a type
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
			}
		}
		else {
			//type = new RecordLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		}
	} else {
		error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
	}
	DescriptorRecord *structRecord = nullptr;
	if(scanner.getLexem().val == (int)Symbols::sys_record) {
		structRecord = new DescriptorRecord();
		st->descriptors.push_back(structRecord);
		//bool ifNeedNextRecord = true;
		scanner.getNextLexem();
		do {
			std::vector<SymbolVar*> r = getVarBlock(st, false);
			for(unsigned int i = 0; i < r.size(); ++i) {
				structRecord->rName.push_back(r[i]);
			}
			//ifNeedNextRecord = false;
		} while(accept(Symbols::ident));
		type = structRecord;

		expect(Symbols::sys_end);
		//scanner.getNextLexem();
	}

	scanner.getNextLexem();
	if(!forFunc) {
		expect(Symbols::semicolon);
		scanner.getNextLexem();
	}
	if(dp != nullptr) {
		dp->baseType = type;
		type = dp;
	}
	if(arr != nullptr) {
		arr->baseType = type;
	} else {
		//return type;
	}
	std::vector<SymbolVar*> result;
	for(unsigned int i = 0; i < idents.size(); ++i) {
		result.push_back(new SymbolVar());
		result[result.size()-1]->lex = idents[i];
		if(arr != nullptr) {
			result[result.size()-1]->type = arr;
		} else {
			result[result.size()-1]->type = type;
		}
		result[result.size()-1]->mot = mot;
	}
	return result;
}

ExpressionNode *Parser::factor(Descriptor **resType) {
	ExpressionNode *res;
	if (accept(Symbols::ident) || accept(Symbols::at) || accept(Symbols::sys_false) ||
        accept(Symbols::sys_true) || accept(Symbols::sys_nil)) {
		std::string lex = scanner.getLexem().lexem;
		std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
		if(lex == "ord") {
			VarNode *ord = new VarNode("ORD", scanner.getLexem());
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;
			if(accept(Symbols::charc) || accept(Symbols::ident)) {
				bool b = false;
				if(accept(Symbols::ident))
					b = true;
				if(b) {
					Descriptor *d = nullptr;
					ident = getIdent(&d, false);
					if(d == nullptr || d->type != DescriptorTypes::scalarChar) {
						//ERROR not legal var
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected char.");
					}
				} else {
					ident = new CharLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
					((CharLiteralNode*)ident)->type = baseTable->findInTable((std::string)"char")->type;
					((CharLiteralNode*)ident)->convertType = ((CharLiteralNode*)ident)->type;
					scanner.getNextLexem();
				}
				ord->children.push_back(ident);
			} else {
				error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
			}
			expect(Symbols::rpar);
			scanner.getNextLexem();
			ord->convertType = baseTable->findInTable((std::string)"integer")->type;
			*resType = baseTable->findInTable((std::string)"integer")->type;
			return ord;
		}  else if(lex == "chr") {
			VarNode *chr = new VarNode("CHR", scanner.getLexem());
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;
			if(accept(Symbols::ident) || accept(Symbols::intc)) {
				Descriptor *d = nullptr;
				ident = expression(&d);
				if(d == nullptr || d->type != DescriptorTypes::scalarInt) {
					//ERROR unexpect type, must be int
					error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "Expected integer.");
				}
				chr->children.push_back(ident);
			} else {
				error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "Expected integer.");
			}
			expect(Symbols::rpar);
			scanner.getNextLexem();
			*resType = baseTable->findInTable((std::string)"char")->type;
			chr->convertType = *resType;
			return chr;
		}
		Descriptor *d = nullptr;
		res = getIdent(&d, false);
		*resType = d;

    } else if (accept(Symbols::intc)) {
		res = new IntLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"integer")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;
		*resType = ((CharLiteralNode*)res)->type;
		scanner.getNextLexem();
	} else if (accept(Symbols::floatc)) {
		res = new FloatLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"real")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;
		*resType = ((CharLiteralNode*)res)->type;
		scanner.getNextLexem();
	} else if (accept(Symbols::charc)) {
		res = new CharLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"char")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;
		*resType = ((CharLiteralNode*)res)->type;
		scanner.getNextLexem();
	} else if (accept(Symbols::lpar)) {
		scanner.getNextLexem();
		Descriptor *d = nullptr;
		res = condition(&d);
		//((ExpressionNode*)res)->convertType = d;
		expect(Symbols::rpar);
		scanner.getNextLexem();
		*resType = d;
	} else if (accept(Symbols::sys_not)) {
		UnarOperationNode *nt = new UnarOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();
		Descriptor *d = nullptr;
		nt->children.push_back(condition(&d));
		nt->convertType = d;
		res = nt;
		scanner.getNextLexem();
		*resType = d;
		if(d->type != DescriptorTypes::scalarInt && d->type != DescriptorTypes::scalarBoolean) {
			//error
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected boolean or integer.");
		}
	} else {
		error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "");
	}
	return res;
}

Descriptor *cast(Descriptor *d1, Descriptor *d2) {
	if(d1->type == DescriptorTypes::scalarFloat && d2->type == DescriptorTypes::scalarInt) {
		return d1;
	}
	if(d2->type == DescriptorTypes::scalarFloat && d1->type == DescriptorTypes::scalarInt) {
		return d2;
	}
	return d1;
}
Descriptor *castSecontToFirst(Descriptor *d1, Descriptor *d2) {
	if(d1->type == d2->type && (d1->type == DescriptorTypes::scalarFloat ||
		d1->type == DescriptorTypes::scalarInt || d1->type == DescriptorTypes::scalarChar ||
		d1->type == DescriptorTypes::scalarBoolean) ) {
		return d1;
	}
	if(d1->type == DescriptorTypes::scalarFloat && d2->type == DescriptorTypes::scalarInt) {
		return d1;
	}
	if(d1->type == DescriptorTypes::arrays && d2->type == DescriptorTypes::arrays) {

		if(((DescriptorArray*)d1)->isOpen) {
			if (((DescriptorArray*)d1)->baseType == ((DescriptorArray*)d2)->baseType &&
				((DescriptorArray*)d2)->indices.size() == 1) {
				return d1;
			}
		} else {
			if (((DescriptorArray*)d1)->baseType == ((DescriptorArray*)d2)->baseType &&
				((DescriptorArray*)d1)->indices.size() == ((DescriptorArray*)d2)->indices.size()) {
				bool control = true;
				for(unsigned int i = 0; i < ((DescriptorArray*)d1)->indices.size(); ++i) {
					DescriptorTypes dtype1 = ((DescriptorLimited*)(((DescriptorArray*)d1)->indices[i]))->baseType->type;
					DescriptorTypes dtype2 = ((DescriptorLimited*)(((DescriptorArray*)d2)->indices[i]))->baseType->type;
					//((DescriptorLimited*)dtype1)->_max == ((DescriptorLimited*)dtype2)->_max
					//((DescriptorLimited*)dtype1)->_min == ((DescriptorLimited*)dtype2)->_min
					if(dtype1 != dtype2) {
						control = false;
						break;
					}
				}
				if(control) {
					return d1;
				}
			}
		}
	}
	if(d1->type == DescriptorTypes::records && d2->type == DescriptorTypes::records) {
		if( ((DescriptorRecord*)d1)->rName.size() == ((DescriptorRecord*)d1)->rName.size()) {
			bool control = true;
			for(unsigned int i = 0; i < ((DescriptorRecord*)d1)->rName.size(); ++i) {
				if(((DescriptorRecord*)d1)->rName[i]->type->type != ((DescriptorRecord*)d2)->rName[i]->type->type) {
					control = false;
					break;
				}
			}
			if(control) {
				return d1;
			}
		}
	}
	if(d1->type == DescriptorTypes::references && d2->type == DescriptorTypes::references) {
		if(((DescriptorReferences*)d1)->baseType->type == ((DescriptorReferences*)d2)->baseType->type) {
			return d1;
		}
	}
	if(d1->type == DescriptorTypes::pointer && d2->type == DescriptorTypes::pointer) {
        if(((DescriptorPointers*)d2)->baseType == nullptr) {
            return d1;
        }
		if(((DescriptorPointers*)d1)->baseType->type == ((DescriptorPointers*)d2)->baseType->type) {
			return d1;
		}
	}
	return nullptr;
}

ExpressionNode *Parser::getIdent(Descriptor **resType, bool ifRecord) {
	UnarOperationNode *uoper = nullptr;
	DescriptorPointers *dp = nullptr;
	if(accept(Symbols::at)) {
		dp = new DescriptorPointers();
		uoper = new UnarOperationNode("uoperation", scanner.getLexem());
		scanner.getNextLexem();
	}
	ExpressionNode *res;
	if(!accept(Symbols::ident) && !accept(Symbols::sys_false) && !accept(Symbols::sys_true) && !accept(Symbols::sys_nil)) {
		error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
	}
	if(scanner.getLexem().lexem == "nil") {
        if(dp != nullptr) {
            error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
        }
        res = new VarNode("nil", scanner.getLexem());
        ((VarNode*)res)->type = baseTable->findInTable("nil");
        res->convertType = ((VarNode*)res)->type->type;
        *resType = ((VarNode*)res)->convertType;
        scanner.getNextLexem();
        return res;
	}

	if(scanner.getLexem().lexem == "true" || scanner.getLexem().lexem == "false") {
		Symbol *sym = symbolStack->findInTables(scanner.getLexem().lexem);
		DescriptorScalarBoolean *boolean = (DescriptorScalarBoolean*)baseTable->findInTable("boolean")->type;
		if(sym->type->type == DescriptorTypes::scalarInt &&
			(boolean->_true == sym || boolean->_false == sym)) {
				res = new BoolLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
				((CharLiteralNode*)res)->type = boolean;
				((CharLiteralNode*)res)->convertType = boolean;
				*resType = ((CharLiteralNode*)res)->type;
				scanner.getNextLexem();
				if(uoper != nullptr) {
					//dp->baseType = ((VarNode*)res)->convertType;
					//uoper->convertType = dp;
					//res = uoper;
					error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
				}
				return res;
			}
	}

	res = new VarNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
	if(ifRecord) {
		bool ifFind = false;
		for(unsigned int i = 0; i < ((DescriptorRecord*)(*resType))->rName.size(); ++i) {
            if(scanner.getLexem().lexem == ((DescriptorRecord*)(*resType))->rName[i]->lex.lexem) {
				((VarNode*)res)->type = ((DescriptorRecord*)(*resType))->rName[i];
				((VarNode*)res)->convertType = ((VarNode*)res)->type->type;
				*resType = ((DescriptorRecord*)(*resType))->rName[i]->type;

				ifFind = true;
				break;
			}
		}
		if(!ifFind) {
			//ERROR not param in record
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos,
							"There is no '" + scanner.getLexem().lexem + "' in the record.");
		}
	} else {
		((VarNode*)res)->type = symbolStack->findInTables(scanner.getLexem().lexem);
		if(((VarNode*)res)->type == nullptr) {
			//ERROR no find this ident
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos,
				                 "Variable not found.");
			}
			((VarNode*)res)->convertType = (((VarNode*)res)->type->type);
			*resType = ((VarNode*)res)->type->type;

		if(((VarNode*)res)->type->_class == SymbolTypes::types) {
			//ERROR
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos,
				                 "Variable not found.");
		}
	}
	scanner.getNextLexem();
	//for array
	Descriptor *d = nullptr;
	bool isArr=false;
	if(accept(Symbols::lbracket)) {
		isArr = true;
		unsigned int index = 0;
		if(((VarNode*)res)->type->type->type != DescriptorTypes::arrays) {
			//ERROR it is not array
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not array.");
		}
		ActionNode *op = new ActionNode("[]");
		scanner.getNextLexem();
		TreeNode *exp = expression(&d);
		if(((DescriptorLimited*)(((DescriptorArray*)(((VarNode*)res)->type->type))->indices[index]))->baseType->type != d->type) {
			//ERROR unexpect type
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		op->children.push_back(exp);
		while(accept(Symbols::comma)) {
			scanner.getNextLexem();
			exp = expression(&d);
			index++;
			if(((DescriptorArray*)(((VarNode*)res))->type->type)->indices.size() <= index) {
				//ERROR big index
				error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Big index.");
			}
			if(((DescriptorLimited*)(((DescriptorArray*)(((VarNode*)res))->type->type)->indices[index]))->baseType->type!= d->type) {
				//ERROR unexpect type
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			op->children.push_back(exp);
		}
		expect(Symbols::rbracket);
		scanner.getNextLexem();
		res->children.push_back(op);
		res->convertType = ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType;;
		*resType = ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType;
	}
	if(accept(Symbols::point)) {
		if(!isArr && ((VarNode*)res)->type->type->type != DescriptorTypes::records) {
			//ERROR it is not record
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a record.");
		}
		if(isArr && ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType->type != DescriptorTypes::records) {
			//ERROR it is not a array of record
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a array of record.");
		}
		ActionNode *op = new ActionNode(".");
		//d = *resType;
		scanner.getNextLexem();
		ExpressionNode *id = getIdent(resType, true);
		op->children.push_back(id);
		//scanner.getNextLexem();
		res->children.push_back(op);
		//std::cout << "###" <<(int)(id->convertType->type);
		res->convertType = id->convertType;
		*resType = id->convertType;

	} else if(accept(Symbols::lpar)) {
		if(((VarNode*)res)->type->_class != SymbolTypes::func && ((VarNode*)res)->type->_class != SymbolTypes::proc) {
			//ERROR it is not func
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a function.");
		}
		ActionNode *op = new ActionNode("func");
		scanner.getNextLexem();
		unsigned int inputParam = 0;
		while(!accept(Symbols::rpar)) {
			TreeNode *funcParam = condition(&d);
			if(inputParam >= ((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec.size()) {
				//ERRPR many arguments
				error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "There is no function with such parameters.");
			}
			Descriptor *dres;
			if(((SymbolVar*)((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam])->mot == MethodOfTransmission::paramvar) {
                dres = ((VarNode*)funcParam)->type->type;
			} else {
                dres = castSecontToFirst(((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam]->type, ((VarNode*)funcParam)->type->type);
			}
			if(dres == nullptr) {
				//ERROR
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			if(((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam]->type->type != dres->type) {
				//ERROR unexpect type
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			if(dres->type == DescriptorTypes::arrays && ((DescriptorArray*)dres)->isOpen == true) {//for open array
				dres = d;
				((VarNode*)funcParam)->type->type = d;
			}
			((ExpressionNode*)funcParam)->convertType = dres;
			op->children.push_back(funcParam);
			inputParam++;
			if(accept(Symbols::rpar)) {
				break;
			}
			expect(Symbols::comma);
			scanner.getNextLexem();
		}
		if(((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec.size() != inputParam) {
			//ERROR not exist function with this params
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "There is no function with such parameters.");
		}
		scanner.getNextLexem();
		res->children.push_back(op);
		if(((VarNode*)res)->type->_class == SymbolTypes::func) {
			res->convertType = ((SymbolFunc*)(((VarNode*)res)->type))->returnParam->type;
		} else {
			res->convertType = nullptr;
		}
	} else if(accept(Symbols::arrow)) {
		if(((VarNode*)res)->type->type->type == DescriptorTypes::pointer) {
			ExpressionNode *op = new ExpressionNode("^", scanner.getLexem());
			op->convertType = ((DescriptorPointers*)(((VarNode*)res)->type->type))->baseType;
			if(op->convertType == nullptr) {
                error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			scanner.getNextLexem();
			op->children.push_back(res);
			res = op;
			*resType = res->convertType;
			//std::cout << (int)(res->convertType->type)<<"&&&";
		} else {
			//ERROR must be pointer
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a pointer.");
		}
	}
	if(uoper != nullptr) {
		dp->baseType = ((VarNode*)res)->convertType;
		uoper->convertType = dp;
		uoper->children.push_back(res);
		res = uoper;
		*resType = res->convertType;
	}
	return res;
}

ExpressionNode *Parser::term(Descriptor **resType) {
	Descriptor *d1 = nullptr, *d2 = nullptr;
	ExpressionNode *fres = factor(&d1);
	//fres->convertType = d1;
	while (accept(Symbols::star) || accept(Symbols::slash) ||
		accept(Symbols::sys_div) || accept(Symbols::sys_shl) ||
		accept(Symbols::sys_shr) || accept(Symbols::sys_and) || accept(Symbols::sys_mod)) {
		if(fres->convertType->type == DescriptorTypes::scalarChar) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if(fres->convertType->type == DescriptorTypes::pointer) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		BinOperationNode *oper = new BinOperationNode("operation", scanner.getLexem());
		scanner.getNextLexem();
		ExpressionNode *f = factor(&d2);
		//f->convertType = d2;
		if(f->convertType->type == DescriptorTypes::scalarChar) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if(f->convertType->type == DescriptorTypes::pointer) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		oper->children.push_back(fres);
		oper->children.push_back(f);
		if((oper->id == (int)Symbols::star || oper->id == (int)Symbols::slash) &&
			(d1->type != DescriptorTypes::scalarInt && d1->type != DescriptorTypes::scalarFloat) &&
			(d2->type != DescriptorTypes::scalarInt && d2->type != DescriptorTypes::scalarFloat)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if((oper->id == (int)Symbols::sys_div || oper->id == (int)Symbols::sys_mod ||
			oper->id == (int)Symbols::sys_shr || oper->id == (int)Symbols::sys_shl) &&
			(d1->type != DescriptorTypes::scalarInt || d2->type != DescriptorTypes::scalarInt)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if(oper->id == (int)Symbols::sys_and) {
			if(((d1->type == DescriptorTypes::scalarInt && d2->type == DescriptorTypes::scalarInt) ||
				(d1->type == DescriptorTypes::scalarBoolean && d2->type == DescriptorTypes::scalarBoolean))) {

			} else {
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
		}
		d1 = cast(d1, d2);
		oper->convertType = d1;
		if(oper->id == (int)Symbols::slash) {
            oper->convertType = baseTable->findInTable("real")->type;
		}
		fres = oper;
		f = nullptr;
		oper = nullptr;
	}

	*resType = fres->convertType;
	return fres;
}

ExpressionNode *Parser::expression(Descriptor **resType) {
	UnarOperationNode *uoper = nullptr;
	if (accept(Symbols::plus) || accept(Symbols::minus)) {
		uoper = new UnarOperationNode("uoperation", scanner.getLexem());
		scanner.getNextLexem();
	}
	Descriptor *d1 = nullptr, *d2 = nullptr;
	ExpressionNode *tnres = term(&d1);
	//tnres->convertType = d1;

	if(uoper != nullptr) {
		if(tnres->convertType->type != DescriptorTypes::scalarInt &&
			tnres->convertType->type != DescriptorTypes::scalarFloat) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		uoper->children.push_back(tnres);
		tnres = uoper;
		tnres->convertType = d1;
	}
	ExpressionNode *tn = nullptr;
	BinOperationNode *oper = nullptr;
	while (accept(Symbols::plus) || accept(Symbols::minus) ||
		accept(Symbols::sys_or) || accept(Symbols::sys_xor)) {
		if(tnres->convertType->type == DescriptorTypes::scalarChar) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		oper = new BinOperationNode("operation", scanner.getLexem());
		scanner.getNextLexem();
		oper->children.push_back(tnres);

		tn = term(&d2);
		//tn->convertType = d2;
		if(tn->convertType->type == DescriptorTypes::scalarChar) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		oper->children.push_back(tn);

		if((oper->id == (int)Symbols::plus || oper->id == (int)Symbols::minus) &&
			(d1->type != DescriptorTypes::scalarInt && d1->type != DescriptorTypes::scalarFloat) &&
			(d2->type != DescriptorTypes::scalarInt && d2->type != DescriptorTypes::scalarFloat)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if(oper->id == (int)Symbols::sys_or || oper->id == (int)Symbols::sys_xor) {
			if(((d1->type == DescriptorTypes::scalarInt && d2->type == DescriptorTypes::scalarInt) ||
				(d1->type == DescriptorTypes::scalarBoolean && d2->type == DescriptorTypes::scalarBoolean))) {

			} else {
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
		}
		//std::cout << (int)(d2->type) << " : " <<(int)(d2->type);
		d1 = cast(d1, d2);
		oper->convertType = d1;
		tnres = oper;
		oper = nullptr;
		tn = nullptr;
	}
	*resType = tnres->convertType;
	return tnres;
}

ExpressionNode *Parser::condition(Descriptor **resType) {
	Descriptor *d1 = nullptr, *d2 = nullptr;
	ExpressionNode *expr0 = expression(&d1);
	//expr0->convertType = d1;
	while (accept(Symbols::equal) || accept(Symbols::latergreater) ||
		accept(Symbols::laterequal) || accept(Symbols::greaterequal) ||
		accept(Symbols::later) || accept(Symbols::greater)) {
		if(expr0->convertType->type != DescriptorTypes::scalarInt &&
			expr0->convertType->type != DescriptorTypes::scalarFloat &&
			expr0->convertType->type != DescriptorTypes::scalarChar &&
			expr0->convertType->type != DescriptorTypes::scalarBoolean) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		BinOperationNode *op = new BinOperationNode("operation", scanner.getLexem());

		scanner.getNextLexem();
		ExpressionNode *expr1 = expression(&d2);
		//expr1->convertType = d2;
		if(expr1->convertType->type != DescriptorTypes::scalarInt &&
			expr1->convertType->type != DescriptorTypes::scalarFloat &&
			expr1->convertType->type != DescriptorTypes::scalarChar &&
			expr1->convertType->type != DescriptorTypes::scalarBoolean) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if(expr0->convertType->type != DescriptorTypes::scalarChar ||
			expr1->convertType->type != DescriptorTypes::scalarChar) {
				if(expr0->convertType->type != expr1->convertType->type) {
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
				}
			}
			d1 = cast(d1, d2);
			op->children.push_back(expr0);
			op->children.push_back(expr1);
			op->convertType = baseTable->findInTable("boolean")->type;
			expr0 = op;
			op = nullptr;
			expr1 = nullptr;
	}
	*resType = expr0->convertType;
	return expr0;
}

TreeNode *Parser::statement(Descriptor **resType) {
	if(scanner.isLastLexem()) {
		return nullptr;
	}
	if (accept(Symbols::ident)) {
		std::string lex = scanner.getLexem().lexem;
		std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
		if(lex == "write") {
			Descriptor *d = nullptr;
			ActionNode *wr = new ActionNode("WRITE");
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;
			if(accept(Symbols::ident) || accept(Symbols::sys_string) || accept(Symbols::charc) ||
 				accept(Symbols::intc) || accept(Symbols::floatc)) {
				bool b = false;
				if(accept(Symbols::ident))
				b = true;
				if(b) {
					ident = getIdent(&d, false);
					if(d->type != DescriptorTypes::scalarChar && d->type != DescriptorTypes::scalarInt &&
						d->type != DescriptorTypes::scalarFloat && d->type != DescriptorTypes::scalarBoolean) {
						//ERROR
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
					}
					((ExpressionNode*)ident)->convertType = d;
				} else {
					if(scanner.getLexem().val == (int)Symbols::charc) {
						ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
						((LiteralNode*)ident)->type = baseTable->findInTable("char")->type;
						((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
					} else if(scanner.getLexem().val == (int)Symbols::floatc) {
						ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
						((LiteralNode*)ident)->type = baseTable->findInTable("real")->type;
						((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
					} else if(scanner.getLexem().val == (int)Symbols::charc) {
						ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
						((LiteralNode*)ident)->type = baseTable->findInTable("integer")->type;
						((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
					} else {
						ident = new StringNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
					}
					scanner.getNextLexem();
				}
				wr->children.push_back(ident);
			} else {
				error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
			}
			while(accept(Symbols::comma)) {
				scanner.getNextLexem();
				if(accept(Symbols::ident) || accept(Symbols::sys_string) || accept(Symbols::charc) ||
					accept(Symbols::intc) || accept(Symbols::floatc)) {
					bool b = false;
					if(accept(Symbols::ident))
						b = true;
					if(b) {
							ident = getIdent(&d, false);
						if(d->type != DescriptorTypes::scalarChar && d->type != DescriptorTypes::scalarInt &&
							d->type != DescriptorTypes::scalarFloat && d->type != DescriptorTypes::scalarBoolean) {
							//ERROR
							error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
						}
						((ExpressionNode*)ident)->convertType = d;
					} else {
						if(scanner.getLexem().val == (int)Symbols::charc) {
							ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
							((LiteralNode*)ident)->type = baseTable->findInTable("char")->type;
							((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
						} else if(scanner.getLexem().val == (int)Symbols::floatc) {
							ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
							((LiteralNode*)ident)->type = baseTable->findInTable("real")->type;
							((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
						} else if(scanner.getLexem().val == (int)Symbols::charc) {
							ident = new LiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
							((LiteralNode*)ident)->type = baseTable->findInTable("integer")->type;
							((LiteralNode*)ident)->convertType = ((LiteralNode*)ident)->type;
						} else {
							ident = new StringNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
						}
						scanner.getNextLexem();
					}
					wr->children.push_back(ident);
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
			Descriptor *d = nullptr;
			ExpressionNode *ident;
			expect(Symbols::ident);
			ident = getIdent(&d, false);
			if(d->type != DescriptorTypes::scalarChar && d->type != DescriptorTypes::scalarInt &&
				d->type != DescriptorTypes::scalarFloat && d->type != DescriptorTypes::scalarBoolean) {
				//ERROR
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
			ident->convertType = d;
			rd->children.push_back(ident);
			while(accept(Symbols::comma)) {
				scanner.getNextLexem();
				ident = getIdent(&d, false);
				if(d->type != DescriptorTypes::scalarChar && d->type != DescriptorTypes::scalarInt &&
					d->type != DescriptorTypes::scalarFloat && d->type != DescriptorTypes::scalarBoolean) {
					//ERROR
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
				}
				ident->convertType = d;
				rd->children.push_back(ident);
			}

			expect(Symbols::rpar);
			scanner.getNextLexem();
			return rd;
		} else if(lex == "ord") {
			ActionNode *ord = new ActionNode("ORD");
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;
			if(accept(Symbols::charc) || accept(Symbols::ident)) {
				//ident = expression();
				bool b = false;
				if(accept(Symbols::ident))
					b = true;
				if(b) {
					Descriptor *d = nullptr;
					ident = getIdent(&d, false);
					if(d == nullptr || d->type != DescriptorTypes::scalarChar) {
						//ERROR not legal var
						error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
					}
				} else {
					ident = new CharLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
					((CharLiteralNode*)ident)->type = baseTable->findInTable((std::string)"char")->type;
					scanner.getNextLexem();
					((CharLiteralNode*)ident)->convertType = ((CharLiteralNode*)ident)->type;
				}
				ord->children.push_back(ident);
			} else {
				error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
			}
			//scanner.getNextLexem();
			expect(Symbols::rpar);
			scanner.getNextLexem();
			*resType = baseTable->findInTable((std::string)"integer")->type;
			return ord;
		} else if(lex == "chr") {
			ActionNode *chr = new ActionNode("CHR");
			scanner.getNextLexem();
			expect(Symbols::lpar);
			scanner.getNextLexem();
			TreeNode *ident;
			if(accept(Symbols::ident) || accept(Symbols::intc)) {
				Descriptor *d = nullptr;
				ident = expression(&d);
				if(d->type != DescriptorTypes::scalarInt) {
					//ERROR unexpect type, must be int
					error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
				}
				//((ExpressionNode*)ident)->convertType = d;
				chr->children.push_back(ident);
			} else {
				error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
			}
			//scanner.getNextLexem();
			expect(Symbols::rpar);
			scanner.getNextLexem();
			return chr;
		}
		Descriptor *d1 = nullptr;
		Descriptor *d2 = nullptr;
		ExpressionNode *ident = getIdent(&d1, false);
		if(ident->nameLex == "nil" || ident->nameLex == "true" || ident->nameLex == "false") {
            error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
		}

		if(accept(Symbols::semicolon)) {
			if(scanner.getPrevLexem().val == (int)Symbols::rpar) {
				scanner.getNextLexem();
				//scanner.getNextLexem();
				return ident;
			} else {
				error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
			}
		}

		expect(Symbols::assign);

		if(scanner.getPrevLexem().val == (int)Symbols::rpar) {
			error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
		} else {
			scanner.getNextLexem();
		}

		BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();		ExpressionNode *expr = condition(&d2);//expression
		//expr->convertType = d2;
		eq->children.push_back(ident);
		eq->children.push_back(expr);
		d2 = castSecontToFirst(d1, d2);
		if(d2 == nullptr) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if(d1->type == d2->type) {
			eq->convertType = d2;
		} else {
			//ERROR in types
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		return eq;
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
			Descriptor *dres = nullptr;
			ident->children.push_back(statement(&dres));
		} while (accept(Symbols::semicolon));

		expect(Symbols::sys_end);
		scanner.getNextLexem();
		return ident;
	} else if (accept(Symbols::sys_if)) {
		TreeNode *br = new ActionNode("BRANCH");
		scanner.getNextLexem();
		Descriptor *d = nullptr;
		ExpressionNode *expr = condition(&d);
		if(d->type != DescriptorTypes::scalarBoolean) {
			//ERROR must be int
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
		}
		expr->convertType = d;
		expect(Symbols::sys_then);
		scanner.getNextLexem();

		IfActionNode *ifbody = new IfActionNode("IF_BODY");

		//scanner.getNextLexem();
		if(accept(Symbols::semicolon)) {
			ifbody->children.push_back(new FreeActionNode("FREE_NODE"));
			//scanner.getNextLexem();
		} else {
			Descriptor *dres = nullptr;
			ifbody->children.push_back(statement(&dres));
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
				Descriptor *dres = nullptr;
				elsebody->children.push_back(statement(&dres));
			}
			br->children.push_back(elsebody);
			if(accept(Symbols::semicolon)) {
				//scanner.getNextLexem();
			}
		} else {
			scanner.getPrevLexem();
		}
		return br;
	} else if (accept(Symbols::sys_while)) {
		bool flag = false;
		if(!inCycle) {
			inCycle = true;
			flag = true;
		}
		WhileActionNode *wh = new WhileActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		Descriptor *d = nullptr;
		ExpressionNode *expr = condition(&d);
		if(d->type != DescriptorTypes::scalarBoolean) {
			//ERROR must be int
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
		}
		expr->convertType = d;
		expect(Symbols::sys_do);
		scanner.getNextLexem();
		wh->children.push_back(expr);

		if(accept(Symbols::semicolon)) {
			wh->children.push_back(new FreeActionNode("FREE_NODE"));
			//scanner.getNextLexem();
		} else {
			Descriptor *dres = nullptr;
			wh->children.push_back(statement(&dres));
		}

		if(flag) {
			inCycle = false;
		}
		return wh;
	} else if (accept(Symbols::sys_for)) {
		bool flag = false;
		if(!inCycle) {
			inCycle = true;
			flag = true;
		}
		ForActionNode *fr = new ForActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();

		expect(Symbols::ident);
		Descriptor *d1 = nullptr, *d2 = nullptr;
		//VarOperationNode *ident = new VarOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		ExpressionNode *ident = getIdent(&d1, false);
		if(d1->type != DescriptorTypes::scalarInt) {
			//ERROR must be int
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
		}
		ident->convertType = d1;

		//scanner.getNextLexem();
		expect(Symbols::assign);
		BinOperationNode *eq = new BinOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();
		//std::cout << scanner.getLexem().lexem << "\n";
		ExpressionNode *expr0 = expression(&d2);
		if(d2->type != DescriptorTypes::scalarInt) {
			//ERROR must be int
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
		}
		expr0->convertType = d2;
		eq->children.push_back(ident);
		eq->children.push_back(expr0);
		eq->convertType = d2;

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
		ExpressionNode *expr1 = expression(&d2);
		if(d2->type != DescriptorTypes::scalarInt) {
			//ERROR must be int
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		expr1->convertType = d2;
		expect(Symbols::sys_do);
		scanner.getNextLexem();
		fr->children.push_back(eq);
		fr->children.push_back(expr1);
		if(accept(Symbols::semicolon)) {
			fr->children.push_back(new FreeActionNode("FREE_NODE"));
			//scanner.getNextLexem();
		} else {
			Descriptor *dres = nullptr;
			fr->children.push_back(statement(&dres));
		}
		if(flag) {
			inCycle = false;
		}
		return fr;
	} else if (accept(Symbols::sys_repeat)) {
		bool flag = false;
		if(!inCycle) {
			inCycle = true;
			flag = true;
		}
		RepeatActionNode *rt = new RepeatActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		if(accept(Symbols::semicolon)) {
			rt->children.push_back(new FreeActionNode("FREE_NODE"));
			//scanner.getNextLexem();
		} else {
			Descriptor *dres = nullptr;
			rt->children.push_back(statement(&dres));
		}
		if(accept(Symbols::semicolon)) {
			scanner.getNextLexem();
		}
		expect(Symbols::sys_until);
		scanner.getNextLexem();
		Descriptor *d = nullptr;
		ExpressionNode *expr = condition(&d);
		if(d->type != DescriptorTypes::scalarBoolean) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		expr->convertType = d;
		rt->children.push_back(expr);
		if(flag) {
			inCycle = false;
		}
		return rt;
	} else if(accept(Symbols::sys_break)) {
		if(!inCycle) {
			error.printError(Errors::ERROR_BREAK, scanner.getLexem().pos, "");
		}
		BreakActionNode *br = new BreakActionNode(scanner.getLexem().lexem);
		scanner.getNextLexem();
		return br;
	} else if(accept(Symbols::sys_continue)) {
		if(!inCycle) {
			error.printError(Errors::ERROR_CONTINUE, scanner.getLexem().pos, "");
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

void Parser::print() {
	baseTable->print(0);
	printTree(root, 0, 0);
}

Parser::~Parser() {
	 delete root;
}



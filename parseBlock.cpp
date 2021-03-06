#include "parser.h"

Descriptor *Parser::getArrLimit() {
    ExpressionNode *from = expression(false, false);
    from = Calculate(from);
    expect(Symbols::twopoints);
    scanner.getNextLexem();
    ExpressionNode *to = expression(false, false);
    to = Calculate(to);
    if(((VarNode*)from)->convertType->type != DescriptorTypes::scalarInt ||
        ((VarNode*)to)->convertType->type != DescriptorTypes::scalarInt) {
        //ERROR
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
    }
    DescriptorLimited *lim = new DescriptorLimited();
    lim->_min = from;
    lim->_max = to;
    lim->baseType = ((VarNode*)from)->convertType;
    return lim;
}

std::vector<SymbolVar*> Parser::getVarBlock(int ifFunc, SymbolTable *st, bool forFunc, bool ifRecord = false) {
	bool ifNeedNext = false;
	MethodOfTransmission mot = MethodOfTransmission::paramval;
	std::vector<Lexem> idents;
	if(forFunc) {
        if(accept(Symbols::sys_var)) {
            mot = MethodOfTransmission::paramvar;
            scanner.getNextLexem();
        }
    }
	do {
		if(ifNeedNext)
			scanner.getNextLexem();
		ifNeedNext = true;
        expect(Symbols::ident);
        idents.push_back(scanner.getLexem());

		scanner.getNextLexem();
	} while (accept(Symbols::comma));

	expect(Symbols::colon);
	scanner.getNextLexem();

	Descriptor *descr = getType(st, forFunc);

	scanner.getNextLexem();
	if(!forFunc) {
		expect(Symbols::semicolon);
		scanner.getNextLexem();
	}

	std::vector<SymbolVar*> result;
	for(unsigned int i = 0; i < idents.size(); ++i) {
		result.push_back(new SymbolVar());
		result[result.size()-1]->lex = idents[i];
        result[result.size()-1]->type = descr;
		result[result.size()-1]->mot = mot;
		if(result[result.size()-1]->type->type == DescriptorTypes::arrays && ((DescriptorArray*)result[result.size()-1]->type)->isOpen) {
            result[result.size()-1]->mot = MethodOfTransmission::paramvar;
		}
	}
	//array of array to double array
	while(true) {
        if(result[0]->type->type == DescriptorTypes::arrays) {
            if(((DescriptorArray*)result[0]->type)->baseType->type == DescriptorTypes::arrays) {
                DescriptorArray *darr = ((DescriptorArray*)((DescriptorArray*)result[0]->type)->baseType);
                for(unsigned int i = 0; i < darr->indices.size(); ++i) {
                    ((DescriptorArray*)result[0]->type)->indices.push_back(darr->indices[i]);
                }
                ((DescriptorArray*)result[0]->type)->baseType = darr->baseType;
            } else {
                break;
            }
        } else {
            break;
        }
	}
	return result;
}

std::string getSize(std::string name="", Descriptor* descr=nullptr) {
    if(descr->type == DescriptorTypes::scalarBoolean ||
       descr->type == DescriptorTypes::scalarChar ||
       descr->type == DescriptorTypes::scalarFloat ||
       descr->type == DescriptorTypes::scalarInt ||
       descr->type == DescriptorTypes::pointer) {
           return "1";
       }
    if(descr->type == DescriptorTypes::arrays) {
        DescriptorArray* darr = ((DescriptorArray*)descr);
        if(darr->isOpen)
            return "1";
        int _timesint = 1;
        for(int j = darr->indices.size()-1; j >= 0 ; --j) {
            int a = std::stoi(((ExpressionNode*)((DescriptorLimited*)darr->indices[j])->_max)->nameLex);
            int b = std::stoi(((ExpressionNode*)((DescriptorLimited*)darr->indices[j])->_min)->nameLex);
            _timesint *= (a - b + 1);
        }
        _timesint *= std::stoi(getSize("", darr->baseType));
        return std::to_string(_timesint);
    }
    if(descr->type == DescriptorTypes::records) {
        DescriptorRecord* drec = ((DescriptorRecord*)descr);
        int _timesint = 0;

        for(unsigned int i = 0; i < drec->rName.size(); ++i) {
            std::string _name = getSize(name+drec->rName[i]->lex.lexem, drec->rName[i]->type);
            _timesint += std::stoi(_name);
        }
        return std::to_string(_timesint);
    }
    return "";
}

Descriptor *Parser::getType(SymbolTable *st, bool forFunc = false) {
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
            Descriptor *lim = getArrLimit();
            st->descriptors.push_back(lim);
            arr->indices.push_back(lim);
            while(accept(Symbols::comma)) {
                scanner.getNextLexem();
                lim = getArrLimit();
                st->descriptors.push_back(lim);
                arr->indices.push_back(lim);
            }
            expect(Symbols::rbracket);
            scanner.getNextLexem();
        } else {
            Descriptor *lim = new DescriptorLimited();
            ((DescriptorLimited*)lim)->baseType = new DescriptorScalarInt();
            arr->indices.push_back(lim);
			arr->isOpen = true;
		}
        expect(Symbols::sys_of);
        scanner.getNextLexem();
        arr->baseType = getType(st, forFunc);
        return arr;
    } else if(accept(Symbols::ident)) {
        Symbol *typeSymbol = symbolStack->findInTables(scanner.getLexem().lexem);
        if(typeSymbol == nullptr) {
            //ERROR it is not a type
            error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
        }
        if(typeSymbol->_class == SymbolTypes::types) {
        } else {
            //ERROR it is not a type
            error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
        }
        return typeSymbol->type;
    } else if(scanner.getLexem().val == (int)Symbols::sys_record) {
        DescriptorRecord *structRecord = nullptr;
        structRecord = new DescriptorRecord();
        st->descriptors.push_back(structRecord);
        scanner.getNextLexem();
        do {
            std::vector<SymbolVar*> r = getVarBlock(-1, st, false, true);
            for(unsigned int i = 0; i < r.size(); ++i) {
                structRecord->rName.push_back(r[i]);
            }
        } while(accept(Symbols::ident));
        //type = structRecord;
        expect(Symbols::sys_end);
        return structRecord;
    } else if(accept(Symbols::arrow)) {
		DescriptorPointers *dp = new DescriptorPointers();
		scanner.getNextLexem();
		dp->baseType = getType(st);
		return dp;
	} else {
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }
    return nullptr;
}

TreeNode *Parser::block(bool ifFunc, SymbolTable *st) {
	TreeNode *blockNode = new BlockActionNode("BLOCK");
	symbolStack->push(st);
	while(1){
        if (accept(Symbols::sys_const)) {
            if(!ifFunc)
                ASMOther.Add("section", ".const");
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

                    TreeNode *expr = condition(ifFunc, false);
                    expr = Calculate((ExpressionNode*)expr);

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
                        type = ((VarNode*)expr)->convertType;
                    }
                    std::string _t = genAsmDerective(type);
                    _t = "v_" + identLex.lexem + ": " + _t + " " + ((ExpressionNode*)expr)->nameLex;
                    ASMOther.Add(_t, 0);
                    SymbolConst *ident = new SymbolConst(identLex, expr, type);
                    st->add(ident);//add new constant
                    expect(Symbols::semicolon);
                    scanner.getNextLexem();
                } while (accept(Symbols::ident));
            }
            continue;
        }
        if (accept(Symbols::sys_var)) {
            scanner.getNextLexem();
            do {
                std::vector<SymbolVar*> r = getVarBlock(ifFunc, st, false);
                for(unsigned int i = 0; i < r.size(); ++i) {
                    if(st->findInTable(r[i]->lex.lexem) != nullptr) {
                        error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
                    }
                    st->add(r[i]);
                }
            } while(accept(Symbols::ident));
            continue;
        }
        if (accept(Symbols::sys_type)) {
            SymbolType *symbolType = nullptr;
            scanner.getNextLexem();
            do {
                expect(Symbols::ident);
                symbolType = new SymbolType();
                symbolType->lex = scanner.getLexem();
                scanner.getNextLexem();
                expect(Symbols::equal);
                scanner.getNextLexem();

                symbolType->type = getType(st);

                scanner.getNextLexem();
                expect(Symbols::semicolon);
                scanner.getNextLexem();
                st->add(symbolType);
            } while (accept(Symbols::ident));
            continue;
        }
        if(accept(Symbols::sys_function)) {
            FuncNode *funcNode;
            SymbolFunc *func = new SymbolFunc();
            func->type = nullptr;
            scanner.getNextLexem();

            if(expect(Symbols::ident)) {
                funcNode = new FuncNode(scanner.getLexem());
                func->lex = scanner.getLexem();
                scanner.getNextLexem();
            }

            st->add(func);
            expect(Symbols::lpar);
            scanner.getNextLexem();
            if(!accept(Symbols::rpar))
            do {
                std::vector<SymbolVar*> r = getVarBlock(-1, st,  true);
                for(unsigned int i = 0; i < r.size(); ++i) {
                    if(func->inputParam->findInTable(r[i]->lex.lexem) != nullptr) {
                        error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
                    }
                    func->inputParam->add(r[i]);
                    if(r[i]->type->type == DescriptorTypes::arrays) {
                        if(((DescriptorArray*)r[i]->type)->isOpen) {
                            for(int y = 0; y < ((DescriptorArray*)r[i]->type)->indices.size(); y++) {
                                SymbolVar *svar = new SymbolVar();
                                TextPos pos;
                                Lexem _lex((std::string)(r[i]->lex.lexem+"size"+std::to_string(y)),
                                            (TextPos)pos, (int)Symbols::intc, (int)literalint);

                                svar->lex = _lex;
                                svar->mot = MethodOfTransmission::paramval;
                                svar->_class = SymbolTypes::vars;
                                svar->type = baseTable->findInTable("integer")->type;
                                func->inputParam->add(svar);
                            }
                        }
                    }
                }
                if(accept(Symbols::semicolon)) {
                    scanner.getNextLexem();
                    if(!accept(Symbols::ident) && !accept(Symbols::sys_var)) {
                        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expected identifier.");
                    }
                }
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
                    //std::cout <<"func " << (int)(func->returnParam->type->type);
                func->type = func->returnParam->type;
            } else {
                //ERROR it is not a type
                error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "It is not a type.");
            }
            scanner.getNextLexem();
            expect(Symbols::semicolon);
            scanner.getNextLexem();
            //symbolStack->push(func->inputParam);
            SymbolTable *st = new SymbolTable();
            SymbolVar *result = new SymbolVar();
            TextPos pos;
            Lexem resultlex("result", pos, (int)Symbols::ident,identifier);
            result->lex = resultlex;
            result->type = func->type;

            func->inputParam->addFront(result);
            //st->add(result);
            symbolStack->push(func->inputParam);

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
            if(!accept(Symbols::rpar))
            do {
                std::vector<SymbolVar*> r = getVarBlock(ifFunc, st, true);
                for(unsigned int i = 0; i < r.size(); ++i) {
                    if(proc->inputParam->findInTable(r[i]->lex.lexem) != nullptr) {
                        error.printError(Errors::ERROR_VAR_ALREADY_EXIST, scanner.getLexem().pos, "");
                    }
                    proc->inputParam->add(r[i]);
                    if(r[i]->type->type == DescriptorTypes::arrays) {
                        if(((DescriptorArray*)r[i]->type)->isOpen) {
                            //std::cout << "!!!!!!\n";
                            for(int y = 0; y < ((DescriptorArray*)r[i]->type)->indices.size(); y++) {
                                SymbolVar *svar = new SymbolVar();
                                TextPos pos;
                                Lexem _lex((std::string)(r[i]->lex.lexem+"size"+std::to_string(y)),
                                            (TextPos)pos, (int)Symbols::intc, (int)literalint);

                                svar->lex = _lex;
                                svar->mot = MethodOfTransmission::paramval;
                                svar->_class = SymbolTypes::vars;
                                svar->type = baseTable->findInTable("integer")->type;
                                proc->inputParam->add(svar);
                            }
                        }
                    }
                }
                if(accept(Symbols::semicolon)) {
                    scanner.getNextLexem();
                    if(!accept(Symbols::ident) && !accept(Symbols::sys_var)) {
                        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Expected identifier.");
                    }
                }
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
            procNode->type = proc;
            scanner.getNextLexem();
            blockNode->children.push_back(procNode);
            symbolStack->pop();
            continue;
        }
        break;
	}
	blockNode->children.push_back(statement(ifFunc));
	symbolStack->pop();
	return blockNode;
 }

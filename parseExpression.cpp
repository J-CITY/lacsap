#include "parser.h"

ExpressionNode *Parser::Calculate(ExpressionNode *expr) {
    float a = 0;
    float b = 0;
    TextPos pos(-1, -1);
    switch(expr->id) {
        case (int)Symbols::star:
            a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) *
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
        break;
		case (int)Symbols::slash:
            b = std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
            if(b == 0) {
                error.printError(Errors::ERROR_DIV_BY_ZERO, pos, "");
            }
            a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) / b;
            break;
		case (int)Symbols::equal:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) ==
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::later:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) <
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::greater:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) >
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::laterequal:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) <=
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::greaterequal:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) >=
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::latergreater:
		    a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) !=
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    break;
		case (int)Symbols::plus:
		    if(expr->children.size() == 1) {
               a = +std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex);
		    } else {
                a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) +
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    }
		    break;
		case (int)Symbols::minus:
		   if(expr->children.size() == 1) {
               a = -std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex);
		    } else {
                a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) -
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    }
		    break;
		case (int)Symbols::sys_xor:
		    if(expr->convertType->type == DescriptorTypes::scalarBoolean) {
                a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex);
                b = std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
                a = ((!a && b) || (a && !b));
		    } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
                a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) -
                std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    }
		    break;
		case (int)Symbols::sys_shl:
            a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) <<
                std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
        case (int)Symbols::sys_shr:
            a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) >>
                std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		case (int)Symbols::sys_or:
		    if(expr->convertType->type == DescriptorTypes::scalarBoolean) {
                a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) ||
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
                a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) |
                std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    }
		    break;
		case (int)Symbols::sys_not:
		    if(expr->convertType->type == DescriptorTypes::scalarBoolean) {
                a = !std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex);
		    } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
                a = ~std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex);
		    }
		case (int)Symbols::sys_mod:
		    b = std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
            if(b == 0) {
                error.printError(Errors::ERROR_DIV_BY_ZERO, pos, "");
            }
            a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) / b;
		case (int)Symbols::sys_div:
		    b = std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
            if(b == 0) {
                error.printError(Errors::ERROR_DIV_BY_ZERO, pos, "");
            }
            a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) / b;
		case (int)Symbols::sys_and:
		    if(expr->convertType->type == DescriptorTypes::scalarBoolean) {
                a = std::stof(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) &&
                std::stof(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    } else if(expr->convertType->type == DescriptorTypes::scalarInt) {
                a = std::stoi(Calculate(((ExpressionNode*)expr->children[0]))->nameLex) &
                std::stoi(Calculate(((ExpressionNode*)expr->children[1]))->nameLex);
		    }
		    break;
		case (int)Symbols::sys_true:
		    return expr;
		case (int)Symbols::sys_false:
		    return expr;
		case (int)Symbols::ident:
		    {
            if(expr->nameLex == "ord") {
                return Calculate((ExpressionNode*)expr->children[0]);
            }  else if(expr->nameLex == "chr") {
                return Calculate((ExpressionNode*)expr->children[0]);
            }
		    Symbol *sym = symbolStack->findInTables(expr->nameLex);
		    return ((ExpressionNode*)((SymbolConst*)sym)->expr);
		    }
		case (int)Symbols::floatc:
		    return expr;
		case (int)Symbols::intc:
		    return expr;
		case (int)Symbols::charc:
		    {
		    expr->nameLex = std::to_string((int)(expr->nameLex[1]));
		    return expr;
		    }
    }
    Lexem lex;
    if(expr->convertType->type == DescriptorTypes::scalarBoolean)
        lex.lexem = std::to_string((bool)a);
    else if(expr->convertType->type == DescriptorTypes::scalarInt)
        lex.lexem = std::to_string((int)a);
    else if(expr->convertType->type == DescriptorTypes::scalarFloat)
        lex.lexem = std::to_string((float)a);
    else if(expr->convertType->type == DescriptorTypes::scalarChar) {
        lex.lexem = std::to_string((unsigned char)a);
    }
    expr->nameLex = lex.lexem;
    return expr;

}

ExpressionNode *Parser::getOrd(bool ifFunc, bool needAsm) {
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
            ident = getIdent(ifFunc, nullptr, false, needAsm);
            if(((ExpressionNode*)ident)->convertType == nullptr || ((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarChar) {
                //ERROR not legal var
                error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
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
    return ord;
}

ExpressionNode *Parser::getChr(bool ifFunc, bool needAsm) {
    VarNode *chr = new VarNode("CHR", scanner.getLexem());
    scanner.getNextLexem();
    expect(Symbols::lpar);
    scanner.getNextLexem();
    TreeNode *ident;
    if(accept(Symbols::ident) || accept(Symbols::intc)) {
        ident = expression(ifFunc, needAsm);
        if(((ExpressionNode*)ident)->convertType->type != DescriptorTypes::scalarInt) {
            error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected integer.");
        }
        chr->children.push_back(ident);
    } else {
        error.printError(Errors::ERROR_WRONG_ARGUMENT, scanner.getLexem().pos, "");
    }
    expect(Symbols::rpar);
    scanner.getNextLexem();
    chr->convertType = baseTable->findInTable((std::string)"char")->type;
    return chr;
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
				((DescriptorArray*)d2)->indices.size() == ((DescriptorArray*)d2)->indices.size()) {
				return d1;
			}
		} else {
			if (((DescriptorArray*)d1)->baseType == ((DescriptorArray*)d2)->baseType &&
				((DescriptorArray*)d1)->indices.size() == ((DescriptorArray*)d2)->indices.size()) {
				bool control = true;
				for(unsigned int i = 0; i < ((DescriptorArray*)d1)->indices.size(); ++i) {
					DescriptorTypes dtype1 = ((DescriptorLimited*)(((DescriptorArray*)d1)->indices[i]))->baseType->type;
					DescriptorTypes dtype2 = ((DescriptorLimited*)(((DescriptorArray*)d2)->indices[i]))->baseType->type;
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

ExpressionNode *Parser::factor(bool ifFunc, bool needAsm, bool ifRead = false) {
	ExpressionNode *res;
	if (accept(Symbols::ident) || accept(Symbols::at) || accept(Symbols::arrow) || accept(Symbols::sys_false) ||
        accept(Symbols::sys_true) || accept(Symbols::sys_nil)) {
		std::string lex = scanner.getLexem().lexem;
		std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
		if(lex == "ord") {
			return getOrd(ifFunc, needAsm);
		}  else if(lex == "chr") {
			return getChr(ifFunc, needAsm);
		} else if(lex == "random") {
            res = new VarNode("RANDOM", scanner.getLexem());
            scanner.getNextLexem();
            expect(Symbols::lpar);
            scanner.getNextLexem();
            expect(Symbols::rpar);
            scanner.getNextLexem();
            res->convertType = baseTable->findInTable("integer")->type;
            return res;
		} else if(lex == "getch") {
            res = new VarNode("GETCH", scanner.getLexem());
            scanner.getNextLexem();
            expect(Symbols::lpar);
            scanner.getNextLexem();
            TreeNode *val;
			val = condition(ifFunc, true);
            if(((ExpressionNode*)val)->convertType->type != DescriptorTypes::scalarChar) {
                //ERROR
                error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
            }
            expect(Symbols::rpar);
            scanner.getNextLexem();
            res->children.push_back(val);
            res->convertType = baseTable->findInTable("boolean")->type;
            return res;
		}
		res = getIdent(ifFunc, nullptr, false, needAsm);
        /*if(res->nameLex == "^") {
        } else if(res->nameLex == "@") {
        } else if(res->convertType->type == DescriptorTypes::pointer) {
        } else if(res->convertType->type == DescriptorTypes::scalarInt ||
            res->convertType->type == DescriptorTypes::scalarFloat ||
            res->convertType->type == DescriptorTypes::scalarChar ||
            res->convertType->type == DescriptorTypes::scalarBoolean && (res->nameLex != "true" && res->nameLex != "false" &&
                                                                         res->nameLex != "nil")) {
		}*/

    } else if (accept(Symbols::intc)) {
		res = new IntLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"integer")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;
		scanner.getNextLexem();
	} else if (accept(Symbols::floatc)) {
		res = new FloatLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"real")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;
		scanner.getNextLexem();
	} else if (accept(Symbols::charc)) {
		res = new CharLiteralNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		((CharLiteralNode*)res)->type = baseTable->findInTable((std::string)"char")->type;
		((CharLiteralNode*)res)->convertType = ((CharLiteralNode*)res)->type;

		if(needAsm) {
            //ASM.Add(AsmOperation::asm_push, new AsmImn(std::to_string(scanner.getLexem().lexem[1])));
		}
		scanner.getNextLexem();
	} else if (accept(Symbols::lpar)) {
		scanner.getNextLexem();
		res = condition(ifFunc, needAsm);
		expect(Symbols::rpar);
		scanner.getNextLexem();
	} else if (accept(Symbols::sys_not)) {
		/*UnarOperationNode *nt = new UnarOperationNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
		scanner.getNextLexem();
		nt->children.push_back(condition(ifFunc, needAsm));
		nt->convertType = ((ExpressionNode*)nt->children[nt->children.size()-1])->convertType;
		res = nt;
		scanner.getNextLexem();
		if(nt->convertType->type != DescriptorTypes::scalarInt && nt->convertType->type != DescriptorTypes::scalarBoolean) {
			//error
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "Expected boolean or integer.");
		}*/
	} else {
		error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "");
	}
	return res;
}

ExpressionNode *Parser::getBracket(ExpressionNode *res, bool ifFunc, bool needAsm) {
    unsigned int index = 0;
    if(((VarNode*)res)->type->type->type != DescriptorTypes::arrays) {
        //ERROR it is not array
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not array.");
    }
    ActionNode *op = new ActionNode("[]");
    scanner.getNextLexem();
    TreeNode *exp = expression(ifFunc, needAsm);
    if(((DescriptorLimited*)(((DescriptorArray*)(((VarNode*)res)->type->type))->indices[index]))->baseType->type != ((ExpressionNode*)exp)->convertType->type) {
        //ERROR unexpect type
        error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
    }

    op->children.push_back(exp);
    while(accept(Symbols::comma)) {
        scanner.getNextLexem();
        exp = expression(ifFunc, needAsm);
        index++;
        if(((DescriptorArray*)(((VarNode*)res))->type->type)->indices.size() <= index) {
            //ERROR big index
            error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "Big index.");
        }
        if(((DescriptorLimited*)(((DescriptorArray*)(((VarNode*)res))->type->type)->indices[index]))->baseType->type!= ((ExpressionNode*)exp)->convertType->type) {
            //ERROR unexpect type
            error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
        }
        op->children.push_back(exp);
    }
    expect(Symbols::rbracket);
    scanner.getNextLexem();
    //res->needAsm = needAsm;
    res->children.push_back(op);
    res->convertType = ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType;
    return res;
}

ExpressionNode *Parser::getPoint(ExpressionNode *res, bool ifFunc, bool needAsm, bool isArr) {
    Descriptor *drec;
    if(!isArr && ((VarNode*)res)->type->type->type != DescriptorTypes::records) {
        //ERROR it is not record
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a record.");
    }
    if(!isArr && ((VarNode*)res)->type->type->type == DescriptorTypes::records) {
        drec = ((VarNode*)res)->type->type;
    }
    if(isArr && ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType->type != DescriptorTypes::records) {
        //ERROR it is not a array of record
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a array of record.");
    }
    if(isArr && ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType->type == DescriptorTypes::records) {
        drec = ((DescriptorArray*)(((VarNode*)res)->type->type))->baseType;
    }
    ActionNode *op = new ActionNode(".");
    scanner.getNextLexem();

    ExpressionNode *id = getIdent(ifFunc, &drec, true, needAsm);

    op->children.push_back(id);
    res->children.push_back(op);
    res->convertType = id->convertType;
    return res;
}

ExpressionNode *Parser::getLpar(ExpressionNode *res, bool ifFunc, bool needAsm) {
    if(((VarNode*)res)->type->_class != SymbolTypes::func && ((VarNode*)res)->type->_class != SymbolTypes::proc) {
        //ERROR it is not func
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "It is not a function.");
    }
    ActionNode *op = new ActionNode("func");
    scanner.getNextLexem();
    unsigned int inputParam = 1;
    if(((VarNode*)res)->type->_class == SymbolTypes::proc)
        inputParam = 0;
    while(!accept(Symbols::rpar)) {
        TreeNode *funcParam = condition(ifFunc, needAsm);

        if(inputParam >= ((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec.size()) {
            //ERRPR many arguments
            error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "There is no function with such parameters.");
        }
        Descriptor *dres;
        if(((SymbolVar*)((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam])->mot == MethodOfTransmission::paramvar) {
            dres = ((VarNode*)funcParam)->type->type;
        } else {
            dres = castSecontToFirst(((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam]->type,
                                        ((ExpressionNode*)funcParam)->convertType);
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
            dres = ((ExpressionNode*)funcParam)->convertType;
        }
        ((ExpressionNode*)funcParam)->convertType = dres;
        op->children.push_back(funcParam);
        //for open array
        if(((ExpressionNode*)funcParam)->convertType->type == DescriptorTypes::arrays) {
            if(((DescriptorArray*)((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam]->type)->isOpen) {
                inputParam += ((DescriptorArray*)((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec[inputParam]->type)->indices.size();
            }
        }
        inputParam++;


        if(accept(Symbols::rpar)) {
            break;
        }
        expect(Symbols::comma);
        scanner.getNextLexem();
    }
    if(((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec.size() != inputParam) {
        //ERROR not exist function with this params
        //std::cout << inputParam << " "<<((SymbolFunc*)(((VarNode*)res)->type))->inputParam->symbolsvec.size();
        error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos, "There is no function with such parameters.");
    }
    scanner.getNextLexem();
    res->children.push_back(op);
    if(((VarNode*)res)->type->_class == SymbolTypes::func) {
        res->convertType = ((SymbolFunc*)(((VarNode*)res)->type))->returnParam->type;
    } else {
        res->convertType = nullptr;
    }
    return res;
}

ExpressionNode *Parser::getIdent(bool ifFunc, Descriptor **recordDescr, bool ifRecord, bool needAsm) {
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
            scanner.getNextLexem();
            if(uoper != nullptr) {
                error.printError(Errors::ERROR_UNEXPECT_SYMBOL, scanner.getLexem().pos, "");
            }
            return res;
        }
	}

	res = new VarNode(scanner.getSymbolTypeStr(scanner.getLexem().type), scanner.getLexem());
	if(ifRecord) {
		bool ifFind = false;
		for(unsigned int i = 0; i < ((DescriptorRecord*)(*recordDescr))->rName.size(); ++i) {
            if(scanner.getLexem().lexem == ((DescriptorRecord*)(*recordDescr))->rName[i]->lex.lexem) {
				((VarNode*)res)->type = ((DescriptorRecord*)(*recordDescr))->rName[i];
				((VarNode*)res)->convertType = ((VarNode*)res)->type->type;
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

		if(((VarNode*)res)->type->_class == SymbolTypes::types) {
			//ERROR
			error.printError(Errors::ERROR_EXPRESSION, scanner.getLexem().pos,
				                 "Variable not found.");
		}
	}
	scanner.getNextLexem();
	//for array
	bool isArr = false;
	if(accept(Symbols::lbracket)) {
        isArr = true;
        res = getBracket(res, ifFunc, needAsm);
	}
	if(accept(Symbols::point)) {
        res = getPoint(res, ifFunc, needAsm, isArr);
	} else if(accept(Symbols::lpar)) {
		res = getLpar(res, ifFunc, needAsm);
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
	}
	return res;
}

ExpressionNode *Parser::term(bool ifFunc, bool needAsm) {
	ExpressionNode *fres = factor(ifFunc, needAsm);
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
		ExpressionNode *f = factor(ifFunc, needAsm);
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
			(fres->convertType->type != DescriptorTypes::scalarInt && fres->convertType->type != DescriptorTypes::scalarFloat) &&
			(f->convertType->type != DescriptorTypes::scalarInt && f->convertType->type != DescriptorTypes::scalarFloat)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if((oper->id == (int)Symbols::sys_div || oper->id == (int)Symbols::sys_mod ||
			oper->id == (int)Symbols::sys_shr || oper->id == (int)Symbols::sys_shl) &&
			(fres->convertType->type != DescriptorTypes::scalarInt || f->convertType->type != DescriptorTypes::scalarInt)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}

		if(oper->id == (int)Symbols::sys_and) {
			if(((fres->convertType->type == DescriptorTypes::scalarInt && f->convertType->type == DescriptorTypes::scalarInt) ||
				(fres->convertType->type == DescriptorTypes::scalarBoolean && f->convertType->type == DescriptorTypes::scalarBoolean))) {

			} else {
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
		}
		oper->convertType = cast(fres->convertType, f->convertType);
		if(oper->id == (int)Symbols::slash) {
            oper->convertType = baseTable->findInTable("real")->type;
		}
		fres = oper;
		f = nullptr;
		oper = nullptr;
	}
	return fres;
}

ExpressionNode *Parser::expression(bool ifFunc, bool needAsm) {
	UnarOperationNode *uoper = nullptr;
	if (accept(Symbols::plus) || accept(Symbols::minus) || accept(Symbols::sys_not)) {
		uoper = new UnarOperationNode("uoperation", scanner.getLexem());
		scanner.getNextLexem();
	}

	ExpressionNode *tnres = term(ifFunc, needAsm);
	if(uoper != nullptr) {
		if(tnres->convertType->type != DescriptorTypes::scalarInt &&
			tnres->convertType->type != DescriptorTypes::scalarFloat &&
			tnres->convertType->type != DescriptorTypes::scalarBoolean) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
        if(tnres->convertType->type == DescriptorTypes::scalarBoolean && uoper->nameLex != "not") {
            //ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
        }
        if(tnres->convertType->type == DescriptorTypes::scalarFloat && uoper->nameLex == "not") {
            //ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
        }
		uoper->children.push_back(tnres);
		uoper->convertType = tnres->convertType;
		tnres = uoper;
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
		tn = term(ifFunc, needAsm);
		if(tn->convertType->type == DescriptorTypes::scalarChar) {
			//ERROR
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		oper->children.push_back(tn);

		if((oper->id == (int)Symbols::plus || oper->id == (int)Symbols::minus) &&
			(tnres->convertType->type != DescriptorTypes::scalarInt && tnres->convertType->type != DescriptorTypes::scalarFloat) &&
			(tn->convertType->type != DescriptorTypes::scalarInt && tn->convertType->type != DescriptorTypes::scalarFloat)) {
			error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
		}
		if(oper->id == (int)Symbols::sys_or || oper->id == (int)Symbols::sys_xor) {
			if(((tnres->convertType->type == DescriptorTypes::scalarInt && tn->convertType->type == DescriptorTypes::scalarInt) ||
				(tnres->convertType->type == DescriptorTypes::scalarBoolean && tn->convertType->type == DescriptorTypes::scalarBoolean))) {

			} else {
				error.printError(Errors::ERROR_BAD_TYPE, scanner.getLexem().pos, "");
			}
		}
		oper->convertType = cast(tnres->convertType, tn->convertType);
		tnres = oper;
		oper = nullptr;
		tn = nullptr;
	}
	return tnres;
}

ExpressionNode *Parser::condition(bool ifFunc, bool needAsm) {
	ExpressionNode *expr0 = expression(ifFunc, needAsm);
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
		ExpressionNode *expr1 = expression(ifFunc, needAsm);
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
        cast(expr0->convertType, expr1->convertType);
        op->children.push_back(expr0);
        op->children.push_back(expr1);
        op->convertType = baseTable->findInTable("boolean")->type;
        expr0 = op;
        op = nullptr;
        expr1 = nullptr;
	}
	return expr0;
}

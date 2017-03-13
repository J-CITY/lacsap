#include "parser.h"

Parser::Parser(std::string _filePath) {
    scanner.open(_filePath);
    scanner.read();
    //scanner.printLexems();
}

 void Parser::error(std::string s) {
     std::cout << s << "\n";
     exit(-1);
 }

 void Parser::Parse() {
     program();
 }


 void Parser::program() {
     root = new BufferNode("PROGRAM");

     if(expect(scanner.cods["program"])) {
         root->children.push_back( new FunctionalNode("keyword",
                                                      scanner.getLexem().lexem,
                                                      scanner.getLexem().type,
                                                      scanner.getLexem().val));
         scanner.getNextLexem();
     }
     if(expect(scanner.cods["ident"])) {
         root->children.push_back(new FunctionalNode("identifier",
                                                     scanner.getLexem().lexem,
                                                     scanner.getLexem().type,
                                                     scanner.getLexem().val));
         scanner.getNextLexem();
     }
     expect(scanner.cods[";"]);
     scanner.getNextLexem();

     TreeNode *blockNode;
     blockNode = block();
     root->children.push_back(blockNode);

     expect(scanner.cods["."]);
 }

 TreeNode *Parser::block() {
     TreeNode *blockNode = new BufferNode("BLOCK");

     if (accept(scanner.cods["const"])) {
         blockNode->children.push_back(new FunctionalNode("keyword",
                                                          scanner.getLexem().lexem,
                                                          scanner.getLexem().type,
                                                          scanner.getLexem().val));
         scanner.getNextLexem();
         do {
             //scanner.getNextLexem();
             expect(scanner.cods["ident"]);
             FunctionalNode *ident = new FunctionalNode("identifier",
                                                        scanner.getLexem().lexem,
                                                        scanner.getLexem().type,
                                                        scanner.getLexem().val);
             scanner.getNextLexem();
             expect(scanner.cods["="]);
             FunctionalNode *eq = new FunctionalNode("operation",
                                                     scanner.getLexem().lexem,
                                                     scanner.getLexem().type,
                                                     scanner.getLexem().val);

             scanner.getNextLexem();

             eq->children.push_back(ident);
             eq->children.push_back(expression());
             blockNode->children[blockNode->children.size()-1]->children.push_back(eq);
             expect(scanner.cods[";"]);
             scanner.getNextLexem();
         } while (accept(scanner.cods["ident"]));
         //expect(scanner.cods[";"]);
         //scanner.getNextLexem();
     }
     if (accept(scanner.cods["var"])) {
         blockNode->children.push_back(new FunctionalNode("keyword",
                                                          scanner.getLexem().lexem,
                                                          scanner.getLexem().type,
                                                          scanner.getLexem().val));

         bool ifNeedNext = true;
         do {
             blockNode->children[blockNode->children.size()-1]->children.push_back(getVarBlock(ifNeedNext));
             ifNeedNext = false;
         } while(accept(scanner.cods["ident"]));
     }

     blockNode->children.push_back(statement());

     return blockNode;
 }

TreeNode *Parser::getVarBlock(bool ifNeedNext) {
    std::vector<TreeNode*> idents;
    do {
        if(ifNeedNext)
            scanner.getNextLexem();
        ifNeedNext = true;
        expect(scanner.cods["ident"]);
        idents.push_back(new FunctionalNode("identifier",
                                            scanner.getLexem().lexem,
                                            scanner.getLexem().type,
                                            scanner.getLexem().val));

        scanner.getNextLexem();
    } while (accept(scanner.cods[","]));
    expect(scanner.cods[":"]);
    scanner.getNextLexem();

    FunctionalNode *arr = nullptr;
    if(accept(scanner.cods["array"])) {
        arr = new FunctionalNode("keyword",
                                    scanner.getLexem().lexem,
                                    scanner.getLexem().type,
                                    scanner.getLexem().val);
        scanner.getNextLexem();
        expect(scanner.cods["["]);
        scanner.getNextLexem();
        TreeNode *from = expression();
        expect(scanner.cods[".."]);
        FunctionalNode *range = new FunctionalNode("range",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
        scanner.getNextLexem();
        TreeNode *to = expression();

        range->children.push_back(from);
        range->children.push_back(to);
        arr->children.push_back(range);

        while(accept(scanner.cods[","])) {
            scanner.getNextLexem();
            from = expression();
            expect(scanner.cods[".."]);
            range = new FunctionalNode("range",
                                        scanner.getLexem().lexem,
                                        scanner.getLexem().type,
                                        scanner.getLexem().val);
            scanner.getNextLexem();
            to = expression();

            range->children.push_back(from);
            range->children.push_back(to);
            arr->children.push_back(range);
        }
        expect(scanner.cods["]"]);
        scanner.getNextLexem();
        expect(scanner.cods["of"]);
        scanner.getNextLexem();

    }
    FunctionalNode *type = nullptr;
    if(accept(scanner.cods["ident"]) || accept(scanner.cods["record"])) {
        type = new FunctionalNode("identifier",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
    } else {
        error("syntax error is nod a type: " + std::to_string(scanner.getLexem().pos.row) +
            " " + std::to_string(scanner.getLexem().pos.col));
    }


    BufferNode *structRecord = nullptr;
    if(type->nameLex == "record") {
        structRecord = new BufferNode("struct");
        bool ifNeedNextRecord = true;
        do {
            structRecord->children.push_back(getVarBlock(ifNeedNextRecord));
            ifNeedNextRecord = false;
        } while(accept(scanner.cods["ident"]));

        expect(scanner.cods["end"]);
        //scanner.getNextLexem();
    }

    scanner.getNextLexem();

    expect(scanner.cods[";"]);
    scanner.getNextLexem();

    type->children = idents;
    if(structRecord != nullptr) {
        type->children.push_back(structRecord);
    }
    if(arr != nullptr) {
        arr->children.push_back(type);
        return arr;
    } else {
        return type;
    }

}

TreeNode *Parser::factor(void) {
    TreeNode *res;
    if (accept(scanner.cods["ident"])) {
        std::string lex = scanner.getLexem().lexem;
        std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
        if(lex == "ord") {
            FunctionalNode *ord = new FunctionalNode("ORD",
                                                        scanner.getLexem().lexem,
                                                        scanner.getLexem().type,
                                                        scanner.getLexem().val);
            scanner.getNextLexem();
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            if(accept(scanner.cods["char"]) || accept(scanner.cods["ident"])) {
                //ident = expression();
                bool b = false;
                if(accept(scanner.cods["ident"]))
                    b = true;
                //scanner.getNextLexem();
                if(b) {
                    ident = getIdent();
                } else {
                    ident = new FunctionalNode(scanner.getSymbolTypeStr(scanner.getLexem().type),
                                            scanner.getLexem().lexem,
                                            scanner.getLexem().type,
                                            scanner.getLexem().val);
                    scanner.getNextLexem();
                }
                ord->children.push_back(ident);
            } else {
                error("error bad argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
            }
            //scanner.getNextLexem();
            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return ord;
        }  else if(lex == "chr") {
            FunctionalNode *chr = new FunctionalNode("CHR",
                                                        scanner.getLexem().lexem,
                                                        scanner.getLexem().type,
                                                        scanner.getLexem().val);
            scanner.getNextLexem();
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            if(accept(scanner.cods["ident"]) || accept(scanner.cods["int"])) {
                ident = expression();
                chr->children.push_back(ident);
            } else {
                error("error wrong argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
            }
            //scanner.getNextLexem();
            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return chr;
        }

        res = getIdent();

    } else if (accept(scanner.cods["int"]) || accept(scanner.cods["float"]) || accept(scanner.cods["char"])) {
        res = new FunctionalNode(scanner.getSymbolTypeStr(scanner.getLexem().type),
                                    scanner.getLexem().lexem,
                                    scanner.getLexem().type,
                                    scanner.getLexem().val);
        scanner.getNextLexem();
    } else if (accept(scanner.cods["("])) {
        scanner.getNextLexem();
        res = condition();
        expect(scanner.cods[")"]);
        scanner.getNextLexem();
    } else {
        error("error in expression: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
    }

    return res;
}

TreeNode *Parser::getIdent() {
    TreeNode *res = new FunctionalNode("identifier",
                                scanner.getLexem().lexem,
                                scanner.getLexem().type,
                                scanner.getLexem().val);
    scanner.getNextLexem();
    //for array
    if(accept(scanner.cods["["])) {
        FunctionalNode *op = new FunctionalNode("[]",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        TreeNode *exp = expression();
        op->children.push_back(exp);
        while(accept(scanner.cods[","])) {
            scanner.getNextLexem();
            exp = expression();
            op->children.push_back(exp);
        }
        expect(scanner.cods["]"]);
        scanner.getNextLexem();
        res->children.push_back(op);
    }
    if(accept(scanner.cods["."])) {
        FunctionalNode *op = new FunctionalNode(".",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);

        scanner.getNextLexem();
        op->children.push_back(getIdent());
        //scanner.getNextLexem();
        res->children.push_back(op);

    }

    return res;
}

TreeNode *Parser::term() {
    TreeNode *fres = factor();
    while (accept(scanner.cods["*"]) || accept(scanner.cods["/"]) ||
            accept(scanner.cods["div"]) || accept(scanner.cods["shl"]) ||
            accept(scanner.cods["shr"]) || accept(scanner.cods["and"])) {
        FunctionalNode *oper = new FunctionalNode("operation",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
        scanner.getNextLexem();
        TreeNode *f = factor();

        oper->children.push_back(fres);
        oper->children.push_back(f);
        fres = oper;
    }
    return fres;
}

TreeNode *Parser::expression() {
    FunctionalNode *uoper = nullptr;
    if (accept(scanner.cods["+"]) || accept(scanner.cods["-"])) {
        uoper = new FunctionalNode("uoperation",
                                    scanner.getLexem().lexem,
                                    scanner.getLexem().type,
                                    scanner.getLexem().val);
        scanner.getNextLexem();
    }
    TreeNode *tnres = term();

    if(uoper != nullptr) {
        uoper->children.push_back(tnres);
        tnres = uoper;
    }
    TreeNode *tn = nullptr;
    FunctionalNode *oper = nullptr;
    while (accept(scanner.cods["+"]) || accept(scanner.cods["-"]) ||
            accept(scanner.cods["or"]) || accept(scanner.cods["xor"])) {

        oper = new FunctionalNode("operation",
                                scanner.getLexem().lexem,
                                scanner.getLexem().type,
                                scanner.getLexem().val);
        scanner.getNextLexem();
        oper->children.push_back(tnres);

        tn = term();
        oper->children.push_back(tn);

        tnres = oper;
        oper = nullptr;
        tn = nullptr;
    }
    return tnres;
}

TreeNode *Parser::condition() {
    TreeNode *expr0 = expression();
    if (accept(scanner.cods["="]) || accept(scanner.cods["<>"]) ||
        accept(scanner.cods["<="]) || accept(scanner.cods[">="]) ||
        accept(scanner.cods["<"]) || accept(scanner.cods[">"])) {

        FunctionalNode *op = new FunctionalNode("operation",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);

        scanner.getNextLexem();
        TreeNode *expr1 = expression();
        op->children.push_back(expr0);
        op->children.push_back(expr1);
        return op;
    }
    return expr0;
}

TreeNode *Parser::statement() {
    if(scanner.isLastLexem()) {
        return nullptr;
    }
    if (accept(scanner.cods["ident"])) {
        std::string lex = scanner.getLexem().lexem;
        std::transform(lex.begin(), lex.end(), lex.begin(), ::tolower);
        if(lex == "write") {
            FunctionalNode *wr = new FunctionalNode("WRITE",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
            scanner.getNextLexem();
            //std::cout<<scanner.getLexem().lexem << " !!!!\n";
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            if(accept(scanner.cods["ident"]) || accept(scanner.cods["string"]) || accept(scanner.cods["char"])) {
                bool b = false;
                if(accept(scanner.cods["ident"]))
                    b = true;
                if(b) {
                    ident = getIdent();
                } else {
                    ident = new FunctionalNode(scanner.getSymbolTypeStr(scanner.getLexem().type),
                                            scanner.getLexem().lexem,
                                            scanner.getLexem().type,
                                            scanner.getLexem().val);
                    scanner.getNextLexem();
                }
                wr->children.push_back(ident);
            } else {
                error("error wrong argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
            }
            //scanner.getNextLexem();
            while(accept(scanner.cods[","])) {
                scanner.getNextLexem();
                if(accept(scanner.cods["ident"]) || accept(scanner.cods["string"]) || accept(scanner.cods["char"])) {
                    bool b = false;
                    if(accept(scanner.cods["ident"]))
                        b = true;
                    if(b) {
                        ident = condition();
                    } else {
                        ident = new FunctionalNode(scanner.getSymbolTypeStr(scanner.getLexem().type),
                                            scanner.getLexem().lexem,
                                            scanner.getLexem().type,
                                            scanner.getLexem().val);
                        scanner.getNextLexem();
                    }
                    wr->children.push_back(ident);
                } else {
                    error("error wrong argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
                }

            }

            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return wr;
        } else if(lex == "read") {
            FunctionalNode *rd = new FunctionalNode("READ",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
            scanner.getNextLexem();
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            expect(scanner.cods["ident"]);
            ident = getIdent();
            rd->children.push_back(ident);

            while(accept(scanner.cods[","])) {
                scanner.getNextLexem();
                expect(scanner.cods["ident"]);
                ident = getIdent();
                rd->children.push_back(ident);
            }

            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return rd;
        } else if(lex == "ord") {
            FunctionalNode *ord = new FunctionalNode("ORD",
                                                        scanner.getLexem().lexem,
                                                        scanner.getLexem().type,
                                                        scanner.getLexem().val);
            scanner.getNextLexem();
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            if(accept(scanner.cods["char"]) || accept(scanner.cods["ident"])) {
                bool b = false;
                if(accept(scanner.cods["ident"]))
                    b = true;
                if(b) {
                    ident = getIdent();
                } else {
                    ident = new FunctionalNode(scanner.getSymbolTypeStr(scanner.getLexem().type),
                                            scanner.getLexem().lexem,
                                            scanner.getLexem().type,
                                            scanner.getLexem().val);
                    scanner.getNextLexem();
                }
                ord->children.push_back(ident);
            } else {
                error("error bad argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
            }
            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return ord;
        }  else if(lex == "chr") {
            FunctionalNode *chr = new FunctionalNode("CHR",
                                                        scanner.getLexem().lexem,
                                                        scanner.getLexem().type,
                                                        scanner.getLexem().val);
            scanner.getNextLexem();
            expect(scanner.cods["("]);
            scanner.getNextLexem();
            TreeNode *ident;
            if(accept(scanner.cods["ident"]) || accept(scanner.cods["int"])) {
                ident = expression();
                chr->children.push_back(ident);
            } else {
                error("error wrong argument: " + std::to_string(scanner.getLexem().pos.row) +
                        " " + std::to_string(scanner.getLexem().pos.col));
            }
            expect(scanner.cods[")"]);
            scanner.getNextLexem();
            return chr;
        }


        TreeNode *ident = getIdent();

        expect(scanner.cods[":="]);
        FunctionalNode *eq = new FunctionalNode("operation",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        TreeNode *expr = expression();
        eq->children.push_back(ident);
        eq->children.push_back(expr);
        return eq;
    } else if (accept(scanner.cods["begin"])) {
        FunctionalNode *ident = new FunctionalNode("keyword",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
        do {
            scanner.getNextLexem();
            if(accept(scanner.cods["end"]))
                break;
            ident->children.push_back(statement());
        } while (accept(scanner.cods[";"]));

        expect(scanner.cods["end"]);
        //std::cout << "!!!!\n";
        scanner.getNextLexem();
        return ident;
    } else if (accept(scanner.cods["if"])) {
        TreeNode *br = new BufferNode("BRANCH");
        scanner.getNextLexem();
        TreeNode *expr = condition();
        expect(scanner.cods["then"]);
        scanner.getNextLexem();

        BufferNode *ifbody = new BufferNode("IF_BODY");
        ifbody->children.push_back(statement());

        br->children.push_back(expr);
        br->children.push_back(ifbody);
        if(accept(scanner.cods[";"])) {
            scanner.getNextLexem();
        }
        //std::cout<<scanner.getLexem().lexem << "\n";
        if(accept(scanner.cods["else"])) {
            scanner.getNextLexem();
            BufferNode *elsebody = new BufferNode("ELSE_BODY");
            elsebody->children.push_back(statement());
            br->children.push_back(elsebody);
            if(accept(scanner.cods[";"])) {
                //scanner.getNextLexem();
            }
        } else {
            scanner.getPrevLexem();
        }
        //std::cout<<scanner.getLexem().lexem << "\n";
        return br;
    } else if (accept(scanner.cods["while"])) {
        bool flag = false;
        if(!inCycle) {
            inCycle = true;
            flag = true;
        }
        FunctionalNode *wh = new FunctionalNode("keyword",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        TreeNode *expr = condition();
        expect(scanner.cods["do"]);
        scanner.getNextLexem();
        wh->children.push_back(expr);
        wh->children.push_back(statement());
        if(flag) {
            inCycle = false;
        }
        return wh;
    } else if (accept(scanner.cods["for"])) {
        bool flag = false;
        if(!inCycle) {
            inCycle = true;
            flag = true;
        }
        FunctionalNode *fr = new FunctionalNode("keyword",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();

        expect(scanner.cods["ident"]);
        FunctionalNode *ident = new FunctionalNode("identifier",
                                                    scanner.getLexem().lexem,
                                                    scanner.getLexem().type,
                                                    scanner.getLexem().val);
        scanner.getNextLexem();
        expect(scanner.cods[":="]);
        FunctionalNode *eq = new FunctionalNode("operation",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();

        TreeNode *expr0 = expression();
        eq->children.push_back(ident);
        eq->children.push_back(expr0);

        if(accept(scanner.cods["to"]) || accept(scanner.cods["downto"])) {
            if(accept(scanner.cods["to"])) {
                fr->nameNode = "to";
            } else {
                fr->nameNode = "downto";
            }
            scanner.getNextLexem();
        } else {
            error("error forgot to or downto: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
        }
        TreeNode *expr1 = expression();
        //std::cout << scanner.getLexem().val;
        expect(scanner.cods["do"]);
        scanner.getNextLexem();
        fr->children.push_back(eq);
        fr->children.push_back(expr1);
        fr->children.push_back(statement());
        if(flag) {
            inCycle = false;
        }
        return fr;
    } else if (accept(scanner.cods["repeat"])) {
        bool flag = false;
        if(!inCycle) {
            inCycle = true;
            flag = true;
        }
        FunctionalNode *rt = new FunctionalNode("keyword",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        rt->children.push_back(statement());
        if(accept(scanner.cods[";"])) {
            scanner.getNextLexem();
        }
        expect(scanner.cods["until"]);
        scanner.getNextLexem();
        TreeNode *expr = condition();
        rt->children.push_back(expr);
        if(flag) {
            inCycle = false;
        }
        return rt;
    } else if(accept(scanner.cods["break"])) {
        if(!inCycle) {
            error("error break only in cycle: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
        }
        FunctionalNode *br = new FunctionalNode("keyword",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        return br;
    } else if(accept(scanner.cods["continue"])) {
        if(!inCycle) {
            error("error continue only in cycle: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
        }
        FunctionalNode *cn = new FunctionalNode("keyword",
                                                scanner.getLexem().lexem,
                                                scanner.getLexem().type,
                                                scanner.getLexem().val);
        scanner.getNextLexem();
        return cn;
    } else {
        error("syntax error: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
    }
}

bool Parser::accept(int symacc) {
    //std::cout<<scanner.getLexem().lexem << "\n";
    int symbol = scanner.getLexem().val;
    //std::cout << symacc << " " << symbol << std::endl;
    if (symbol == symacc) {
        return true;
    }
    return false;
}

bool Parser::expect(int symacc) {
    if (accept(symacc))
        return true;
    error("error unexpect symbol: " + std::to_string(scanner.getLexem().pos.row) +
                " " + std::to_string(scanner.getLexem().pos.col));
    return false;
}

void Parser::printTree(TreeNode *root, int l) {
    bool if_print = false;
    if(root->children.size() == 0) {
        for(int j = 0; j < l; ++j) {
            std::cout << "            ";
        }
        root->print();
        std::cout << std::endl;
    }

    for (unsigned int k = 0; k < root->children.size(); ++ k) {
        if(k < root->children.size()/2 && root->children[k] != nullptr){
            printTree(root->children[k], l+1);
            std::cout << std::endl;
        }
        if(k >= root->children.size()/2 && !if_print) {
            for(int j = 0; j < l; ++j) {
                std::cout << "            ";
            }
            root->print();
            std::cout << std::endl;
            if_print = true;
        }
        if(k >= root->children.size()/2  && root->children[k] != nullptr) {
            printTree(root->children[k], l+1);
            std::cout << std::endl;
        }
    }
}

void Parser::print() {
    printTree(root, 0);
}

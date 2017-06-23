#include "parser.h"

Symbol* SymbolStack::findInTables(std::string id) {
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

void SymbolStack::push(SymbolTable *in) {
    in->parent = symbolTable;
    symbolTable = in;
}

SymbolTable *SymbolStack::get() {
    return symbolTable;
}

void SymbolStack::pop() {
    symbolTable = symbolTable->parent;
}

Symbol* SymbolTable::findInTable(std::string id) {
    decltype(symbolsmap)::iterator it = symbolsmap.find(id);
    if(it != symbolsmap.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

int SymbolTable::movInTable(std::string id, bool param) {
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

void SymbolTable::addFront(Symbol *s) {
    symbolsvec.insert(symbolsvec.begin(), s);
    symbolsmap.insert(std::pair<std::string, Symbol*>(s->lex.lexem,
                        symbolsvec[0]));
}

void SymbolTable::add(Symbol *s) {
    symbolsvec.push_back(s);
    symbolsmap.insert(std::pair<std::string, Symbol*>(s->lex.lexem,
                        symbolsvec[symbolsvec.size()-1]));
}

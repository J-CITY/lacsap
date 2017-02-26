#include "scanner.h"

Scanner::Scanner(std::string _filePath) {
    filePath = _filePath;
    in.open(_filePath);
    if(!in.is_open()) {
        in.close();
        std::cout << "Can not open file!\n";
        exit(-2);
    }

    keyWords.insert("if");
    keyWords.insert("do");
    keyWords.insert("of");
    keyWords.insert("or");
    keyWords.insert("in");
    keyWords.insert("to");
    keyWords.insert("end");
    keyWords.insert("var");
    keyWords.insert("div");
    keyWords.insert("and");
    keyWords.insert("for");
    keyWords.insert("mod");
    keyWords.insert("nil");
    keyWords.insert("set");
    keyWords.insert("then");
    keyWords.insert("else");
    keyWords.insert("case");
    keyWords.insert("file");
    keyWords.insert("goto");
    keyWords.insert("type");
    keyWords.insert("with");
    keyWords.insert("begin");
    keyWords.insert("while");
    keyWords.insert("array");
    keyWords.insert("const");
    keyWords.insert("label");
    keyWords.insert("until");
    keyWords.insert("downto");
    keyWords.insert("packed");
    keyWords.insert("record");
    keyWords.insert("repeat");
    keyWords.insert("program");
    keyWords.insert("function");
    keyWords.insert("procedure");
}

std::string Scanner::getSymbolType(int type) {
    switch(type) {
    case identifier:
        return "identifier";
    case literalint:
        return "literalInt";
    case literalfloat:
        return "literalFloat";
    case literalchar:
        return "literalChar";
    case separator:
        return "separator";
    case operation:
        return "operation";
    case keyword:
        return "keyword";
    case directive:
        return "directive";
    default:
        return "unknown";
    }
}

unsigned char Scanner::nextChar() {
    textPos.col++;
    if((unsigned int)textPos.col >= str.size()) {
        //std::getline(in, str);
        if(!std::getline(in, str)) {
            EOFbool = true;
        }
        textPos.col = 0;
        textPos.row++;
    }
    return str[textPos.col];
}

void Scanner::readSpace(unsigned char ch) {
    if(ch != ' ' && ch != '\n' && ch != 0 && ch != '\t') {
        return;
    }
    ch = nextChar();
    while(ch == ' ' || ch == '\n' || ch == 0 || ch == '\t') {
        ch = nextChar();
    }
}

unsigned char Scanner::getType(unsigned char type) {
    if(
        (type >= 'A' && type <= 'Z') ||
        (type >= 'a' && type <= 'z') || type == '_'
        ) {
            return LETER;
        }
    if(type >= '0' && type <= '9') {
        return NUMBER;
    }
    return type;
}

void Scanner::readWord(unsigned char ch) {
    int len;
    while( (getType(ch) == LETER || getType(ch) == NUMBER) || ch == '_' ) {
        name += ch;
        len++;
        ch = nextChar();
    }
    std::string _name = name;
    for(unsigned int i = 0; i < name.length(); ++i) {
        _name[i] = std::tolower(name[i]);
    }
    if(keyWords.find(_name) != keyWords.end()) {
        symbol = cods[_name];
        symbolType = keyword;
        if(_name == "begin") {
            beginEndCount++;
        } else if(_name == "end") {
            beginEndCount--;
        }
    } else {
        symbol = cods["ident"] + identifiers;
        symbolType = identifier;
        identifiers++;
        for(unsigned int i = 0; i < lexems.size(); ++i) {
            if(lexems[i].type == identifier) {
                if(lexems[i].lexem == name) {
                    identifiers--;
                    symbol = lexems[i].val;
                    break;
                }
            }
        }


    }

}

void Scanner::readNumber(unsigned char ch) {
    int len;
    while ( getType(ch) == NUMBER || ch == '.' ) {
        if(ch == '.') {
            if(str.size() > (unsigned int)textPos.col+1 && str[textPos.col+1] == '.') {
                break;
            }
        }
        name += ch;
        len++;
        ch = nextChar();
    }
    int _int = atoi(name.c_str());
    std::string _str = static_cast<std::ostringstream*>( &(std::ostringstream() << _int) )->str();
    if(name == _str) {
        if(abs(_int) > 32767) {
            error(ERROR_BIG_INT_CONST);
        }
        symbol = cods["int"] + literalsInt;
        symbolType = literalint;
        literalsInt++;
        return;
    }
    float _float = atof(name.c_str());
    _str = static_cast<std::ostringstream*>( &(std::ostringstream() << _float) )->str();
    if(name == _str) {
        symbol = cods["float"] + literalsFloat;
        symbolType = literalfloat;
        literalsFloat++;
        return;
    }
    error(ERROR_IT_IS_NOT_NUMBER);
    return;
}

void Scanner::error(int err) {
    ERRORbool = true;
    std::stringstream ss;
    std::string a;

    errorStr += "ERROR: ";
    switch(err) {
    case ERROR_BAD_SYMBOL: {
        std::cout << "ERROR: " << ERROR_BAD_SYMBOL << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: bad symbol" << std::endl;
        ss.clear();
        ss << ERROR_BAD_SYMBOL;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_BIG_INT_CONST: {
        std::cout << "ERROR: " << ERROR_BIG_INT_CONST << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: big integer constant" << std::endl;
        ss.clear();
        ss << ERROR_BIG_INT_CONST;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_CHAR_CONST: {
        std::cout << "ERROR: " << ERROR_CHAR_CONST << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: in char constant" << std::endl;
        ss.clear();
        ss << ERROR_CHAR_CONST;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_IT_IS_NOT_NUMBER: {
        std::cout << "ERROR: " << ERROR_IT_IS_NOT_NUMBER << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: it is not number" << std::endl;
        ss.clear();
        ss << ERROR_IT_IS_NOT_NUMBER;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_FORGOT_PAR: {
        std::cout << "ERROR: " << ERROR_FORGOT_PAR << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: forgot ()" << std::endl;
        ss.clear();
        ss << ERROR_FORGOT_PAR;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_FORGOT_BASKETPAR: {
        std::cout << "ERROR: " << ERROR_FORGOT_BASKETPAR << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: forgot []" << std::endl;
        ss.clear();
        ss << ERROR_FORGOT_BASKETPAR;
        ss >> a;
        errorStr += a;
        break;
    }
    case ERROR_FORGOT_END_OR_BEGIN: {
        std::cout << "ERROR: " << ERROR_FORGOT_END_OR_BEGIN << " " << _pos.row << " " << _pos.col << std::endl;
        std::cout << "ERROR: forgot end or begin" << std::endl;
        ss.clear();
        ss << ERROR_FORGOT_END_OR_BEGIN;
        ss >> a;
        errorStr += a;
        break;
    }
    }
    errorStr += " ";
    ss.clear();
    ss << _pos.row;
    ss >> a;
    errorStr += a + " ";
    ss.clear();
    ss << _pos.col;
    ss >> a;
    errorStr += a;

    //exit(err);
}

void Scanner::nextLexem() {
    unsigned char ch = str[textPos.col];
    readSpace(ch);
    ch = str[textPos.col];
    unsigned char type = getType(ch);
    _pos.col = textPos.col;
    _pos.row = textPos.row;
    name = "";
    switch(type) {
    case LETER:
        readWord(ch);
    break;
    case '_':
        readWord(ch);
    break;
    case NUMBER:
        readNumber(ch);
    break;
    case '/':
        name += ch;
        symbol = cods[name];
        symbolType = operation;
        nextChar();
    break;
    case '*':
        name += ch;
        symbol = cods[name];
        symbolType = operation;
        nextChar();
    break;
    case '+':
        name += ch;
        symbol = cods[name];
        symbolType = operation;
        nextChar();
    break;
    case '-':
        name += ch;
        symbol = cods[name];
        symbolType = operation;
        nextChar();
    break;
    case '=':
        name += ch;
        symbol = cods[name];
        symbolType = operation;
        nextChar();
    break;
    case ',':
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ';':
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ':':
        name += ch;
        ch = nextChar();
        if(ch == '=') {
            name += ch;
            symbolType = operation;
            symbol = cods[name];
            nextChar();
            break;
        }
        symbol = cods[name];
        symbolType = separator;
    break;
    case '.':
        name += ch;
        ch = nextChar();
        if(ch == '.') {
            name += ch;
            symbol = cods[name];
            symbolType = separator;
            nextChar();
            break;
        }
        symbol = cods[name];
        symbolType = separator;
    break;
    case '(':
        parCount++;
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ')':
        parCount--;
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case '[':
        bascketCount++;
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ']':
        bascketCount--;
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case '{':
        fparCount++;
        ch = nextChar();
        readSpace(ch);
        ch = str[textPos.col];
        if(ch == '$') {
            symbol = directive;
            symbolType = directive;
            name = "{$";
        } else {
            symbolType = -1;
        }
        while(fparCount != 0) {
            ch = nextChar();
            if(ch == '}') {
                fparCount--;
            }
            if(ch == '{') {
                fparCount++;
            }
            if(ch == ' ' || ch == '\n' || ch == 0 || ch == '\t') {
                ch = ' ';
            }
            if(symbol == directive) {
                name += ch;
            }
            if(EOFbool) {
                break;
            }
        }
        nextChar();
    break;
    case '<':
        name += ch;
        ch = nextChar();
        if(ch == '=') {
            name += ch;
            symbolType = operation;
            symbol = cods[name];
            nextChar();
        } else if(ch == '>') {
            name += ch;
            symbolType = operation;
            symbol = cods[name];
            nextChar();
        } else {
            symbolType = operation;
            symbol = cods[name];
        }
    break;
    case '>':
        name += ch;
        ch = nextChar();
        if(ch == '=') {
            name += ch;
            symbolType = operation;
            symbol = cods[name];
            nextChar();
        } else {
            symbolType = operation;
            symbol = cods[name];
        }
    break;
    case '\'' :
    {
        name += ch;
        bool b = true;
        while(b) {
            ch = nextChar();
            if(ch == '\'') {
                ch = nextChar();
                if(ch == '\'') {
                    name += ch;
                } else {
                    name += '\'';
                    b = false;
                }
            } else {
                name += ch;
            }
            if(EOFbool) {
                break;
            }
        }
        symbol = cods["char"] + literalsChar;
        symbolType = literalchar;
        literalsChar++;
        if(lexems[lexems.size()-1].val == assign && (name == "''" || name.size() > 3)) {
            error(ERROR_CHAR_CONST);
        }
        break;
    }
    default:
    {
        error(ERROR_BAD_SYMBOL);
    }
    }
    if(symbolType != -1) {
        /*
        std::string s = getSymbolType(symbolType);
        std::cout.width(20);
        std::cout << name << " ";
        std::cout.width(3);
        std::cout << _pos.row << " ";
        std::cout.width(3);
        std::cout << _pos.col << " ";
        std::cout.width(15);
        std::cout  << s << " ";
        std::cout.width(5);
        std::cout << symbol <<  std::endl;
        */

        if(symbol == semicolon) {
            if(parCount != 0) {
                error(ERROR_FORGOT_PAR);
            }
            if(bascketCount != 0) {
                error(ERROR_FORGOT_BASKETPAR);
            }
        }

        Lexem l(name, _pos, symbol, symbolType);
        lexems.push_back(l);
    }
}
void Scanner::printLexems() {
    if(errorStr != "") {
        std::cout << errorStr << std::endl;
        return;
    }
    for(unsigned int i = 0; i < lexems.size(); ++i) {
        std::string s = getSymbolType(lexems[i].type);
        std::cout.width(20);
        std::cout << lexems[i].lexem << " ";
        std::cout.width(3);
        std::cout << lexems[i].pos.row << " ";
        std::cout.width(3);
        std::cout << lexems[i].pos.col << " ";
        std::cout.width(15);
        std::cout  << s << " ";
        std::cout.width(5);
        std::cout << lexems[i].val <<  std::endl;
    }
}
void Scanner::printLexems2() {
    std::cout << "\n\n";
    for(unsigned int i = 0; i < lexems.size(); ++i) {
        std::string s = getSymbolType(lexems[i].type);
        //std::cout.width(20);
        std::cout << lexems[i].lexem << " ";
        //std::cout.width(3);
        std::cout << lexems[i].pos.row << " ";
        //std::cout.width(3);
        std::cout << lexems[i].pos.col << " ";
        //std::cout.width(15);
        std::cout  << s << " ";
        //std::cout.width(5);
        std::cout << lexems[i].val <<  std::endl;
    }
}


void Scanner::test() {
    std::ifstream test;
    std::string testPath = filePath;
    std::string teststr, teststrin;
    testPath.resize(testPath.size() - 2);
    testPath += "out";

    test.open(testPath);
    if(!test.is_open()) {
        test.close();
        std::cout << "Can not open file!\n";
        exit(-2);
    }
    if(errorStr != "") {
        std::getline(test, teststrin);
        //std::cout << teststrin << std::endl << errorStr << std::endl;
        if(teststrin == errorStr) {
            std::cout << "OK\n";
        } else {
            std::cout << "ERROR\n";
        }
        return;
    }

    bool testBool = true;
    for(unsigned int i = 0; i < lexems.size(); ++i) {
        std::string s = getSymbolType(lexems[i].type);
        std::stringstream ss;
        std::string a;

        teststr = lexems[i].lexem+" ";
        ss.clear();
        ss << lexems[i].pos.row;
        ss >> a;
        teststr += a + " ";
        ss.clear();
        ss << lexems[i].pos.col;
        ss >> a;
        teststr += a + " ";
        teststr += s + " ";
        ss.clear();
        ss << lexems[i].val;
        ss >> a;
        teststr += a;
        std::getline(test, teststrin);
        //std::cout << teststr <<":" << std::endl << teststrin <<":" << std::endl;
        if(teststr != teststrin) {
            testBool = false;
            break;
        }
    }
    if(testBool) {
        std::cout << "OK\n";
    } else {
        std::cout << "ERROR\n";
    }
}

void Scanner::read() {
    std::getline(in, str);
    while(!EOFbool && !ERRORbool) {
        nextLexem();
        //getchar();
    }
    if(!ERRORbool) {
        if(beginEndCount != 0) {
            error(ERROR_FORGOT_END_OR_BEGIN);
        } else if(bascketCount != 0) {
            error(ERROR_FORGOT_BASKETPAR);
        } else if(parCount != 0) {
            error(ERROR_FORGOT_PAR);
        }
    }

}

Scanner::~Scanner() {
    in.close();
}

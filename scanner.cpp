#include "scanner.h"

const int LETER  = 101;
const int NUMBER = 102;

Scanner::Scanner(std::string _filePath) {
    filePath = _filePath;
    in.open(_filePath);
    if(!in.is_open()) {
        in.close();
        std::cout << "Can not open file!\n";
        exit(-2);
    }

    cods = {
      { "#", sharp          },
      { "*", star           },
      { "/", slash          },
      { "=", equal          },
      { ",", comma          },
      { ";", semicolon      },
      { ":", colon          },
      { ".", point          },
      { "^", arrow          },
      { "(", lpar           },
      { ")", rpar           },
      { "[", lbracket       },
      { "]", rbracket       },
      { "{", flpar          },
      { "}", frpar          },
      { "<", later          },
      { ">", greater        },
      { "<=", laterequal    },
      { ">=", greaterequal  },
      { "<>", latergreater  },
      {  "+", plus          },
      {  "-", minus         },
      { ":=", assign        },
      { "..", twopoints     },
      { "ident", ident        },
      { "float", floatc       },
      { "int", intc         },
      { "char", charc        },
      { "and", sys_and      },
      { "array", sys_array    },
      { "begin", sys_begin    },
      { "break", sys_break    },
      { "case", sys_case     },
      { "const", sys_const    },
      { "continue", sys_continue },
      { "div", sys_div      },
      { "do", sys_do       },
      { "downto", sys_downto   },
      { "else", sys_else     },
      { "end", sys_end      },
      { "false", sys_false    },
      { "file", sys_file     },
      { "for", sys_for      },
      { "function", sys_function },
      { "goto", sys_goto     },
      { "if", sys_if       },
      { "lable", sys_lable    },
      { "mod", sys_mod      },
      { "nil", sys_nil      },
      { "not", sys_not      },
      { "of", sys_of       },
      { "or", sys_or       },
      { "packed", sys_packed   },
      { "procedure", sys_procedure},
      { "program", sys_program  },
      { "record", sys_record   },
      { "repeat", sys_repeat   },
      { "set", sys_set      },
      { "shl", sys_shl      },
      { "shr", sys_shr      },
      { "string", sys_string   },
      { "then", sys_then     },
      { "to", sys_to       },
      { "true", sys_true     },
      { "type", sys_type     },
      { "unit", sys_unit     },
      { "until", sys_until    },
      { "var", sys_var      },
      { "while", sys_while    },
      { "with", sys_with     },
      { "xor", sys_xor      }
    };

    keyWords.insert("and");
    keyWords.insert("array");
    keyWords.insert("begin");
    keyWords.insert("break");
    keyWords.insert("case");
    keyWords.insert("const");
    keyWords.insert("continue");
    keyWords.insert("div");
    keyWords.insert("do");
    keyWords.insert("downto");
    keyWords.insert("else");
    keyWords.insert("end");
    keyWords.insert("false");
    keyWords.insert("file");
    keyWords.insert("for");
    keyWords.insert("function");
    keyWords.insert("goto");
    keyWords.insert("if");
    keyWords.insert("lable");
    keyWords.insert("mod");
    keyWords.insert("nil");
    keyWords.insert("not");
    keyWords.insert("of");
    keyWords.insert("or");
    keyWords.insert("packed");
    keyWords.insert("procedure");
    keyWords.insert("program");
    keyWords.insert("record");
    keyWords.insert("repeat");
    keyWords.insert("set");
    keyWords.insert("shl");
    keyWords.insert("shr");
    keyWords.insert("string");
    keyWords.insert("then");
    keyWords.insert("to");
    keyWords.insert("true");
    keyWords.insert("type");
    keyWords.insert("unit");
    keyWords.insert("until");
    keyWords.insert("var");
    keyWords.insert("while");
    keyWords.insert("with");
    keyWords.insert("xor");

}

std::string Scanner::getSymbolTypeStr(int type) {
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

unsigned char Scanner::getSymbolType(unsigned char type) {
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

unsigned char Scanner::nextChar() {
    textPos.col++;
    if((unsigned int)textPos.col >= str.size()) {
        if(!std::getline(in, str)) {
            EOFbool = true;
        }
        str += '\n';
        textPos.col = 0;
        textPos.row++;
    }
    return str[textPos.col];
}

unsigned char Scanner::nextLine() {
    if(!std::getline(in, str)) {
        EOFbool = true;
    }
    str += '\n';
    textPos.col = 0;
    textPos.row++;
    return str[textPos.col];
}

void Scanner::readSpace(unsigned char ch) {
    if(ch != ' ' && ch != '\n' && ch != 0 && ch != '\t') {
        return;
    }
    ch = nextChar();
    while(ch == ' ' || ch == '\n' || ch == 0 || ch == '\t') {
        ch = nextChar();
        if(EOFbool) {
            return;
        }
    }
}

void Scanner::readIdentifier(unsigned char ch) {
    int len;
    while( (getSymbolType(ch) == LETER || getSymbolType(ch) == NUMBER) || ch == '_' ) {
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
    } else {
        symbol = cods["ident"];
        symbolType = identifier;
    }

}

void Scanner::readNumber(unsigned char ch) {
    int len;
    while ( getSymbolType(ch) == NUMBER || ch == '.' || ch == '+' || ch == '-' || ch == 'e' || ch == 'E') {
        if(ch == '.') {
            if(str.size() > (unsigned int)textPos.col+1 && str[textPos.col+1] == '.') {
                break;
            }
        }
        name += ch;
        len++;
        ch = nextChar();
    }
    std::string::size_type sz;
    int _int = std::stoi(name, &sz);
    if(sz == name.size()) {
        if(abs(_int) > 32767) {
            error(ERROR_BIG_INT_CONST);
        }
        symbol = cods["int"];
        symbolType = literalint;
        return;
    }

    double _real = std::stod(name, &sz);
    if(sz == name.size()) {
        symbol = cods["float"];
        symbolType = literalfloat;
        return;
    }
    error(ERROR_IT_IS_NOT_NUMBER);
    return;
}

void Scanner::error(int err) {
    ERRORbool = true;
    errorStr += "ERROR: ";
    switch(err) {
    case ERROR_BAD_SYMBOL: {
        errorStr += "ERROR BAD SYMBOL (";
        break;
    }
    case ERROR_BIG_INT_CONST: {
        errorStr += "ERROR BIG INTEGER CONST (";
        break;
    }
    case ERROR_IT_IS_NOT_NUMBER: {
        errorStr += "ERROR IT IS NOT NUMBER (";
        break;
    }
    case ERROR_FORGOT_TILDA: {
        errorStr += "ERROR FORGOT TILDA (";
        break;
    }
    case ERROR_BIG_CHAR_CONST: {
        errorStr += "ERROR BIG CHAR CONST (";
        break;
    }
    case ERROR_FORGOT_FPAR: {
        errorStr += "ERROR FORGOT FPAR (";
        break;
    }
    }
    errorStr = errorStr + std::to_string(err) + ")"
    + " row: " + std::to_string(_pos.row) + " col: " + std::to_string(_pos.row) + '\n';

    //exit(err);
}

void Scanner::nextLexem() {
    unsigned char ch = str[textPos.col];
    readSpace(ch);
    if(EOFbool) {
        return;
    }
    ch = str[textPos.col];
    unsigned char type = getSymbolType(ch);
    _pos.col = textPos.col;
    _pos.row = textPos.row;
    name = "";
    switch(type) {
    case LETER:
        readIdentifier(ch);
    break;
    case '_':
        readIdentifier(ch);
    break;
    case NUMBER:
        readNumber(ch);
    break;
    case '/':
        ch = nextChar();
        if(ch != '/') {
            name += '/';
            symbol = cods[name];
            symbolType = operation;
        } else {
            symbolType = -1;
            nextLine();
        }
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
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ')':
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case '[':
        name += ch;
        symbol = cods[name];
        symbolType = separator;
        nextChar();
    break;
    case ']':
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
            name = "{";
        } else {
            symbolType = -1;
        }
        while(fparCount != 0) {
            if(EOFbool) {
                error(ERROR_FORGOT_FPAR);
                break;
            }
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
            ch = nextChar();
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
            if(ch == '\n') {
                error(ERROR_FORGOT_TILDA);
                b = false;
            }
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
                error(ERROR_FORGOT_TILDA);
                break;
            }
        }
        symbol = cods["char"];
        symbolType = literalchar;
        break;
    }
    case '\\':
        ch = nextChar();
        if(ch != '\\') {
            error(ERROR_BAD_SYMBOL);
        }
        symbolType = -1;
        nextLine();
        break;
    case '#':
    {
        ch = nextChar();
        if(getSymbolType(ch) != NUMBER) {
            error(ERROR_BAD_SYMBOL);
            break;
        }
        while ( getSymbolType(ch) == NUMBER) {
            name += ch;
            ch = nextChar();
        }
        int _int = atoi(name.c_str());

        if(_int > 255) {
            error(ERROR_BIG_CHAR_CONST);
        }
        symbol = cods["char"];
        symbolType = literalchar;
        name = (unsigned char)_int;

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
        std::string s = getSymbolTypeStr(lexems[i].type);
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

void Scanner::test() {
    if(errorStr != "") {
        std::cout << errorStr << std::endl;
        return;
    }
    std::string teststr;
    for(unsigned int i = 0; i < lexems.size(); ++i) {
        teststr = lexems[i].lexem+" " + std::to_string(lexems[i].pos.row) + " "
                            + std::to_string(lexems[i].pos.col) + " " + getSymbolTypeStr(lexems[i].type)
                            +" "+ std::to_string(lexems[i].val) + "\n";

        std::cout << teststr;
    }

}

void Scanner::read() {
    std::getline(in, str);
    str += '\n';
    while(!EOFbool && !ERRORbool) {
        nextLexem();
        //getchar();
    }


}

Scanner::~Scanner() {
    in.close();
}

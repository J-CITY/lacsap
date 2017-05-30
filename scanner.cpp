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
	init();
}

Scanner::Scanner() {
	init();
}

void Scanner::open(std::string _filePath) {
	filePath = _filePath;
	in.open(_filePath);
	if(!in.is_open()) {
		in.close();
		std::cout << "Can not open file!\n";
		exit(-2);
	}
}

void Scanner::init() {
	cods = {
		{ "#",        (int)Symbols::sharp        },
		{ "*",        (int)Symbols::star         },
		{ "/",        (int)Symbols::slash        },
		{ "=",        (int)Symbols::equal        },
		{ ",",        (int)Symbols::comma        },
		{ ";",        (int)Symbols::semicolon    },
		{ ":",        (int)Symbols::colon        },
		{ ".",        (int)Symbols::point        },
		{ "^",        (int)Symbols::arrow        },
		{ "@",        (int)Symbols::at           },
		{ "(",        (int)Symbols::lpar         },
		{ ")",        (int)Symbols::rpar         },
		{ "[",        (int)Symbols::lbracket     },
		{ "]",        (int)Symbols::rbracket     },
		{ "{",        (int)Symbols::flpar        },
		{ "}",        (int)Symbols::frpar        },
		{ "<",        (int)Symbols::later        },
		{ ">",        (int)Symbols::greater      },
		{ "<=",       (int)Symbols::laterequal   },
		{ ">=",       (int)Symbols::greaterequal },
		{ "<>",       (int)Symbols::latergreater },
		{  "+",       (int)Symbols::plus         },
		{  "-",       (int)Symbols::minus        },
		{ ":=",       (int)Symbols::assign       },
		{ "..",       (int)Symbols::twopoints    },
		{ "ident",    (int)Symbols::ident        },
		{ "float",    (int)Symbols::floatc       },
		{ "int",      (int)Symbols::intc         },
		{ "char",     (int)Symbols::charc        },
		{ "and",      (int)Symbols::sys_and      },
		{ "array",    (int)Symbols::sys_array    },
		{ "begin",    (int)Symbols::sys_begin    },
		{ "break",    (int)Symbols::sys_break    },
		{ "case",     (int)Symbols::sys_case     },
		{ "const",    (int)Symbols::sys_const    },
		{ "continue", (int)Symbols::sys_continue },
		{ "div",      (int)Symbols::sys_div      },
		{ "do",       (int)Symbols::sys_do       },
		{ "downto",   (int)Symbols::sys_downto   },
		{ "else",     (int)Symbols::sys_else     },
		{ "end",      (int)Symbols::sys_end      },
		{ "false",    (int)Symbols::sys_false    },
		{ "file",     (int)Symbols::sys_file     },
		{ "for",      (int)Symbols::sys_for      },
		{ "function", (int)Symbols::sys_function },
		{ "goto",     (int)Symbols::sys_goto     },
		{ "if",       (int)Symbols::sys_if       },
		{ "lable",    (int)Symbols::sys_lable    },
		{ "mod",      (int)Symbols::sys_mod      },
		{ "nil",      (int)Symbols::sys_nil      },
		{ "not",      (int)Symbols::sys_not      },
		{ "of",       (int)Symbols::sys_of       },
		{ "or",       (int)Symbols::sys_or       },
		{ "packed",   (int)Symbols::sys_packed   },
		{ "procedure",(int)Symbols::sys_procedure},
		{ "program",  (int)Symbols::sys_program  },
		{ "record",   (int)Symbols::sys_record   },
		{ "repeat",   (int)Symbols::sys_repeat   },
		{ "set",      (int)Symbols::sys_set      },
		{ "shl",      (int)Symbols::sys_shl      },
		{ "shr",      (int)Symbols::sys_shr      },
		{ "string",   (int)Symbols::sys_string   },
		{ "then",     (int)Symbols::sys_then     },
		{ "to",       (int)Symbols::sys_to       },
		{ "true",     (int)Symbols::sys_true     },
		{ "type",     (int)Symbols::sys_type     },
		{ "unit",     (int)Symbols::sys_unit     },
		{ "until",    (int)Symbols::sys_until    },
		{ "var",      (int)Symbols::sys_var      },
		{ "while",    (int)Symbols::sys_while    },
		{ "with",     (int)Symbols::sys_with     },
		{ "xor",      (int)Symbols::sys_xor      }
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
	if((type >= 'A' && type <= 'Z') ||
		(type >= 'a' && type <= 'z') || type == '_') {
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
	for(unsigned int i = 0; i < name.length(); ++i) {
		name[i] = std::tolower(name[i]);
	}
	if(keyWords.find(name) != keyWords.end()) {
		symbol = cods[name];
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
		if(ch == '+' || ch == '-') {
			if(name[name.size()-1] != 'e' && name[name.size()-1] != 'E') {
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
			error.printError(Errors::ERROR_BIG_INT_CONST, _pos, "");
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
	error.printError(Errors::ERROR_IT_IS_NOT_NUMBER, _pos, "");
	return;
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
	case '^':
		name += ch;
		symbol = cods[name];
		symbolType = operation;
		nextChar();
	break;
	case '@':
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
				error.printError(Errors::ERROR_FORGOT_FPAR, _pos, "");
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
				error.printError(Errors::ERROR_FORGOT_TILDA, _pos, "");
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
				error.printError(Errors::ERROR_FORGOT_TILDA, _pos, "");
				break;
			}
		}
		if(name.size() == 3) {
			symbol = cods["char"];
			symbolType = literalchar;
		} else {
		    name.erase(0, 1);
		    name.erase(name.size()-1, 1);
			symbol = cods["string"];
			symbolType = literalchar;
		}
	break;
	}
	case '\\':
		ch = nextChar();
		if(ch != '\\') {
			error.printError(Errors::ERROR_BAD_SYMBOL, _pos, "");
		}
		symbolType = -1;
		nextLine();
	break;
	case '#':
	{
		ch = nextChar();
		if(getSymbolType(ch) != NUMBER) {
			error.printError(Errors::ERROR_BAD_SYMBOL, _pos, "");
			break;
		}
		while ( getSymbolType(ch) == NUMBER) {
			name += ch;
			ch = nextChar();
		}
		int _int = atoi(name.c_str());

		if(_int > 255) {
			error.printError(Errors::ERROR_BIG_CHAR_CONST, _pos, "");
		}
		symbol = cods["char"];
		symbolType = literalchar;
		name = "'";
		name += (unsigned char)_int;
		name += "'";
	break;
	}
	default:
	{
		error.printError(Errors::ERROR_BAD_SYMBOL, _pos, "");
	}
	}
	if(symbolType != -1) {
		Lexem l(name, _pos, symbol, symbolType);
		lexems.push_back(l);
	}
}
void Scanner::printLexems() {
	for(unsigned int i = 0; i < lexems.size()-1; ++i) {
		std::string s = getSymbolTypeStr(lexems[i].type);
		std::cout.width(20);
		std::cout << lexems[i].lexem << " ";
		std::cout.width(3);
		std::cout << lexems[i].pos.row+1 << " ";
		std::cout.width(3);
		std::cout << lexems[i].pos.col+1 << " ";
		std::cout.width(15);
		std::cout  << s << " ";
		std::cout.width(5);
		std::cout << lexems[i].val <<  std::endl;
	}
}

void Scanner::read() {
	std::getline(in, str);
	str += '\n';
	while(!EOFbool && !ERRORbool) {
		nextLexem();
		//getchar();
	}
	Lexem l("", _pos, -10, -10);
	lexems.push_back(l);
}

Scanner::~Scanner() {
	in.close();
}

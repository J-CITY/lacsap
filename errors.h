#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED
#include <string>
#include <cstdlib>

struct TextPos {
public:
	int col = 0;
	int row = 0;
	TextPos(){}
	TextPos(int row, int col) : col(col), row(row){}
	TextPos& operator=(const TextPos& right) {
		if (this == &right) {
			return *this;
		}
		row = right.row;
		col = right.col;
		return *this;
	}
};

enum class Errors {
	ERROR_BAD_SYMBOL             = -1001,
	ERROR_BIG_INT_CONST          = -1003,
	ERROR_IT_IS_NOT_NUMBER       = -1004,
	ERROR_FORGOT_TILDA           = -1008,
	ERROR_BIG_CHAR_CONST         = -1009,
	ERROR_FORGOT_FPAR            = -1002,

	ERROR_UNEXPECT_SYMBOL        = -1010,
	ERROR_SYNTAX_ERROR           = -1011,
	ERROR_CONTINUE               = -1012,
	ERROR_BREAK                  = -1013,
	ERROR_TO_DOWNTO              = -1014,
	ERROR_WRONG_ARGUMENT         = -1015,
	ERROR_EXPRESSION             = -1016,
	ERROR_BAD_TYPE               = -1017,
	ERROR_FORGOT_SQUARE_DRACKETS = -1018,
	ERROR_VAR_ALREADY_EXIST      = -1019,
	ERROR_DIV_BY_ZERO            = -1020
};


class Error {
    std::string errorStr = "";
public:
	Error(){}
    void printError(Errors err, TextPos tp, std::string inputStr) {
		switch(err) {
			case Errors::ERROR_BAD_SYMBOL: {
			errorStr += "LEXER ERROR: bad symbol (";
			break;
		}
		case Errors::ERROR_BIG_INT_CONST: {
			errorStr += "LEXER ERROR: big integer const (";
			break;
		}
		case Errors::ERROR_IT_IS_NOT_NUMBER: {
			errorStr += "LEXER ERROR: it is not number (";
			break;
		}
		case Errors::ERROR_FORGOT_TILDA: {
			errorStr += "LEXER ERROR: expected apostrophe (";
			break;
		}
		case Errors::ERROR_BIG_CHAR_CONST: {
			errorStr += "LEXER ERROR: big char const (";
			break;
		}
		case Errors::ERROR_FORGOT_FPAR: {
			errorStr += "LEXER ERROR: expected brace (";
			break;
		}

		case Errors::ERROR_UNEXPECT_SYMBOL: {
			errorStr += "PARSER ERROR: unexpected symbol (";
			break;
		}
		case Errors::ERROR_SYNTAX_ERROR: {
			errorStr += "PARSER ERROR: syntax error (";
			break;
		}
		case Errors::ERROR_CONTINUE: {
			errorStr += "PARSER ERROR: 'continue' not in a cycle (";
			break;
		}
		case Errors::ERROR_BREAK: {
			errorStr += "PARSER ERROR: 'break' not in a cycle (";
			break;
		}
		case Errors::ERROR_TO_DOWNTO: {
			errorStr += "PARSER ERROR: expect 'to' or 'downto' (";
			break;
		}
		case Errors::ERROR_WRONG_ARGUMENT: {
			errorStr += "PARSER ERROR: wrong argument (";
			break;
		}
		case Errors::ERROR_EXPRESSION: {
			errorStr += "PARSER ERROR: error in expression (";
			break;
		}
		case Errors::ERROR_BAD_TYPE: {
			errorStr += "PARSER ERROR: bad type (";
			break;
		}
		case Errors::ERROR_FORGOT_SQUARE_DRACKETS: {
			errorStr += "PARSER ERROR: forgot [] (";
			break;
		}
		case Errors::ERROR_VAR_ALREADY_EXIST: {
			errorStr += "PARSER ERROR: var already exist (";
			break;
		}
		case Errors::ERROR_DIV_BY_ZERO: {
			errorStr += "ERROR: division by zero.";
			break;
		}

		}
		if((int)tp.col >= 0) {
            errorStr = errorStr + std::to_string((int)err) + ")"
                + " row: " + std::to_string((int)tp.row+1) + " col: " +
                std::to_string((int)tp.col+1) + " " + inputStr + "\n";
		}
		std::cout << errorStr;
		exit((int)err);
	}
};

#endif // ERRORS_H_INCLUDED

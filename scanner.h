#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED
#include <map>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include "errors.h"

enum class Symbols {
	sharp        ,//#
	star         ,//*
	slash        ,// /
	equal        ,//=
	comma        ,//,
	semicolon    ,//;
	colon        ,//:
	point        ,//.
	arrow        ,//^
	at           ,//@
	lpar         ,// (
	rpar         ,// )
	lbracket     ,// [
	rbracket     ,// ]
	flpar        ,// {
	frpar        ,// }
	later        ,//<
	greater      ,//>
	laterequal   ,//<=
	greaterequal ,//=>
	latergreater ,//<>
	plus         ,//+
	minus        ,//-
	assign       ,//:=
	twopoints    ,//..
	ident        ,//ident
	floatc       ,//float const
	intc         ,//int const
	charc        ,//char const
	boolc        ,//bool const
	sys_and      ,
	sys_array    ,
	sys_begin    ,
	sys_break    ,
	sys_case     ,
	sys_const    ,
	sys_continue ,
	sys_div      ,
	sys_do       ,
	sys_downto   ,
	sys_else     ,
	sys_end      ,
	sys_false    ,
	sys_file     ,
	sys_for      ,
	sys_function ,
	sys_goto     ,
	sys_if       ,
	sys_lable    ,
	sys_mod      ,
	sys_nil      ,
	sys_not      ,
	sys_of       ,
	sys_or       ,
	sys_packed   ,
	sys_procedure,
	sys_program  ,
	sys_record   ,
	sys_repeat   ,
	sys_set      ,
	sys_shl      ,
	sys_shr      ,
	sys_string   ,
	sys_then     ,
	sys_to       ,
	sys_true     ,
	sys_type     ,
	sys_unit     ,
	sys_until    ,
	sys_var      ,
	sys_while    ,
	sys_with     ,
	sys_xor
};

const int identifier  = 4000;
const int operation   = 8001;
const int separator   = 8002;
const int keyword     = 8003;
const int directive   = 8004;
const int literalint  = 7000;
const int literalfloat= 6000;
const int literalchar = 5000;


struct Lexem {
	std::string lexem;
	TextPos pos;
	int val;
	int type;

	Lexem(){};

	Lexem(std::string lexem, TextPos pos, int val, int type) :
		                                                     lexem(lexem),
		                                                     pos(pos),
		                                                     val(val),
		                                                     type(type) {}
};

class Scanner {
public:
	unsigned int lexemNow=0;

	Scanner(std::string _filePath);
	Scanner();
	~Scanner();
	void printLexems();
	void printLexems2();
	void read();
	void open(std::string _filePath);
	void nextLexem();

	Lexem getLexem() {
		if(lexemNow < lexems.size()) {
			return lexems[lexemNow];
		}
		return lexems[lexems.size()-1];
	}

	Lexem getNextLexem() {
		if(lexemNow < lexems.size()) {
			lexemNow++;
		}
		if(lexemNow < lexems.size()) {
			return lexems[lexemNow];
		}
		return lexems[lexems.size()-1];
	}
	Lexem getPrevLexem() {
		if(lexemNow != 0) {
			lexemNow--;
		}
		return lexems[lexemNow];
	}

	bool isLastLexem() {
		if(lexemNow >= lexems.size()-1) {
			return true;
		}
		return false;
	}

	std::string getSymbolTypeStr(int type);
	std::map <std::string, int> cods;
	Error error;
private:
	bool EOFbool = false;

	std::vector<Lexem> lexems;
	std::set<std::string> keyWords;
	std::ifstream in;
	std::string filePath;
	std::string str;

	int symbol;
	TextPos textPos;

	TextPos _pos;
	std::string name = "";
	int symbolType;


	bool ERRORbool = false;
	std::string errorStr = "";

	int fparCount = 0;

	unsigned char nextChar();
	void init();
	unsigned char nextLine();
	unsigned char getSymbolType(unsigned char type);
	void readSpace(unsigned char ch);
	void readIdentifier(unsigned char ch);
	void readNumber(unsigned char ch);
};

#endif // SCANNER_H_INCLUDED

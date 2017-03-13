#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

enum Symbols {
  sharp        ,//#
  star         ,//*
  slash        ,// /
  equal        ,//=
  comma        ,//,
  semicolon    ,//;
  colon        ,//:
  point        ,//.
  arrow        ,//^
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

//ERRORS
enum errors {
  ERROR_BAD_SYMBOL          = -1001,
  ERROR_BIG_INT_CONST       = -1003,
  ERROR_IT_IS_NOT_NUMBER    = -1004,
  ERROR_FORGOT_TILDA        = -1008,
  ERROR_BIG_CHAR_CONST      = -1009,
  ERROR_FORGOT_FPAR         = -1002
};

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

struct Lexem {
    std::string lexem;
    TextPos pos;
    int val;
    int type;

    Lexem(std::string lexem, TextPos pos, int val, int type) :
                                                            lexem(lexem),
                                                            pos(pos),
                                                            val(val),
                                                            type(type) {}
};

class Scanner {
public:
    Scanner(std::string _filePath);
    ~Scanner();
    void printLexems();
    void printLexems2();
    void test();
    void read();
    void error(int err);
    void nextLexem();
private:
    bool EOFbool = false;

    std::vector<Lexem> lexems;
    std::map <std::string, int> cods;
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

    std::string getSymbolTypeStr(int type);
    unsigned char nextChar();
    unsigned char nextLine();
    unsigned char getSymbolType(unsigned char type);
    void readSpace(unsigned char ch);
    void readIdentifier(unsigned char ch);
    void readNumber(unsigned char ch);
};



#endif // SCANNER_H_INCLUDED

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

#define LETER 0
#define NUMBER 1

#define star          0//*
#define slash         1// /
#define equal         2//=
#define comma         3//,
#define semicolon     4//;
#define colon         5//:
#define point         6//.
#define arrow         7//^
#define lpar          8// (
#define rpar          9// )
#define lbracket     10// [
#define rbracket     11// ]
#define flpar        12// {
#define frpar        13// }
#define later        14//<
#define greater      15//>
#define laterequal   16//<=
#define greaterequal 17//=>
#define latergreater 18//<>
#define plus         19//+
#define minus        20//-
#define lcoment      21// {
#define rcoment      22// }
#define assign       23//:=
#define twopoints    24//..

#define direct        3000
#define ident         4000//ident
#define floatc        6000//float const
#define intc          7000//int const
#define charc         5000//char const

#define sys_case      301
#define sys_else      302
#define sys_file      303
#define sys_goto      304
#define sys_then      305
#define sys_type      306
#define sys_unit      307
#define sys_with      309
#define sys_do        308
#define sys_if        310
#define sys_of        311
#define sys_or        312
#define sys_in        313
#define sys_to        314
#define sys_end       315
#define sys_var       316
#define sys_div       317
#define sys_and       318
#define sys_not       319
#define sys_for       320
#define sys_mod       321
#define sys_nil       322
#define sys_set       323
#define sys_begin     324
#define sys_while     325
#define sys_array     326
#define sys_const     327
#define sys_lable     328
#define sys_until     329
#define sys_downto    330
#define sys_packed    331
#define sys_record    332
#define sys_repeat    333
#define sys_program   334
#define sys_function  335
#define sys_procedure 336

#define identifier   4000
#define operation    8001
#define separator    8002
#define keyword      8003
#define directive    8004
#define literalint   7000
#define literalfloat 6000
#define literalchar  5000

//ERRORS
#define ERROR_BAD_SYMBOL       -1001
#define ERROR_CHAR_CONST       -1002
#define ERROR_BIG_INT_CONST    -1003
#define ERROR_IT_IS_NOT_NUMBER -1004
#define ERROR_FORGOT_PAR        -1005
#define ERROR_FORGOT_BASKETPAR  -1006
#define ERROR_FORGOT_END_OR_BEGIN  -1007

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
//#define err 0
//#define val -1
class Scanner {
public:
    /*const static char STAT [5][9] = {
        //  *  /   =   ,   ;   :   .   (   )   [   ]   <   >   <>  <=  >=  +   -   :=  .. ide  con case else file goto then type until with do   if   of  or  in  to  end var div  and  not  for  mod  nil  set  begin while array const  lable  until downto packed record repeat program function procedure
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  *
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  /
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  =
          {err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,val,err,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  ,
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,val,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  ;
          {err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  :
          {err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,val,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  .
        //  (
        //  )
        //  [
        //  ]
        //  <
        //  >
        //  <>
        //  <=
        //  >=
        //  +
        //  -
        //  :=
        //  ..
        //  ident
        //  const
        //  space
        //  case
        //  else
        //  file
        //  goto
        //  then
        //  type
        //  until
        //  with
        //  do
        //  if
        //  of
        //  or
        //  in
        //  to
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  end
        //  var
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  div
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  and
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  not
        //  for
          {err,err,err,err,err,err,err,err,val,err,val,err,err,err,err,err,err,err,err,err,val,val,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  mod
        //  nil
        //  set
        //  begin
        //  while
        //  *  /   =   ,   ;   :   .   (   )   [   ]   <   >   <>  <=  >=  +   -   :=  .. ide  con case else file goto then type until with do   if   of  or  in  to  end var div  and  not  for  mod  nil  set  begin while array const  lable  until downto packed record repeat program function procedure
          {err,err,err,err,err,val,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err,err, err, err, err, err, err, err,  err, err, err, err,err,err,err,err,err,err, err, err, err, err, err, err, err,  err,  err,  err,   err,   err,  err,   err,   err,   err,   err,    err,     err},//  array
        //  const
        //  lable
        //  until
        //  downto
        //  packed
        //  record
        //  repeat
        //  program
        //  function
        //  procedure
    };*/
    bool EOFbool = false;

    std::map <std::string, int> cods = {
      {       "*", star           },
      {       "/", slash          },
      {       "=", equal          },
      {       ",", comma          },
      {       ";", semicolon      },
      {       ":", colon          },
      {       ".", point          },
      {       "^", arrow          },
      {       "(", lpar           },
      {       ")", rpar           },
      {       "[", lbracket       },
      {       "]", rbracket       },
      {       "{", flpar          },
      {       "}", frpar          },
      {       "<", later          },
      {       ">", greater        },
      {       "<=", laterequal    },
      {       ">=", greaterequal  },
      {       "<>", latergreater  },
      {        "+", plus          },
      {        "-", minus         },
      {       "{*", lcoment       },
      {       "*}", rcoment       },
      {       ":=", assign        },
      {       "..", twopoints     },
      {   "direct", direct        },
      {    "ident", ident         },
      {    "float", floatc        },
      {      "int", intc          },
      {     "char", charc         },
      {     "case", sys_case      },
      {     "else", sys_else      },
      {     "file", sys_file      },
      {     "goto", sys_goto      },
      {     "then", sys_then      },
      {     "type", sys_type      },
      {    "until", sys_unit      },
      {     "with", sys_with      },
      {       "do", sys_do        },
      {       "if", sys_if        },
      {       "of", sys_of        },
      {       "or", sys_or        },
      {       "in", sys_in        },
      {       "to", sys_to        },
      {      "end", sys_end       },
      {      "var", sys_var       },
      {      "div", sys_div       },
      {      "and", sys_and       },
      {      "not", sys_not       },
      {      "for", sys_for       },
      {      "mod", sys_mod       },
      {      "nil", sys_nil       },
      {      "set", sys_set       },
      {    "begin", sys_begin     },
      {    "while", sys_while     },
      {    "array", sys_array     },
      {    "const", sys_const     },
      {    "lable", sys_lable     },
      {    "until", sys_until     },
      {   "downto", sys_downto    },
      {   "packed", sys_packed    },
      {   "record", sys_record    },
      {   "repeat", sys_repeat    },
      {  "program", sys_program   },
      { "function", sys_function  },
      {"procedure", sys_procedure },
    };
    std::vector<Lexem> lexems;

    int literalsInt = 0;
    int literalsFloat = 0;
    int literalsChar = 0;
    int identifiers = 0;

    std::set<std::string> keyWords;

    std::ifstream in;

    std::string str;
    std::string filePath;

    int symbol;
    TextPos textPos;

    int intConst;
    float floatConst;
    char charConst;
    bool ERRORbool = false;

    Scanner(std::string _filePath);
    std::string getSymbolType(int type);
    unsigned char nextChar();
    void readSpace(unsigned char ch);
    unsigned char getType(unsigned char type);
    void readWord(unsigned char ch);
    void readNumber(unsigned char ch);

    TextPos _pos;
    std::string name = "";
    int symbolType;

    int parCount = 0;
    int bascketCount = 0;
    int fparCount = 0;
    int beginEndCount = 0;

    std::string errorStr = "";
    void error(int err);
    void nextLexem();
    void printLexems();
    void printLexems2();
    void test();
    void read();
    ~Scanner();
};



#endif // SCANNER_H_INCLUDED

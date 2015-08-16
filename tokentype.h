/* tokentype.h
   Alistair Campbell
   Fall 2013
   CS 310 Compilers

   A "mapping" between token types and their names
   In the future, this file, or one like it, will get 
   Generated automatically.

   Also contains the definition of a Token for purposes of 
   out lexical analysis phase.
*/

#ifndef SCANNER
#define SCANNER

#include <iostream>
#include <cstdlib>

using namespace std;

/* Constants for all 51 types of tokens in Decaf */
enum TokenType {
  /* The 22 keywords */
  T_Void = 257, T_Int, T_Double, T_Bool,
  T_String, T_Class, T_Interface, T_Null,
  T_This, T_Extends, T_Implements, T_For,
  T_While, T_If, T_Else, T_Return,
  T_Break, T_New, T_NewArray, T_Print,
  T_ReadInteger, T_ReadLine,
  /* 1 Identifier */
  T_Identifier,
  /* The 4 kinds of constants (literals) */
  T_IntConstant, T_BoolConstant, T_DoubleConstant, T_StringConstant,
  /* The 24 other tokens 
     + - * / % < <= > >= = == != && || ! ; , . [ ] ( ) { } */
  T_Plus, T_Minus, T_Times, T_Div,
  T_Mod, T_Less, T_LessEqual, T_Greater,        
  T_GreaterEqual, T_Assign, T_Equal, T_NotEqual,
  T_And, T_Or, T_Not, T_Semicolon,
  T_Comma, T_Dot, T_LBracket, T_RBracket,
  T_LParen, T_RParen, T_LBrace, T_RBrace,
  T_TypeIdentifier
};

/* And their associated names.  Which we will use solely to verify our
   scanner is working. */

static const char *TokenNames[] = {
  /* The 22 keywords */
  "T_Void", "T_Int", "T_Double", "T_Bool",
  "T_String", "T_Class", "T_Interface", "T_Null", 
  "T_This", "T_Extends", "T_Implements", "T_For",
  "T_While", "T_If", "T_Else", "T_Return", 
  "T_Break", "T_New", "T_NewArray", "T_Print",
  "T_ReadInteger", "T_ReadLine",
  /* 1 Identifier */
  "T_Identifier",
  /* The 4 kinds of constants (literals) */
  "T_IntConstant", "T_BoolConstant", "T_DoubleConstant", "T_StringConstant",
  /* The 24 other tokens 
     + - * / % < <= > >= = == != && || ! ; , . [ ] ( ) { } */
  "T_Plus", "T_Minus", "T_Times", "T_Div", 
  "T_Mod", "T_Less", "T_LessEqual", "T_Greater",        
  "T_GreaterEqual", "T_Assign", "T_Equal", "T_NotEqual",
  "T_And", "T_Or", "T_Not", "T_Semicolon",
  "T_Comma", "T_Dot", "T_LBracket", "T_RBracket",
  "T_LParen", "T_RParen", "T_LBrace", "T_RBrace",
  "T_TypeIdentifier"
};

/* The struct for tokens for the Decaf lexer assignment */

struct Token {
  int type;
  string text;
  int line;
  Token() {} // leave uninitialized
  Token(TokenType type, string text, int line) : type(type), text(text), line(line) {}
  string toString() {
    // convert line to a C string
    char lineStr[200];
    sprintf(lineStr,"%d",line);
    return string(TokenNames[type-257]) + '(' + text + ',' + lineStr + ')';
  }
  void print() {
    cout << toString() << endl;
  }
};
 
#endif




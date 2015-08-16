#include <vector>
#include <stack>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "tokentype.h" 

  /* we are building parse trees */
#define YYSTYPE ParseTree *

  /* prototype for yyerror, needed on my linux laptop */
int yyerror(const char * s);

using namespace std;

enum PTtype {TERMINAL, NONTERMINAL};

struct Symtab;

struct ParseTree {
  Symtab *symtab; // symbol table
  PTtype type;
  string stype; // semantic type
  string description;
  Token * token;
  vector<ParseTree *> children;
  ParseTree(string description);
  ParseTree(Token * tokp);
  void addChild(ParseTree * tree);
  string toString();

};

void traverseTree(ParseTree * tree, int depth, int seq);




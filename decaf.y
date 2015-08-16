%token Y_Void
%token Y_Int
%token Y_Double
%token Y_Bool
%token Y_String
%token Y_Class
%token Y_Interface
%token Y_Null
%token Y_This
%token Y_Extends
%token Y_Implements
%token Y_For
%token Y_While
%token Y_If
%token Y_Else
%token Y_Return
%token Y_Break
%token Y_New
%token Y_NewArray
%token Y_Print
%token Y_ReadInteger
%token Y_ReadLine
%token Y_Identifier
%token Y_IntConstant
%token Y_BoolConstant
%token Y_DoubleConstant
%token Y_StringConstant
%token Y_Plus
%token Y_Minus
%token Y_Times
%token Y_Div
%token Y_Mod
%token Y_Less
%token Y_LessEqual
%token Y_Greater
%token Y_GreaterEqual
%token Y_Assign
%token Y_Equal
%token Y_NotEqual
%token Y_And
%token Y_Or
%token Y_Not
%token Y_Semicolon
%token Y_Comma
%token Y_Dot
%token Y_LBracket
%token Y_RBracket
%token Y_LParen
%token Y_RParen
%token Y_LBrace
%token Y_RBrace
%token Y_TypeIdentifier

%{
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <stack>
#include "tokentype.h" 
#include "parsetree.h"

  /* we are building parse trees */
#define YYSTYPE ParseTree *

using namespace std;

 extern Token *myTok;
 extern int yylineno;
 int yylex();

 stack<Token*> opStack;

 ParseTree* binaryOpChildBuilder(ParseTree* expr1, ParseTree* expr2)
 {
   ParseTree *result = new ParseTree("bExpr");
   result->addChild(expr1);
   result->addChild(new ParseTree(opStack.top()));
   opStack.pop();
   result->addChild(expr2);
   return result;
 }
 
 ParseTree* unaryOpChildBuilder(ParseTree* expr1)
 {
   ParseTree *result = new ParseTree("uExpr");
   result->addChild(new ParseTree(opStack.top()));
   opStack.pop();
   result->addChild(expr1);
   return result;
 }

 extern ParseTree *top;
  %}

%nonassoc '='
%left Y_Or
%left Y_And
%nonassoc Y_Equal Y_NotEqual
%nonassoc '<' '>' Y_LessEqual Y_GreaterEqual
%left '+' '-'
%left '*' '/' '%'
%right '!'
%left '.' '['


%start program   // for testing purposes

%%

program: sdecl { 
  /* ParseTree* result = new ParseTree("program"); */
  /* result->addChild($1); */
  top=$$= $1; 
 };  

decl: variabledecl { 
  //ParseTree* result = new ParseTree("vardecl");
  //result->addChild($1);
  top=$$=$1; 
 }

| funcdecl {
  //ParseTree* result = new ParseTree("decl");
  //result->addChild($1);
  top=$$= $1;
 }

| classdecl {
  /* ParseTree* result = new ParseTree("decl"); */
  /* result->addChild($1); */
  top=$$= $1;
  }

| interfacedecl {
  /* ParseTree* result = new ParseTree("decl"); */
  /* result->addChild($1); */
  top=$$= $1;
  };

sdecl: sdecl decl { 
  $1->addChild($2);
  top=$$= $1; 
 }

|decl {
  ParseTree* result = new ParseTree("program");
  result->addChild($1);
  top=$$= result;
  };



 ////////////////////////// VARIABLE DECL /////////////////
variabledecl: variable ';' { 
  ParseTree* result = new ParseTree("variabledecl");
  result->addChild($1);
  top=$$= result; 
 };

/////////////////////// VARIABLE ////////////////////////
variable: type  ident {
  ParseTree* result = new ParseTree("variable");
  result->addChild($1);
  result->addChild($2);
  top=$$= result;   
 };


////////////////////////// IDENT ////////////////////////

ident: Y_Identifier {
  ParseTree* result = new ParseTree(myTok);
  top=$$= result;   
 };
  




/////////////////////////// TYPE ///////////////////////////
type: tint { 
  ParseTree* result = new ParseTree("TYPE_Integer");
  result->addChild($1);
  top=$$= result;   
 }

| tdouble { 
  ParseTree* result = new ParseTree("TYPE_Double");
  result->addChild($1);
  top=$$= result;  
  }

| tbool { 
  ParseTree* result = new ParseTree("TYPE_Bool");
  result->addChild($1);
  top=$$= result;  
 }

| tstring { 
  ParseTree* result = new ParseTree("TYPE_String");
  result->addChild($1);
  top=$$= result;  
 }

| tident { 
  ParseTree* result = new ParseTree("TYPE_userdef");
  result->addChild($1);
  top=$$= result; 
  };

| type '[' ']' {
  ParseTree* result = new ParseTree("TYPE_array");
  result->addChild($1);
  top=$$= result;
  };

tint: Y_Int {top=$$=new ParseTree(myTok); };

tdouble: Y_Double { top=$$=new ParseTree(myTok); };

tbool: Y_Bool { top=$$=new ParseTree(myTok); };

tstring: Y_String { top=$$=new ParseTree(myTok); };

tident : Y_TypeIdentifier { top=$$=new ParseTree(myTok); };


////////////////// FUNCTION DECL ////////////////////////
funcdecl: type ident '(' nformals ')' stmtblock {
  ParseTree* result = new ParseTree("funcdecl");
  result->addChild($1);
  result->addChild($2);
  result->addChild($4);
  result->addChild($6);
  top=$$= result;   
 }

| voidDecl ident '(' nformals ')' stmtblock { 
  ParseTree* result = new ParseTree("funcdecl");
  result->addChild($1);
  result->addChild($2);
  result->addChild($4);
  result->addChild($6);
  top=$$= result;   
 };

///////////////////// VOID /////////////////////
voidDecl: Y_Void {
  ParseTree* result = new ParseTree("TYPE_Void");
  result->addChild(new ParseTree(myTok));
  top=$$= result;  
  };


/////////////////// FORMALS ////////////////////

nformals: formals {
  ParseTree* result = new ParseTree("formal");
  result->addChild($1);
  top=$$= $1;   
 }
|  {top=$$= NULL; };


formals: formals ',' variable {
  $1->addChild($3);
  top=$$= $1;   
 }
| variable  { 
  ParseTree* result = new ParseTree("formal");
  result->addChild($1);
  top=$$= result;
};

nextends:  Y_Extends {opStack.push(myTok);} ident {
  ParseTree* result = new ParseTree("extends");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($3);
  top=$$= result;
					    }

| { top=$$= NULL;};

nimplements:  Y_Implements {opStack.push(myTok);} idents {
  ParseTree* result = new ParseTree("implements");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($3);
  top=$$= result;
					    }

| { top=$$= NULL;};

/////////////////// CLASS DECL ///////////////////

classdecl: Y_Class {opStack.push(myTok);} ident nextends nimplements '{' nfields '}' {
  ParseTree* result = new ParseTree("classdecl");
  ParseTree* tempT = new ParseTree(opStack.top()); opStack.pop();
  

  result->addChild(tempT);
  result->addChild($3);
  result->addChild($4);
  result->addChild($5);
  result->addChild($7); // idents
  top=$$= result;   							     
};

idents: idents ',' ident {
  $1->addChild($3);
  top=$$= $1;
 }

| ident {
  ParseTree* result = new ParseTree("nident");
  result->addChild($1);
  top=$$=result;
  };
  


nfields: fields {
  /* ParseTree* result = new ParseTree("nfields"); */
  /* result->addChild($1); */
  top=$$= $1;   							     
 }
|   { top=$$= NULL; };

fields: fields field {
  $1->addChild($2);
  top=$$= $1;
 }
| field {
  ParseTree* result = new ParseTree("fields");
  result->addChild($1);
  top=$$= result; 
  };

///////////////////////////////////// FIELD ////////////////////////

field: variabledecl {
  /* ParseTree* result = new ParseTree("field"); */
  /* result->addChild($1); */
  top=$$= $1;   							     
 }
| funcdecl {
  /* ParseTree* result = new ParseTree("field"); */
  /* result->addChild($1); */
  top=$$= $1;   							     
  };


///////////////////////////// INTERFACE DECL ///////////////////////////
interfacedecl: Y_Interface {opStack.push(myTok);} ident '{' nprototypes '}' {
  ParseTree* result = new ParseTree("interface");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();//Interface);
  result->addChild($3);
  result->addChild($5);

  top=$$= result;   			
};

nprototypes: prototypes {
  /* ParseTree* result = new ParseTree("prototype"); */
  /* result->addChild($1); */
  top=$$= $1;   							     
 }
|  { top=$$= NULL; };

prototypes: prototypes prototype {
  /* ParseTree* result = new ParseTree("nprototypes"); */
  /* result->addChild($1); */
  $1->addChild($2);
  top=$$= $1;
 }
| prototype {
  ParseTree* result = new ParseTree("prototypes");
  result->addChild($1);
  top=$$= result;
  };


////////////////////////// PROTOTYPE /////////////////////////

prototype: type Y_Identifier {opStack.push(myTok);} '(' nformals ')' ';' {
  ParseTree* result = new ParseTree("prototype");
  result->addChild($1);
  result->addChild(new ParseTree(opStack.top())); opStack.pop();//Identifier
  result->addChild($5);
  top=$$= result;   							     
						    }

| voidDecl Y_Identifier {opStack.push(myTok);} '(' nformals ')' ';' {
  ParseTree* result = new ParseTree("prototype");
  result->addChild($1);
  result->addChild(new ParseTree(opStack.top())); opStack.pop();//Void
  result->addChild($5);
  top=$$= result;   							     
								   };



///////////////////////// STMT BLOCK ////////////////////

/* nformals: formals { */
/*   //ParseTree* result = new ParseTree("formal"); */
/*   //result->addChild($1); */
/*   top=$$= $1;    */
/*  } */
/* |  {top=$$= NULL; }; */


/* formals: formals ',' variable { */
/*   $1->addChild($3); */
/*   top=$$= $1;    */
/*  } */
/* | variable  {  */
/*   ParseTree* result = new ParseTree("formal"); */
/*   result->addChild($1); */
/*   top=$$= result; */
/* }; */


stmtblock: '{' nvardecls nstmts '}' { 
  ParseTree* result = new ParseTree("stmtblock");
  result->addChild($2);
  result->addChild($3);
  top=$$= result;
 };

nvardecls: vardecls { 
  /* ParseTree* result = new ParseTree("nvardecls"); */
  /* result->addChild($1); */
  top=$$= $1;
 }
|  { top=$$= NULL; };

vardecls: vardecls variabledecl {
  $1->addChild($2);
  top=$$=$1;
 }
| variabledecl {
  ParseTree* result = new ParseTree("vardecls");
  result->addChild($1);
  top=$$= result;
  };





//////////////////// STATEMENTS //////////////
  
nstmts: stmts {
  /* ParseTree* result = new ParseTree("nstmts"); */
  /* result->addChild($1); */
  top=$$= $1;
 } 

|  {  top=$$= NULL; };


stmt: matched_stmt {
  /* ParseTree* result = new ParseTree("stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| open_stmt {
  /* ParseTree* result = new ParseTree("stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  };


stmts: stmt {
  ParseTree* result = new ParseTree("stmts");
  result->addChild($1);
  top=$$=result;
 }

| stmts stmt {
  
  $1->addChild($2);
  top=$$=$1;
 };


open_stmt: open_if {
  /* ParseTree* result = new ParseTree("open_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| open_while {
  /* ParseTree* result = new ParseTree("open_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| open_for {
  /* ParseTree* result = new ParseTree("open_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  };


matched_stmt: matched_if {
  /* ParseTree* result = new ParseTree("matched_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| matched_while {
  /* ParseTree* result = new ParseTree("matched_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| matched_for {
  /* ParseTree* result = new ParseTree("matched_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| other_stmt {
  /* ParseTree* result = new ParseTree("matched_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  };


other_stmt: nexpr ';' {
  /* ParseTree* result = new ParseTree("other_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
 } 

| print_stmt {
  /* ParseTree* result = new ParseTree("other_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| return_stmt {
  /* ParseTree* result = new ParseTree("other_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| break_stmt {
  /* ParseTree* result = new ParseTree("other_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  }

| stmtblock {
  /* ParseTree* result = new ParseTree("other_stmt"); */
  /* result->addChild($1); */
  top=$$=$1;
  };


yelse: Y_Else { 
  ParseTree* result = new ParseTree(myTok);
  top=$$= result;
      };


////////////////////// IF STATEMENTS //////////////

common_if: Y_If {opStack.push(myTok); } '(' expr ')' { 
  ParseTree* result = new ParseTree("common_if");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  top=$$=result;
 }

matched_if: common_if matched_stmt yelse matched_stmt {
  ParseTree* result = new ParseTree("matched_if");
  result->addChild($1);
  result->addChild($2);
  result->addChild($3);
  result->addChild($4);
  top=$$=result;
}



open_if: common_if stmt { 
  ParseTree* result = new ParseTree("open_if");
  result->addChild($1);
  result->addChild($2);
  top=$$=result; 
 }

| common_if matched_stmt yelse open_stmt {
  ParseTree* result = new ParseTree("open_if");
  result->addChild($1);
  result->addChild($2);
  result->addChild($3);
  result->addChild($4);
  top=$$=result;
 };


////////////////// WHILE STATEMENT ///////////////

common_while: Y_While {opStack.push(myTok);} '(' expr ')' {
  ParseTree* result = new ParseTree("common_while");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  top=$$=result;
 };

matched_while: common_while matched_stmt  {
  ParseTree* result = new ParseTree("matched_while");
  result->addChild($1);
  result->addChild($2);
  top=$$=result; 
 };

open_while: common_while open_stmt  {  
  ParseTree* result = new ParseTree("open_while");
  result->addChild($1);
  result->addChild($2);
  top=$$=result; 
 };



/////////////// FOR STATEMENT ////////////////

common_for: Y_For {opStack.push(myTok); } '(' nexpr ';' expr ';' nexpr ')' {
  ParseTree* result = new ParseTree("common_for");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  result->addChild($6);
  result->addChild($8);
  top=$$=result;
};

matched_for: common_for matched_stmt  { 
  ParseTree* result = new ParseTree("matched_for");
  result->addChild($1);
  result->addChild($2);
  top=$$=result; 
};

open_for: common_for open_stmt  {
  ParseTree* result = new ParseTree("open_for");
  result->addChild($1);
  result->addChild($2);
  top=$$=result; 
};


/////////////// RETURN STATEMENT //////////////

return_stmt: Y_Return {opStack.push(myTok);} nexpr ';' {
  ParseTree* result = new ParseTree("return");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($3);
  top=$$= result; 
};

/////////// BREAK STATEMENT ////////////////

break_stmt: Y_Break {opStack.push(myTok);} ';' { 
  ParseTree* result = new ParseTree("break");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  top=$$= result;
 };


//////////// PRINT STATEMENT /////////////////////

print_stmt: Y_Print {opStack.push(myTok);} '(' exprs ')'  { 
  ParseTree* result = new ParseTree("print");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  //result->addChild($5);
  top=$$= result; 
};

//////////////// P EXPR ////////////////////

exprs: exprs ',' expr { 
  $1->addChild($3);
  top=$$ = $1;
  }  

| expr { 
  ParseTree* result = new ParseTree("pExpr"); 
  result->addChild($1); 
  top=$$= result;
  };

//////////////////////////////// NEXPR ///////////////

/* nexprs: exprs {  */
/*   ParseTree* result = new ParseTree("nExpr"); */
/*   result->addChild($1); */
/*   top=$$ = $1;  */
/*  } */

/* |  {top=$$= NULL;  }; */

nexpr: expr { 
  ParseTree* result = new ParseTree("nExpr");
  result->addChild($1);
  top=$$ = $1; 
 }

|  {top=$$= NULL;  };


//////////////////////////////////// EXPR ///////////////
expr: lvalue '=' {opStack.push(myTok);} expr { 
  ParseTree* result = new ParseTree("bExpr");
  result->addChild($1);
  result->addChild(new ParseTree(opStack.top()));
  opStack.pop();
  result->addChild($4);
  top=$$= result;
}
    
| constant { 
  /* ParseTree* result = new ParseTree("expr"); */
  /* result->addChild($1); */
  top=$$= $1;  
 }

| lvalue { 
  top=$$= $1;  
} 

| Y_This { 
  ParseTree *result = new ParseTree("expr"); 
  result->addChild(new ParseTree(myTok)); 
  top=$$ = result; 
}

| call {
  /* ParseTree *result = new ParseTree("expr");  */
  /* result->addChild($1);  */
  top=$$ = $1; 
  }
  

| '(' expr ')' { top = $$ = $2; } // always use top = $$ = ...

| expr '+' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr '-' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr '*' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr '/' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr '%' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| '-' {opStack.push(myTok);} expr {top=$$= unaryOpChildBuilder($3);}

| expr '<' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_LessEqual {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr '>' {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_GreaterEqual {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_Equal {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_NotEqual {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_And {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| expr Y_Or {opStack.push(myTok);} expr {top=$$= binaryOpChildBuilder($1,$4);}

| '!' {opStack.push(myTok);} expr {top=$$= unaryOpChildBuilder($3);}

| Y_ReadInteger {opStack.push(myTok);} '(' ')' {  
  ParseTree* result = new ParseTree("readinteger");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  top=$$= result;
}

| Y_ReadLine {opStack.push(myTok);} '(' ')' {    
  ParseTree* result = new ParseTree("readline");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  top=$$= result; 
}
   
| Y_New {opStack.push(myTok);} '(' ident ')' { 
  ParseTree* result = new ParseTree("new");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  top=$$= result;  
  }

| Y_NewArray {opStack.push(myTok);} '(' expr ',' type ')' {
  ParseTree* result = new ParseTree("newarray");
  result->addChild(new ParseTree(opStack.top())); opStack.pop();
  result->addChild($4);
  result->addChild($6);
  top=$$= result; 
  };


///////////////////////// L VALUE ////////////////////////////////

lvalue: ident { 
  /* ParseTree *result = new ParseTree("lvalue2");  */
  /* result->addChild($1);  */
  top=$$ = $1;
 }

| expr '.' ident {
  ParseTree *result = new ParseTree("varAccess"); 
  result->addChild($1);
  result->addChild($3); 
  top=$$ = result; 
  }

| expr '[' expr ']' {
  ParseTree *result = new ParseTree("arrayRef");
  result->addChild($1);
  result->addChild($3);
  top=$$ = result;
  };

/////////////////////// CALL //////////////////
call: ident '(' nactuals ')' {
  ParseTree *result = new ParseTree("call"); 
  result->addChild($1); 
  result->addChild($3); 
  top=$$ = result;}

| expr '.' ident '(' nactuals ')' {
  ParseTree *result = new ParseTree("methodcall"); 
  result->addChild($1);
  result->addChild($3);
  result->addChild($5); 
  top=$$ = result; 
  };

nactuals: actuals {
  top=$$=$1;
 }
| { top=$$=NULL;};
  
actuals: actuals ',' expr {
  $1->addChild($3);
  top=$$=$1;
 }
| expr {
  ParseTree *result = new ParseTree("actuals"); 
  result->addChild($1);
  top=$$=result;
  };

///////////////////// CONSTANT //////////////////////////

constant: Y_IntConstant {
  ParseTree *result = new ParseTree("constant_Integer"); 
  result->addChild(new ParseTree(myTok)); 
  top=$$ = result; 
 }
| Y_DoubleConstant {
  ParseTree *result = new ParseTree("constant_Double"); 
  result->addChild(new ParseTree(myTok)); 
  top=$$ = result; 
  }
| Y_BoolConstant {
  ParseTree *result = new ParseTree("constant_Bool"); 
  result->addChild(new ParseTree(myTok)); 
  top=$$ = result; 
  }
| Y_StringConstant {
  ParseTree *result = new ParseTree("constant_String"); 
  result->addChild(new ParseTree(myTok)); 
  top=$$ = result; 
  }
| Y_Null {  };// non terminal node with no children


%%

int yyerror(const char * s)
{
  fprintf(stderr, "%s\n", s);
  return 0;
}

extern FILE * yyin; 

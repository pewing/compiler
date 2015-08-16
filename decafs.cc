//#include "parsetree.h"

//#include "semantics.h"
#include "decafs.h" 


extern int yyparse(void);
extern ParseTree * top;
extern S_interface* currentInterface;
extern S_class* currentClass;
extern S_function* currentFunction;
extern Symtab * currentScope;
extern vector<sstriple> subsuprels;

extern int varcount;
//extern bool interfaceCheck;

void pass1(ParseTree * tree)
{
  //cout<<"start of pass 1"<<endl;
  //cout<<"tree->description = "<<tree->description<<endl;

  // if (tree->description=="program") {
  //   //tree = tree->children[0];
  //   for (size_t i = 0; i < tree->children.size();i++) {
  //     pass1(tree->children[i]);
  //   }
  // }
  // if (tree->description=="sdecl")
  //   tree=tree->children[0];
  
  //bool open = false;
  if (!tree) return;
  if (tree->type==TERMINAL) return;
  // must be nonterminal
  // if (tree->description=="bExpr")
  //   cout << "binop" << endl;
  // if (tree->description=="vardecls")
  //   cout << "vardecls" << endl;
  if (tree->description=="stmtblock") {
    makeStmtBlock(tree);
  }
  else if (tree->description == "variabledecl")
    makeVariable(tree);

  else if (tree->description == "funcdecl"){
    //cout<<"In make function"<<endl;
    makeFunction(tree);}

  else if (tree->description == "classdecl") {
    //cout<<"In make class"<<endl;
    makeClass(tree);}
  
  else if (tree->description == "interface") 
    makeInterface(tree);
  
  else if (tree->description=="prototype")//||tree->description=="prototypes")
    makePrototype(tree);  
  else if (tree->type==NONTERMINAL) 
    for (size_t i = 0; i < tree->children.size();i++) {
      pass1(tree->children[i]);
    }
  
}

void pass1_5(ParseTree * tree) {
  for (size_t i = 0; i<subsuprels.size();i++) {
    //cout << "i = " << i << endl;
    sstriple temptriple = subsuprels[i];
    S_class * sub = temptriple.first;
    S_class * super = temptriple.second;
    int linenumber = temptriple.third;
    //cout << "i2 = " << i << endl;
    //S_class * subclass = (S_class *)(currentScope->lookup(sub));
    //semantics * thing = currentScope->lookup(super);

    if (!super)
      semantic_error("Class does not have given super class to implement",linenumber);
    if (super->kind() != "S_class")
      semantic_error("Object implemented not a class",linenumber);
    //cout << "i3 = " << i << endl;
    sub->parentClass = super;
    if (class_occurs(sub, super))
      semantic_error("Class cycle detected in class hierarchy on line", linenumber); 
    //cout << "i4 = " << i << endl;
  }
}


void pass2(ParseTree * tree) {
  if (!tree)
    return;
  if (tree->symtab) {
    //cout << "                                Tree has symtab " <<tree->description<< endl;
    currentScope = tree->symtab; 
    //cout<<"scope from tree "<<endl;currentScope->parent->print(cout);  
  }
  if (tree->type==TERMINAL) return;
  // if (tree->type==NONTERMINAL)
  //   cout << "tree description = "<< tree->description<<endl;
  


  
  if (tree->description == "variabledecl"||tree->description == "variable")
    pass2vardecl(tree);
  else if (tree->description == "classdecl") {
    //cout << "class name = "<< tree->children[1]->token->text<<endl;
    pass2classdecl(tree);}
  
  else if (tree->description == "funcdecl") {
    //cout << "func name = "<< tree->children[1]->token->text<<endl; 
    pass2funcdecl(tree);}
  
  else if (tree->description == "fields") {
    //cout << "field"<<endl; 
    pass2fielddecl(tree);
  }
  
  
  else 
    for (size_t i=0;i<tree->children.size();i++) {
      pass2(tree->children[i]);}
}






void pass3(ParseTree * tree) {
  //cout << "||"<<tree->description<< "||"<<tree->line<<endl;
  //cout<<"yo"<<endl;    
  if (!tree) 
    return; 
  if (tree->symtab) 
    currentScope = tree->symtab;

  if (tree->type == TERMINAL) {
    //cout << tree->token->text<<endl;
    pass3token(tree);
    // if (tree->token->type == T_Identifier){
    //cout << tree->token->text<<endl;
    //   string varName = tree->token->text;
    //   if (currentInterface)
    // 	cout<<"YES"<<endl;
    //   //cout << "hey: "<<varName<<endl;
    //   if (!currentScope->lookup(varName)&&!currentInterface){
    // 	//cout<<"scope outside main "<<endl;currentScope->print(cout); 
    // 	// if (interfaceCheck)
    // 	//   cout<<"hey"<<endl;
    // 	// else
    // 	semantic_error(varName + " used but never defined in this scope", tree->token->line); }
    //   // if (interfaceCheck)
    //   // 	cout<<"INTERFACE CHECKED"<<endl;
    // }
  }
    
    
    
  else if (tree->description == "classdecl")
    pass3classdecl(tree);
  
  else if (tree->description == "funcdecl") {
    //cout << "FUNCDECL PASS 3" << endl;
    pass3funcdecl(tree); 
    //cout << "AFTER FUNCDECL PASS 3" << endl;
  }
  
  
  else if (tree->description == "bExpr"){
    
    //cout << "BINOP PASS 3" << endl;
    pass3binop(tree);
    //cout << "AFTER BINOP PASS 3" << endl;
  }
  
  else if (tree->description == "pExpr"){
    pass3pexpr(tree);
  }

  else if (tree->description == "uExpr") 
    pass3unop(tree);
  
  else if (tree->description == "varAccess") 
    pass3varaccess(tree);

  else if (tree->description == "arrayRef") // need to look at decaf.y
    pass3arrayref(tree);

  else if (tree->description == "call") 
    pass3call(tree);

  else if (tree->description == "methodcall") 
    pass3methodcall(tree);
  
  else if (tree->description == "interface") 
    pass3interface(tree);

  else if (tree->description.substr(0,9)=="constant_") {
    string temp_description = tree->description.substr(9);
    tree->stype = temp_description;
    tree->children[0]->stype = temp_description;
  }
  else if (tree->description.substr(0,5)=="TYPE_")
    tree->stype = tree->description.substr(5);
  else if (tree->description.substr(0,5)=="open_") {
    pass3open(tree);

  }
  else if (tree->description.substr(0,8)=="matched_") {
    pass3matched(tree);
  }
  // else if (tree->token) { 
  //   pass3token(tree);
  // }
  else if (tree->description == "return")
    pass3return(tree);

  else if (tree->description == "expr")
    pass3expr(tree);
  else if (tree->description == "print")
    pass3print(tree);
  else if (tree->description == "readline")
    tree->stype = "String";
  else if (tree->description == "readinteger")
    tree->stype = "Integer";
  else if (tree->description == "newarray")
    pass3newarray(tree);
  else if (tree->description == "new"){
    pass3(tree->children[1]);
    tree->stype = tree->children[1]->stype;
}
  
  else 
    for (size_t i=0;i<tree->children.size();i++) {
      if (tree->children[i])
	pass3(tree->children[i]);}
}


extern string filename;

extern FILE * yyin;  



#include <iostream>
#include <map>
#include <cassert>
#include <vector>

using namespace std;

#include "semantics.h"
//#include "parsetree.h"

extern ParseTree * top;
extern string filename;

Symtab * currentScope = NULL;
S_interface* currentInterface = NULL;
S_class* currentClass = NULL;
S_function* currentFunction = NULL;
vector<sstriple> subsuprels;
int currentSeq = 0;
//bool interfaceCheck = false;


int varcount = 0;

Symtab::Symtab(Symtab *p) : parent(p) {}

semantics * Symtab::lookup(string key) { 
  semantics *answer;
  return 
    (answer = local_lookup(key)) ? answer : 
    parent ? parent->lookup(key) : NULL;

}

semantics * Symtab::local_lookup(string key) { 
  if (dict.count(key) == 0)
    return NULL;
  return dict[key];
}

// S_function * class_lookup(S_class * currClass, string key) {
//   if (!currClass)
//     return NULL;
//   for (int i = 0; i < currClass->fields.size();i++) {
//     if (currClass->fields[i]

//   }



// }



void Symtab::insert(string key, semantics * item) {
  assert (local_lookup(key) == NULL);
  dict[key] = item;
}

void Symtab::print(ostream & ostr) 
{
  for (Dictionary::iterator i = dict.begin(); i != dict.end(); i++) {
    semantics * value = i->second;
    ostr << i->first << " => " << (value==NULL?"NULL":value->kind()) << endl;
  }
  if (parent)
    parent->print(ostr);
}

void openscope()
{
  //if (currentScope)
  //cout<<"scope before opening "<<endl;//currentScope->print(cout);    
  currentScope = new Symtab(currentScope);
  //cout<<"scope after opening "<<endl;currentScope->print(cout);    
}

Symtab *closescope()
{
  //cout<<"scope before closing "<<endl;currentScope->print(cout);  
  Symtab *v = currentScope;
  currentScope = currentScope->parent;

  return v;
}

void semantic_error(string err, int line)
{
  cout << filename<< ":"<<line <<":Semantic error: " << err << endl;
  exit(1);
}


string get_type_name(ParseTree *typetree)
{
  // Convert a tree representing a type to a string.
  // My descriptions for these always start with TYPE_.
  // Your mileage may vary.
  assert(typetree->description.substr(0,5)=="TYPE_");
  //cout << "In get type name with " << typetree->toString() << endl;
  // Arrays go to "[]",
  // User-defined things go to the identifier.
  // Built-ins are like TYPE_int, so strip off the TYPE_.
  if (typetree->description == "TYPE_userdef") // !!!!! Look at parser for this
	return typetree->children[0]->token->text;
  else if (typetree->description == "TYPE_array")
	return get_type_name(typetree->children[0])+" []";
  return typetree->description.substr(5);
}

string get_type_name_no_array(ParseTree * typetree) {
  assert(typetree->description.substr(0,5)=="TYPE_");

  if (typetree->description == "TYPE_userdef") // !!!!! Look at parser for this
    return typetree->children[0]->token->text;
  else if (typetree->description == "TYPE_array")
    return get_type_name(typetree->children[0]);
  return typetree->description.substr(5);
}

string get_base_type(string str)
{
  int len = str.length();
  if (len <=3)
    return str;
  if (str.substr(len-3) == " []")
    return get_base_type(str.substr(0,len-3));
  return str;
}

bool array_object(string str)
{
  return (str.substr(str.length()-3)==" []");
}

bool defined_type(string str)
{
  return (str == "Integer"||str == "Bool"||str == "Double"||str == "String"||(currentScope->lookup(str)));
}   




void error_if_defined_locally(ParseTree *tree)
{
  // tree represents an identifier.  Make sure it's not already defined.
  assert(tree->type == TERMINAL and tree->token->type == T_Identifier);
  //cout << "Error 1" << endl;
  string var_name = tree->token->text;
  //  cout << "Error 2" << endl;
  int lineno = tree->token->line;
  //cout << "Error 3" << endl;
  if (currentScope->local_lookup(var_name)) {
    //cout << "Error 3.5" << endl; 
    semantic_error(var_name + " is already defined", lineno);
    //cout << "Error 4" << endl; 
  }
    //cout << "Error 5" << endl; 
}


bool class_occurs(S_class * subclass, S_class * superclass) 
{
  if (subclass == superclass)
    return true;
  for (size_t i = 0; i < subsuprels.size();i++) {
    if (subsuprels[i].first == superclass)  
      return class_occurs(subclass, subsuprels[i].second);}
  return false;
}
	
bool matching_classes(S_class * sub, S_class * sup) 
{
  if (!sup)
    return false;
  return (sub == sup)||matching_classes(sub,sup->parentClass);
}

bool matching_classes_or_interface(string name1, string name2)
{
  
  S_class * temp1 = (S_class *)currentScope->lookup(name1);
  if (!temp1)
    return false;
  
  if (temp1->kind()=="S_class") {
    S_interface * temp2 = (S_interface *)currentScope->lookup(name2);
    if (!temp2)
      return false;
    if (temp2->kind()!="S_interface") 
      return false;
    while (temp1) {
      for (size_t i=0;i<temp1->interfaces.size();i++) {
	S_interface * temp = temp1->interfaces[i];
	if (temp2 == temp)
	  return true;
      }
      temp1=temp1->parentClass;
    }
  }
  else if (temp1->kind()=="S_interface") {
    S_interface * temp1 = (S_interface *)currentScope->lookup(name1);
    S_class * temp2 = (S_class *)currentScope->lookup(name2);
    if (!temp1||!temp2)
      return false;
    if (temp2->kind()!="S_class") 
      return false;
    while (temp2) {
      for (size_t i=0;i<temp2->interfaces.size();i++) {
	S_interface * temp = temp2->interfaces[i];
	if (temp1 == temp)
	  return true;
      }
      temp2=temp2->parentClass;
    }
  }
  return false;
}



bool ident_in_class(string str, S_class * temp)
{
  if (!temp)
    return false;
  for (size_t i = 0; i<temp->fields.size();i++) {
    S_variable * tempVar = (S_variable *)temp->fields.at(i);
    if (tempVar->kind() == "S_variable"){
      //cout<<"temp name = "<<tempVar->name<<endl;
      if (tempVar->name == str)
	return true;  
    }
  }
  if (!temp->parentClass)
    return false;
  return ident_in_class(str, temp->parentClass);
}

S_function * func_in_class(string str, S_class * temp)
{
  if (!temp)
    return NULL;
  for (size_t i = 0; i<temp->fields.size();i++) {
    S_function * tempVar = (S_function *)temp->fields.at(i);
    if (tempVar->kind() == "S_function"){
      //cout<<"temp name = "<<tempVar->name<<endl;
      if (tempVar->name == str)
	return tempVar;  
    }
  }
  if (!temp->parentClass)
    return NULL;
  return func_in_class(str, temp->parentClass);
}

S_function * method_ident_in_class(string str, S_class * temp)
{
  if (!temp)
    return NULL;
  for (size_t i = 0; i<temp->fields.size();i++) {
    //cout << "i = "<<i<<endl;
    S_function * tempFunc = (S_function *)temp->fields.at(i);
    if (tempFunc->kind() == "S_function"){
      if (tempFunc->name == str)
	return tempFunc;  
    }
  }
  for (size_t i = 0; i<temp->interfaces.size();i++) {
    S_interface * tempInter = (S_interface *)temp->interfaces.at(i);
    for (size_t j = 0; j<tempInter->functions.size();j++) {
      S_function * tempFunc = (S_function *)tempInter->functions.at(i);
      if (tempFunc->kind() == "S_function"){
	if (tempFunc->name == str)
	  return tempFunc;  
      }
    }
  }
  if (!temp->parentClass)
    return NULL;
  return method_ident_in_class(str, temp->parentClass);
}




void makeClass(ParseTree * tree) {
  assert(tree->description=="classdecl");
  //cout<<"Name = "<<tree->children[1]->token->text<<endl;
  error_if_defined_locally(tree->children[1]);
  //cout<<"After error if defined locally"<<endl;
  S_class * entry = new S_class;
  entry->parentClass = NULL;
  
  currentClass = entry;
  // if (tree->children[3]) {
  //   cout << tree->children[3]->children[1]->children[0]->token->text<<endl;
  //   assert(tree->children[3]->description=="implements");
  //   assert(tree->children[3]->children[1]->description=="nident");
    
  //   /////////////////////////////////// CALL PASS ! ON INTERFACES INSTEAD????
  //   for (int i = 0; i< tree->children[3]->children[1]->children.size();i++) {
  //     pass
  //     // S_interface * tempInter = (S_interface *)currentScope->lookup(tree->children[3]->children[1]->children[i]->token->text);
  //     // if (tempInter->kind()!="S_interface")
  //     // 	semantic_error("Implementing non-interface type", tree->children[3]->children[1]->children[i]->token->line);
  //     cout<<"hi"<<endl;
  //     currentClass->interfaces.push_back(tempInter);
  //   }
  // }
  
  openscope();//cout<<"open"<<endl;
  ParseTree * fieldTree = tree->children[4]; // fields
  if (fieldTree)
    for (size_t i = 0; i < fieldTree->children.size();i++) {  
      
      pass1(fieldTree->children[i]);}
  
  tree->symtab = closescope();//cout<<"closed class scope "<< tree->description<<endl;
  
  string class_name = tree->children[1]->token->text;
  currentScope->insert(class_name,currentClass);
  // if (tree->children[2]) {
  //   cout << "IN EXTENDS: "<<class_name<<endl;
  //   string super = tree->children[2]->children[1]->token->text;
  //   S_class * superClass = (S_class *)currentScope->lookup(super);
  //   int lineno = tree->children[1]->token->line;
  //   if (class_occurs(currentClass, superClass))// for Pass 1.5
  // 	semantic_error(class_name+" already defined", lineno);
  //   sstriple trip = sstriple(currentClass,superClass,lineno);
  //   subsuprels.push_back(trip);
  // }
  currentClass = NULL;
}
    
 

void makeInterface(ParseTree * tree) {
  assert(tree->description == "interface");
  error_if_defined_locally(tree->children[1]);

  string interface_name = tree->children[1]->token->text;

  S_interface * entry = new S_interface;
  // ParseTree * protoTree = tree->children[2];
  // for (int i=0; i < protoTree->children.size();i++)
  //   entry->functions.push_back(
	 
  currentInterface = entry;
  openscope();// cout<<"open"<<endl;
  if (tree->children[2])
    for (size_t i=0; i < tree->children[2]->children.size();i++)
      pass1(tree->children[2]->children[i]);
  //cout<<"scope in make interface "<<endl;currentScope->print(cout);
  tree->symtab = closescope();//cout<<"close"<<endl;
  currentScope->insert(interface_name,entry);
  currentInterface = NULL;
}

void makePrototype(ParseTree * tree) {
  //assert(tree->description == "prototypes");
  assert(tree->description == "prototype");
  //cout << "after proto assert" << endl;
  error_if_defined_locally(tree->children[1]);

  string func_name = tree->children[1]->token->text;
  string func_type = get_type_name(tree->children[0]);
  S_function * entry = new S_function;
  entry->name = func_name;

  if (func_type != "Void") {
    entry->returnType = new S_type;
    entry->returnType->name = func_type;
  }
  else {
    entry->returnType = NULL;
    // entry->returnType = new S_type;
    // entry->returnType->name = "Void";
  }
  openscope();//cout<<"open"<<endl;
  ParseTree * formalTree = tree->children[2];
  S_variable * tempVar;
  if (formalTree)
    for (size_t i = 0; i < formalTree->children.size();i++) {
      tempVar = makeVariable(formalTree->children[i]);
      entry->formals.push_back(tempVar);
    }
  tree->symtab = closescope();//cout<<"close"<<endl;
  currentScope->insert(func_name,entry);

  //cout<<"scope in make prototype "<<endl;
  //currentScope->print(cout);
    
  if (currentInterface)
    currentInterface->functions.push_back(entry);
  
  else if (currentClass)
    currentClass->fields.push_back(entry); 
}

void makeFunction(ParseTree * tree)
{
  
  //cout << "Make Function 0" << endl;
  assert(tree->description == "funcdecl");
  //cout << "Make Function 1" << endl;
  error_if_defined_locally(tree->children[1]);

  string func_name = tree->children[1]->token->text;
  string func_type = get_type_name(tree->children[0]);
  S_function * entry = new S_function;
  entry->name = func_name;

  //cout << "Make Function 2" << endl;

  //cout << "return type = " << func_type<<endl;

  if (func_type != "Void") {
    //cout<<"_________FUNCTION TYPE = "<< func_type<<" "<< func_name<<endl;
    entry->returnType = new S_type;
    entry->returnType->name = func_type;
  }
  else {
    //cout<<"_________FUNCTION TYPE = VOID "<< func_type<<" "<< func_name<<endl;
    entry->returnType = NULL;
  }
  openscope(); //cout<<"open"<<endl;
 
  //cout<<"1"<<endl;
  currentFunction = entry; 
  //if (tree->children[2])
    ParseTree * formalTree = tree->children[2];
  S_variable * tempVar;

  //cout<<"2"<<endl;
  
  if (formalTree) {
    //cout << "size = " << formalTree->children.size()<<endl;
    for (size_t i = 0; i < formalTree->children.size();i++) {
      
      tempVar = makeVariable(formalTree->children[i]);
      //cout<<"IIIIIIIIIIIIII"<<tempVar->type->name;
      entry->formals.push_back(tempVar);
    }
  }
  //cout<<"3"<<endl;
  pass1(tree->children[3]);
  currentFunction=NULL;

  //cout<<"scope inside main 2 "<<endl;currentScope->print(cout);  

  tree->symtab = closescope();
  //tree->symtab->print(cout);
  //cout<<"closed function scope " <<tree->description<< endl;


 
  currentScope->insert(func_name,entry);
  
  if (currentInterface)
    currentInterface->functions.push_back(entry);
  
  else if (currentClass){
    //cout<<"yup "<<entry->name<<" "<<entry->returnType->name<<endl;
    currentClass->fields.push_back(entry);
  }
}

S_variable * makeVariable(ParseTree *tree)
{
  // In pass1, Create a S_variable object, and insert in the symbol table.
  // This is useful for formal parameters as well as
  // variable declarations in  side Stmt blocks.  Your
  // parse tree may vary slightly, so tweak this as needed.
  
  //assert(tree->description == "variabledecl");
  if (tree->description == "variabledecl")
    tree = tree->children[0];
  assert(tree->description == "variable");
  
  


  
  string var_name = tree->children[1]->token->text;
  string type_name = get_type_name(tree->children[0]);
  //cout << "var_name = "<<var_name << endl;
  
  // if (currentScope->lookup(tree->children[1]->token->text))
  //     semantic_error(tree->children[1]->token->text+ " already defined222",tree->children[1]->token->line);

  error_if_defined_locally(tree->children[1]);
  
  S_variable *entry = new S_variable;
  entry->type = new S_type;
  entry->name = var_name;
  
  entry->type->name = type_name;
  currentScope->insert(var_name, entry);
  
  
  
  if (currentClass && !currentFunction)  {
    currentClass->fields.push_back(entry);
    entry->instanceClass = currentClass;
  }
  return entry;
}

void makeStmtBlock(ParseTree * tree) {
  assert(tree->description == "stmtblock");
  //cout<<"4"<<endl;

  openscope(); //cout<<"open"<<endl;

  ParseTree * tempTree;
  if(tree->children[0]) {
    assert(tree->children[0]->description=="vardecls");
    tempTree = tree->children[0]; // goes to "vardecls"
    for (size_t i = 0; i < tempTree->children.size();i++)
      pass1(tempTree->children[i]);
  }  
  //cout<<"5"<<endl;
  if(tree->children[1]) {
    assert(tree->children[1]->description=="stmts");
    //cout<<"6"<<endl;
    tempTree = tree->children[1]; // goes to "stmts"
    for (size_t i = 0; i < tempTree->children.size();i++){
      //cout<<"7"<<endl;
      if (tempTree->children[i])
	pass1(tempTree->children[i]);
      //cout<<"8"<<endl;
    }
  }
  //cout<<"8"<<endl;
  //cout<<"scope inside main "<<endl;currentScope->print(cout);  

  //closescope();
  tree->symtab = closescope();//cout<<"close"<<endl;
}


void pass2classdecl(ParseTree * tree) {
  assert(tree->description == "classdecl");
    
  currentClass = (S_class *)(currentScope->lookup(tree->children[1]->token->text));
  
  // S_class * tempClass = currentClass;
  // S_class * tempClass = currentClass;
  
  // while (currentClass->parentClass) {
  //   S_class * tempClass = tempClass->parentClass;
    
  // }

  string class_name = tree->children[1]->token->text;
  if (tree->children[2]) {
    //cout << "IN EXTENDS: "<<class_name<<endl;
    string super = tree->children[2]->children[1]->token->text;
    S_class * superClass = (S_class *)currentScope->lookup(super);
    int lineno = tree->children[1]->token->line;
    if (class_occurs(currentClass, superClass))// for Pass 1.5
	semantic_error(class_name+" already defined", lineno);
    //sstriple trip = sstriple(currentClass,superClass,lineno);
    //subsuprels.push_back(trip);
    currentClass->parentClass = superClass;
  }

  for (size_t i = 0; i < tree->children.size();i++){
    
    pass2(tree->children[i]);
  }

  S_interface * tempInterface;
  S_function * tempFunction;
  

  if (tree->children[3]) {
    
    ParseTree * implementTree = tree->children[3];
    for (size_t j = 0;j<implementTree->children[1]->children.size();j++) {
      S_interface * tempInterface = (S_interface *)currentScope->lookup(implementTree->children[1]->children[j]->token->text);
      currentClass->interfaces.push_back(tempInterface);
      // for (int k=0;k<currentClass->interfaces.size();k++){
      // 	S_function * tempFunc = currentClass->interfaces[k];
      // 	currentScope->insert(tempFunc->returnType->name,tempFunc);
    }
  }
  
  
  //cout<< "currentClass->interfaces.size() = "<<   currentClass->interfaces.size()<<endl;
  for (size_t i = 0; i < currentClass->interfaces.size();i++){
    //cout<<"hi"<<endl;
    assert(currentClass->interfaces.at(i));
    assert(currentClass->interfaces.at(i)->kind() == "S_interface");
    tempInterface = currentClass->interfaces.at(i);
    
    for (size_t p = 0; p < tempInterface->functions.size();p++) {
      //cout<<"p = "<<p<<endl;
      tempFunction = tempInterface->functions[p];
      if (!tempFunction)
	semantic_error("Function not defined",0); // NEED TO FIX LINENUMBER
      if (tempFunction->kind() != "S_function")
	semantic_error(tempFunction->name + "defined but is not a function",tree->children[1]->token->line);
      //cout<<"scope before sem check main "<<endl;currentScope->print(cout);  
      if (!currentScope->lookup(tempFunction->name)&&!func_in_class(tempFunction->name,currentClass))
	semantic_error(tempFunction->name + " never defined in class",tree->children[1]->token->line);
      //cout<<"scope before sem check main "<<endl;currentScope->print(cout);  
      S_function * tempFunction2 = func_in_class(tempFunction->name,currentClass);// (S_function *)currentScope->lookup(tempFunction->name);
      //tempFunction->name
      //cout <<"hey"<<endl;
      //cout << "tempFunction1 = "<<tempFunction->returnType->name<<" "<<tempFunction->name<<endl;
      // cout<< "in between"<<endl;
      // cout <<tempFunction2->name<<endl;
      // cout << "tempFunction2 = "<<tempFunction2->returnType->name<<" "<<endl;
      if (tempFunction->returnType){
	//cout<<"hi"<<endl;
	if (!tempFunction2->returnType)
	  semantic_error(tempFunction->name+" has mismatched return 1types1",tree->children[1]->token->line);
	if (tempFunction->returnType->name != tempFunction2->returnType->name)
	  semantic_error(tempFunction->name+" has mismatched return types2",tree->children[1]->token->line);
	//cout<<"hi"<<endl;
      }
      else 
	if (tempFunction2->returnType)
	  semantic_error(tempFunction->name+" has mismatched return types3",tree->children[1]->token->line);
    }
    /// look up P in class hierarchy?
      
      //assert(currentScope->lookup(tempFunction->name));
      
          //        or has return type that doesn't match, or has
          //        parameters that do not match exactly,
          //            issue the appropriate semantic error
  } 
  //cout<<"hi"<<endl;
  currentClass = NULL;
}




void pass2funcdecl(ParseTree * tree) {
  assert(tree->description == "funcdecl");
  //cout << "IN FUNC DECL" << endl;
  S_function * tempFunc = (S_function *)(currentScope->lookup(tree->children[1]->token->text));
  currentFunction = tempFunc;
  varcount = 0;
  if (currentClass)
    varcount = 1;
  for (size_t i=0;i<tree->children.size();i++)
    pass2(tree->children[i]);
  currentFunction=NULL;
  currentScope = currentScope->parent;
}


void pass2vardecl(ParseTree * tree) {
  if (tree->description == "variabledecl")
    tree = tree->children[0];
  assert(tree->description == "variable");

  //cout << "in var, name = " << tree->children[1]->token->text<<endl;

  int linenumber = tree->children[1]->token->line;
  //cout <<"hey"<<endl;
  //cout << "in var, name = " << tree->children[1]->token->text<<endl;
  string vartype = get_type_name_no_array(tree->children[0]);

  // if (currentScope->lookup(tree->children[1]->token->text))
  //     semantic_error(tree->children[1]->token->text+ " already defined222",linenumber);
  
  // if type not defined at all, semantic error. ??

  //cout <<"hey2"<<endl;
  
  if (tree->children[0]->description == "TYPE_userdef" && !currentScope->lookup(vartype))
    semantic_error(vartype+ " used but never defined",linenumber);
  //cout<<"scope inside main "<<endl;currentScope->parent->print(cout);  
  //cout << "NAME = "<<tree->children[1]->token->text<<endl;
  //cout <<"hey2"<<endl;
  //currentScope->print(cout);
  S_variable * tempVar = (S_variable *)(currentScope->lookup(tree->children[1]->token->text));  
  //cout <<"hey3"<< tree->children[1]->token->text<<endl;

  // if (tempVar->globalType==true && tempVar->localType==true) {
  //   cout << tempVar->varnumber <<endl;
  //   cout << "_________BOTH LOCAL AND GLOBAL__________" << tempVar->name<<endl; }

  //currentScope->print(cout);
  
  //cout << "AAAAAAAAAAAA"<<tempVar->name<<endl;
  //cout <<"hey4"<<endl;
  //cout << "AAAAAAAAAAAA"<<tempVar->type->name<<endl;
  //cout <<"hey3"<<endl;
  if (currentFunction) {
    //cout <<"current Function:"<< tree->children[1]->token->text<<endl;
    //cout <<"hey3.5"<<endl;
    tempVar->localType = true;
    tempVar->globalType = false;
    tempVar->instanceClass = NULL;
    //cout <<"hey4"<<endl;
    tempVar->varnumber = varcount;
    varcount++;
    if (tempVar->type->name == "Double")
      varcount++;
    
  }


  else if (currentClass){
    //cout <<"current class:"<< tree->children[1]->token->text<<endl;
    //cout <<"hey5"<<endl;    
    //tempVar->instanceClass = new S_class;
    //cout <<"hey50____________________________"<<endl;    
    tempVar->globalType = false; 
    tempVar->localType = false;
    tempVar->instanceClass =currentClass;
    
    
  }
  else // variable is global
    {
      //cout <<"hey51"<<endl;    
    tempVar->globalType = true; 
    tempVar->localType = false;
    tempVar->instanceClass =NULL;
    //cout <<"hey"<<endl;
    }
  //cout <<"hey"<<endl;


}

void pass2fielddecl(ParseTree * tree) {
  assert(tree->description == "fields");
  ParseTree * tempTree;
  S_variable * tempVar;
  //S_function * tempFunc;

  for (size_t i = 0; i<tree->children.size();i++) { 
    //cout<<"In pass2 field decl, i = "<<i<<endl;
    tempTree = tree->children[i];
    
    if (tempTree->description == "variabledecl") {
      //cout<<"In pass2 field decl, variable decl "<<i<<endl;
      // if (currentScope->parent->parent->lookup(tempTree->children[0]->children[1]->token->text)) {
      // 	cout<<"AJABF"<<endl;
      // 	S_variable * tempVar2 = (S_variable *)currentScope->parent->lookup(tempTree->children[0]->children[1]->token->text);
	

      // 	cout <<endl<<endl<< "___Current Scope___"<<endl;
      // 	currentScope->print(cout);
      // 	cout << "___Parent Scope___"<<endl;
      // 	currentScope->parent->print(cout);
      // 	cout << endl<<endl;

      // 	if (!tempVar2->globalType) {
      // 	  cout <<"________HI1.6"<<endl;
      // 	  semantic_error("Variable " + tempVar2->name +" has already been declared in parent class", tempTree->children[0]->children[1]->token->line);}
      // 	cout <<"________HI1.7"<<endl;
      // }
      //cout <<"________HI2"<<endl;
      tempVar = (S_variable *)currentScope->lookup(tempTree->children[0]->children[1]->token->text); // PHILIP: removed local_ from lookup
      if (tempVar->kind() != "S_variable")
	semantic_error("Object declared as variable but is actually a "+tempVar->kind().substr(2), tree->children[0]->children[1]->token->line);
      
      
      tempVar->instanceClass = currentClass;
      
      pass2(tempTree);
      
    }// NEED TO CHECK FURTHER ABOUT USING CURRENTSCOPE VS CURRENTCLASS
    
    else if (tempTree->description == "funcdecl") {
      //look up the functions name in the class hierarchy (not scope)
      
      pass2(tempTree);

      // for (size_t j = 0; j<tempTree->children.size();j++){
      //  	pass2(tempTree->children[j]);  
      // 	cout << "In pass2 field decl, j = "<<j<<endl;
      // }
    }
    
    else 
      semantic_error("field not a variable or function declaration", tree->children[0]->children[1]->token->line);
    
  }  
  //cout << "End of Field"<<endl;
}

void pass3classdecl(ParseTree * tree)
{
  assert(tree->description == "classdecl");
  ///////////////////////////////////////////////////////////////////
  currentClass =  (S_class *)(currentScope->lookup(tree->children[1]->token->text)); // How do I get class?
  for (size_t i = 0; i<tree->children.size();i++) 
    pass3(tree->children[i]);
  
  // for (int i = 0; i<currentClass->interfaces.size();i++) {
  //   for (int j = 0; j<currentClass->interfaces[i]->functions.size();j++) {
  //     S_function * tempIntFunc = currentClass->interfaces[i]->functions.at(j);
    
  //     S_function * tempClassFunc = (S_function *)currentScope->lookup(tempIntFunc->name);
  //     if (tempClassFunc->kind()!="S_function")
  // 	semantic_error("Interface function redeclared as variable", tree->children[1]->token->line);
    
  //     if (tempIntFunc->formals.size()!=tempClassFunc->formals.size())
  // 	semantic_error("Mismatched number of parameters between class and interface", tree->children[1]->token->line);
    
  //     for (int k = 0; k<tempIntFunc->formals.size();k++){
  // 	S_variable * tempVar1 = tempIntFunc->formals.at(k);
  //     S_variable * tempVar1 = tempIntFunc->formals.at(k);
  // }

currentClass = NULL;	  
}

void pass3funcdecl(ParseTree * tree){
  assert(tree->description == "funcdecl");
  
  currentFunction =  (S_function *)(currentScope->lookup(tree->children[1]->token->text));
  //cout << "INSIDE FUNCTION DECL PASS 3" << endl;
  for (size_t i = 0; i<tree->children.size();i++) {
    //cout << "in func decl loop i = "<< i<<endl;
    if (tree->children[i])
      pass3(tree->children[i]);
    //cout << "in main"<<endl;
  }
  currentFunction = NULL;	  
}

void pass3interface(ParseTree * tree){
  assert(tree->description == "interface");

  //cout<<"scope outside interface "<<endl;currentScope->print(cout);  
  //cout<<"name of interface = "<< tree->children[1]->token->text<<endl;
  
  //interfaceCheck = true;

  if (currentScope->lookup(tree->children[1]->token->text)) 
    currentInterface = (S_interface *)currentScope->lookup(tree->children[1]->token->text);
  else
    semantic_error("Interface not found", tree->children[1]->token->line);
  pass3(tree->children[2]);
  currentInterface = NULL;
  //interfaceCheck = false;
  
}



void pass3binop(ParseTree * tree){
  //cout<<"in binop"<<endl;
  assert(tree->description == "bExpr");
  pass3(tree->children[0]);
  pass3(tree->children[2]);
  string binop = tree->children[1]->token->text;
  //cout << "binop: "<<binop<<endl;
  ///////////////////////////////////////////////////
  string T1 = get_base_type(tree->children[0]->stype); 
  //cout<<"hey1"<<endl;
  string T2 = get_base_type(tree->children[2]->stype); 
  // cout<<"hey1"<<endl;
  // cout << "T1 = "<<T1<<endl;
  // cout << "T2 = "<<T2<<endl;
  // cout << "binop: "<<binop<<endl;
  // cout << "T3 = "<<tree->children[2]->children[1]->stype<<endl;
    
  if (T1!=T2){
    if (T1 == "Integer"||T1 == "Bool"||T1 == "Double"||T1 == "String"||T2 == "Integer"||T2 == "Bool"||T2 == "Double"||T2 == "String")
      semantic_error("Unmatched types-1", tree->children[1]->token->line); // NEED TO UPDATE LINE NUMBER, ALSO separate assignment? ALSO comparison with bool
    else {
      S_class * tempClass1 = (S_class*)currentScope->lookup(T1);
      S_class * tempClass2 = (S_class*)currentScope->lookup(T2);
      if (!(matching_classes(tempClass1,tempClass2)||matching_classes(tempClass2,tempClass1)||(matching_classes_or_interface(T1, T2))))
	semantic_error("Unmatched user-defined types", tree->children[1]->token->line); // NEED TO UPDATE LINE NUMBER

    }
  }
  
  if (binop == "=") {
    string L1 = tree->children[0]->stype;
    string L2 = tree->children[2]->stype;
    //cout << "L1 = "<<L1<<endl;
    //out << "L2 = "<<L2<<endl;

     if ((L1 != L2)&&!(matching_classes_or_interface(L1, L2))&&(T1!=L1&&T2!=L2))//&&L1!=L2.substr(0,L2.length()-3))
       semantic_error("Array assignment with array", tree->children[1]->token->line); // NEED TO UPDATE LINE NUMBER
     
     tree->stype = T1;
     //tree->stype = "assign";
  }
  else if (binop == "==" || binop == "!="){
    tree->stype = "Bool";
  }
  //cout << "BINOP 2" <<endl;
  else if (binop == "+" || binop == "-" || binop == "*" || binop == "/" || binop == "%") {
    if (!(T1 == "Integer"||T1 == "Double"))
      semantic_error("Unmatched types", 0); // NEED TO UPDATE LINE NUMBER
    tree->stype = T1;
  }
  //cout << "BINOP 3" <<endl;
  else if (binop == "<" || binop == "<=" || binop == ">" || binop == ">=") {
    if (!(T1 == "Integer"||T1 == "Double"))
      semantic_error("Unmatched types - only integers and doubles may be used with comparison operators", 0); // NEED TO UPDATE LINE NUMBER
    tree->stype = "Bool";
  }
  else if (binop=="&&"||binop=="||") {
    if (!(T1 == "Bool"))
      semantic_error("Unmatched types", 0); // NEED TO UPDATE LINE NUMBER
    tree->stype = "Bool";
  }
  
  else
    semantic_error("Binary operation does not match permitted operations", 0); 
  //    NEED TO UPDATE LINE NUMBER
  //cout << "BINOP 4" <<endl;
}




void pass3unop(ParseTree * tree){
  assert(tree->description == "uExpr");
  pass3(tree->children[0]);
  pass3(tree->children[1]);
  string binop = tree->children[0]->token->text;
  ///////////////////////////////////////////////////
  string T1 = tree->children[1]->stype; 
  if (binop == "!") {
    if (T1!="Bool")
      semantic_error("Unary operator '!' only works with boolean expressions", tree->children[0]->children[1]->token->line);
    tree->stype = "Bool";
  }
  else if (binop == "-") {
    if (T1!="Integer"&&T1!="Double")
      semantic_error("Unary operator '-' only works with integers or doubles", tree->children[0]->token->line);
    tree->stype = T1;
  }  
  else
    semantic_error("Unary operation "+binop+" does not match permitted operations", 0);
}



void pass3varaccess(ParseTree * tree) { // expr.ident
  assert(tree->description == "varAccess");
  pass3(tree->children[0]);
  pass3(tree->children[1]);
  tree->stype = tree->children[1]->stype;

  string exprType = tree->children[0]->stype;
  string identToAccess = tree->children[1]->token->text;
  //cout << "exprType = |"<<exprType<<"|"<<endl;
  //cout << "identToAccess = |"<<identToAccess<<"|"<<endl;
  
  if (!currentScope->lookup(identToAccess))
    semantic_error("Accessing member data "+identToAccess+" outside class", tree->children[1]->token->line);

  if (exprType=="Integer"||exprType=="Bool"||exprType=="Double"||exprType=="String")
    semantic_error("Variable must be accessed from a class", tree->children[1]->token->line);
  
  S_class * tempClass = (S_class *)currentScope->lookup(exprType);
    
  if (!ident_in_class(identToAccess,tempClass)){
    semantic_error("Variable undefined in given class", tree->children[1]->token->line);
  }
  if (!currentClass)
    semantic_error("Class should be set", tree->children[1]->token->line);
}

void pass3print(ParseTree * tree)
{
  pass3(tree->children[1]);
  // string T1 = get_base_type(tree->children[1]->stype);
  // cout << "t1 = "<<T1<<endl;
  // if (T1!="Bool"&&T1!="String"&&T1!="Integer")
  //   semantic_error("Can only print a string, bool, or integer", tree->children[0]->token->line);

}

void pass3pexpr(ParseTree * tree)
{
  for (size_t i=0; i<tree->children.size();i++) {
    pass3(tree->children[i]);
    string T1 = get_base_type(tree->children[i]->stype);
    if (T1!="Bool"&&T1!="String"&&T1!="Integer")
      semantic_error("Can only print a string, bool, or integer", tree->children[0]->token->line);
  }
}

void pass3newarray(ParseTree * tree)
{
  assert(tree->children[0]->token->text == "NewArray");
  pass3(tree->children[1]);
  pass3(tree->children[2]);

  if (tree->children[1]->stype != tree->children[2]->stype)
    semantic_error("Types for NewArray must match", tree->children[0]->token->line);
  //cout<<"AAAAAAAAAAAAA"<<tree->children[1]->stype+" []"<<endl;
  tree->stype = tree->children[1]->stype+" []";

}




void pass3arrayref(ParseTree * tree) {
  assert(tree->description == "arrayRef");
  pass3(tree->children[0]);
  pass3(tree->children[1]);

  string T1 = tree->children[0]->stype; 
  string T2 = tree->children[1]->stype; 

  if (!array_object(T1))
    semantic_error("Must index an indexed array", 0);//Need to figure out line  
  if (T2 != "Integer")
    semantic_error("Must index using integer type", 0);//Need to figure out line
  if (!defined_type(get_base_type(T1)))
    semantic_error("Type of object being indexed undefined1", 0);//Need to figure out line

  // cout << "T1 = "<<T1<<endl;
  // cout << "T2 = "<<T2<<endl;


  // ensure(base(type(expr1)) is either built-in or a class)
  // ensure (expr1 is an array object)
  //cout << T1.substr(0,T1.length()-3)<<endl;
  tree->stype = T1.substr(0,T1.length()-3);

}










void pass3call(ParseTree * tree) {
  assert(tree->description == "call");
  
  pass3(tree->children[0]);
  pass3(tree->children[1]);

  //cout << "FUNCTION CALL PASS 3" << endl;

  string funcName = tree->children[0]->token->text;
  //cout<<"hey "<<funcName<<endl;
  if (!currentScope->lookup(funcName))
    semantic_error("Function never defined", tree->children[0]->token->line);
  //cout<<"CURRENTSCOPE IS = "<<endl;currentScope->print(cout);
  S_function * tempFunc = (S_function *)currentScope->lookup(funcName); 
  if (tempFunc->kind() != "S_function")
    semantic_error("Calling "+tempFunc->kind().substr(2)+" like function", tree->children[0]->token->line);
  
  //cout<<"temp func kind = "<<tempFunc->formals.size()<<endl;
  if (tree->children[1]) { 
    //cout<<"hey1 "<<tree->children[1]->children.size()<<endl;
    //cout<<"hey2 "<<tempFunc->formals.size()<<endl;
    if (tree->children[1]->children.size() != tempFunc->formals.size()){
      //cout<<"tree->children[1]->children.size() = "<<tree->children[1]->children.size()<<endl;
      //cout<<"tempFunc->formals.size() = "<<tempFunc->formals.size()<<endl;
      semantic_error("Number of arguments given differs from number needed", tree->children[0]->token->line);   }
    //cout<<"hey "<<funcName<<endl;
    for (size_t i = 0; i < tree->children[1]->children.size(); i++) {
      pass3(tree->children[1]->children[i]);
      //cout<<"BEFORE ->FORMALS[i]"<<endl;
      if (tree->children[1]->children[i]->stype != tempFunc->formals[i]->type->name){
	// cout<<"__"<<tree->children[1]->children[i]->token->text<<endl;
	// cout<<tree->children[1]->children[i]->stype<<endl;
	// cout<<tempFunc->formals[i]->type->name<<endl;
	semantic_error("Formal types required by function do not match those given", tree->children[0]->token->line);   }
    }
    //cout<<"hey "<<funcName<<endl;
  }
  else if (tempFunc->formals.size() != 0)
    semantic_error("No arguments given", tree->children[0]->token->line);   
  //cout<<"hey "<<funcName<<endl;
  if (tempFunc->returnType)
    tree->stype = tempFunc->returnType->name;
  else
    tree->stype = "Void";
  //cout<<"hey "<<funcName<<endl;
}




void pass3methodcall(ParseTree * tree) {
  assert(tree->description == "methodcall");
  pass3(tree->children[0]);
  string methodName = tree->children[1]->token->text;
  string className = tree->children[0]->stype;

  //cout << "class name = |"<<className<<"|"<<endl;

  if (!currentScope->lookup(className))
    semantic_error("Class undefined in current scope", tree->children[1]->token->line);  
  S_class * tempClass = (S_class *)currentScope->lookup(className);
  if (tempClass->kind() != "S_class")
    semantic_error("Calling method from non-class object", tree->children[1]->token->line);  

  if (tempClass->parentClass)
    cout << className <<endl;
  cout << className <<endl;
  // cout << "METHOD CALL PASS 3" << endl;
  //cout<<"scope inside method call "<<endl;currentScope->print(cout);  

  S_function * tempFunc = (S_function *)method_ident_in_class(methodName,tempClass);
  
  //cout << "METHOD CALL PASS 32" << endl;

  if (!tempFunc)
    semantic_error("Method "+methodName+" undefined in given class "+className, tree->children[1]->token->line);  
  if (tree->children[2]) {
    //cout<<tree->children[2]->children.size()<<endl;
    if (tree->children[2]->children.size() != tempFunc->formals.size())
      semantic_error("Number of arguments given differs from number needed", tree->children[1]->token->line);   
    for (size_t i = 0; i < tree->children[2]->children.size(); i++) {
      //cout<<tree->children[2]->children.size()<<" = "<<tempFunc->formals.size()<<endl;
      pass3(tree->children[2]->children[i]);


      //cout <<"i2= "<<i<<endl;
      //cout<<tempFunc->formals[i]->type->name<<endl;
      //cout<<tree->children[2]->children[i]->stype<<endl;
      
      S_variable * tempFormal = (S_variable *)tempFunc->formals.at(i);
      //cout<<tempFunc->kind()<<endl;
      //cout<<"NULL"<<endl;

      // if (!tempFunc)
      // 	cout<<"NULL"<<endl;
      // else
      // 	cout<<"no"<<endl;
      // if (!tempFormal->type) {
      // 	cout << "name = "<<tempFormal->name<<endl;
      // 	cout<<"NULL1"<<endl;}
      // else
      // 	cout<<"no1"<<endl;
      
      // cout<<tempFormal->type->name<<endl;
      

      if (tree->children[2]->children[i]->stype != tempFormal->type->name){
	cout<<tree->children[2]->children[i]->stype<<endl;
	cout<<tempFormal->type->name<<endl;
	semantic_error("Formal type required by method do not match those given", tree->children[0]->token->line);
      }
      //cout <<"i3= "<<i<<endl;
    }
  }
  //cout << "METHOD CALL PASS 322" << endl;

  
  //if (tempFunc->returnType)
  
  if (tempFunc->returnType)
    tree->stype = tempFunc->returnType->name;
  else
    tree->stype = "Void";

  tree->children[1]->stype = tree->children[0]->stype;

  //tree->stype = tempFunc->returnType->name;

}
   // else if tree is object method call (expr.ident(actuals))
   //    pass3(expr);
   //    ensure (expr is a class)
   // 	ensure (ident names a function)
   // 	ensure (ident function belongs to expr class)
   // 	follow procedure for function_call above
   // 	ascribe type(tree) = return type of the function


void pass3open(ParseTree * tree) {
  if (tree->description.substr(0,13)=="open_while") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    if (tree->children[0]->children[1]->stype != "Bool")
      semantic_error("While statement requires a boolean expression", tree->children[0]->children[0]->token->line);
  }
  else if (tree->description.substr(0,13)=="open_for") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    if (tree->children[0]->children[2]->stype != "Bool")
      semantic_error("For loop requires the middle expression to be of type bool", tree->children[0]->children[0]->token->line);
  }
  else if (tree->description.substr(0,13)=="open_if") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    if (tree->children[0]->children[1]->stype != "Bool")
      semantic_error("If statement requires a valid boolean expression", tree->children[0]->children[0]->token->line);
  }
  else
    semantic_error("Invalid open statement", tree->children[0]->children[0]->token->line);
}


void pass3matched(ParseTree * tree) {
  if (tree->description.substr(0,13)=="matched_while") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    if (tree->children[0]->children[1]->stype != "Bool")
      semantic_error("While statement requires a boolean expression", tree->children[0]->children[0]->token->line);
  }
  else if (tree->description.substr(0,13)=="matched_for") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    if (tree->children[0]->children[2]->stype != "Bool")
      semantic_error("For loop requires the middle expression to be of type bool", tree->children[0]->children[0]->token->line);
  }
  else if (tree->description.substr(0,13)=="matched_if") {
    pass3(tree->children[0]);
    pass3(tree->children[1]);
    pass3(tree->children[2]);
    pass3(tree->children[3]);
    if (tree->children[0]->children[1]->stype != "Bool")
      semantic_error("If statement requires a valid boolean expression", tree->children[0]->children[0]->token->line);
  }
  else
    semantic_error("Invalid matched statement", tree->children[0]->children[0]->token->line);
}

void pass3token(ParseTree * tree) {
  //cout << "TREE IS TOKEN" << tree->description<<endl;
  //cout << "TREE IS TOKEN" << tree->token->text<<endl;
  if (tree->token->type == 279 || tree->token->type==308) { // If T_identifier
    
    semantics * tempSemantic = currentScope->lookup(tree->token->text);
    // cout<<tree->token->text<<endl;
    // cout<<tree->token->text<<" "<<tempSemantic->kind()<<endl;
    
    if (!tempSemantic)
      semantic_error("Identifier used but never defined", tree->token->line);

    else if (tempSemantic->kind() == "S_variable"){
      //cout<< "variable token = " <<tree->token->text<<endl;
      S_variable * tempVar = (S_variable *)tempSemantic;
      if (!tempVar->type)
	tree->stype = "Void";
      else
	tree->stype = tempVar->type->name;
      //cout<<tree->stype<<endl;
      
      //	else {	}
    }
    else if (tempSemantic->kind() == "S_function") {
      S_function * tempFunc = (S_function *)currentScope->lookup(tree->token->text);//tempSemantic;
      if (tempFunc->returnType)
	tree->stype = tempFunc->returnType->name;
      else
	tree->stype = "Void";
      
    }
    else if (tempSemantic->kind() == "S_class") {
      //cout<<"HELLLOOOOOOOO"<<endl;
      tree->stype = tree->token->text;
      
    }
  }
}

void pass3return(ParseTree * tree) {
  pass3(tree->children[1]);
  if (!currentFunction->returnType) {
    if (tree->children[1])
      semantic_error("Function of type void is returning an expression", tree->children[0]->token->line);
  }
  else {
    if (!tree->children[1])
      semantic_error("Must return an expression", tree->children[0]->token->line);
    else if (currentFunction->returnType->name != tree->children[1]->stype)
      semantic_error("Returning wrong type", tree->children[0]->token->line);
  }
}

void pass3expr(ParseTree * tree) {
  if (tree->children[0]->token->text == "New") {
    pass3(tree->children[1]);
    tree->children[0]->stype = tree->children[1]->stype;
    tree->stype = tree->children[1]->stype;
  }
  else if (tree->children[0]->token->text == "this") {
    if (!currentClass)
      semantic_error("Keyword 'this' can only be used within a class", tree->children[0]->token->line);
    //else  NEED TO CHECK ON THIS
  }
}

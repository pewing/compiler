#include "decafs.h" // TURN INTO .h FILE
#include <fstream>
#include <sstream>

using namespace std;

extern int yyparse(void);
extern ParseTree * top;
extern S_interface* currentInterface;
extern S_class* currentClass;
extern S_function* currentFunction;
extern Symtab * currentScope;
extern vector<sstriple> subsuprels;

ofstream * currentFile;
ofstream * topClassFile;

bool returning = false;

string filename;
string currentClassName = "";

string breakstr = "NULL";
string globVars = "";

void generate(ParseTree * tree);
void generate_expr(ParseTree * expr);
void generate_methodcall(ParseTree * tree);

void string_simplifier(string command, int ref) 
{
  //cout << command<<ref << endl;
  string canvas = "";
  if (command == "iload") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   iload_";
    else
      canvas = "   iload                 ";
  }
  else if (command == "istore") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   istore_";
    else
      canvas = "   istore                ";
  }
  else if (command == "dstore") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   dstore_";
    else
      canvas = "   dstore                ";
  }
  else if (command == "dload") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   dload_";
    else
      canvas = "   dload                 ";
  }
  else if (command == "astore") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   astore_";
    else
      canvas = "   astore                ";
  }
  else if (command == "aload") {
    if (ref==0||ref==1||ref==2||ref==3)
      canvas = "   aload_";
    else
      canvas = "   aload                 ";
  }
  else
    canvas = ("   "+command+"                                  ").substr(0,31-command.size());
  
  *currentFile << canvas << ref << endl;
}


string decaf_type_to_jasmin_type(string tname)
{
  string array = "";
  
  
  // suck up the array type stuff...
  while (tname[tname.size()-1]==']') {
    array += "[";
    tname = tname.substr(0, tname.size()-3);
  }
  // convert from decaf builtin 
  // to jasmin/java builtin
  if (tname=="String")
    tname = "Ljava/lang/String;";
  else if (tname=="Integer") 
    tname = "I"; 
  else if (tname=="Bool")
    tname = "Z";
  else if (tname=="Double")
    tname = "D"; 
  else if (tname=="Void")
    tname = "V"; 
  else
    tname = "L" + tname + ";";
  return array + tname;
}

string decaf_type_to_full_jasmin_type(string tname)
{
  string array = "";
  
  
  // suck up the array type stuff...
  while (tname[tname.size()-1]==']') {
    array += "[";
    tname = tname.substr(0, tname.size()-3);
  }
  // convert from decaf builtin 
  // to jasmin/java builtin
  if (tname=="String")
    tname = "java/lang/String;";
  else if (tname=="Integer") 
    tname = "int"; 
  else if (tname=="Bool")
    tname = "boolean";
  else if (tname=="Double")
    tname = "double"; 
  else {
    tname = "NEED TO FIX IN DECAF_TYPE_TO_FULL_JASMIN_TYPE";
    //tname = "L" + tname + ";"; 
  }
  return array + tname;
}

string jasmin_formals(ParseTree * formals)
 {
  // collect all the jasmin types of the 
  // formals into one string
  string answer = "";
  for (size_t i=0;i<formals->children.size();i++) {
    ParseTree* var = formals->children[i];
    //cout<<"IN JASMIN FORMALS var->children[0] = "<<var->children[0]<<endl;
    answer += decaf_type_to_jasmin_type(get_type_name(var->children[0]));
  }
  return "(" + answer + ")";
}

string jasmin_actuals(ParseTree * actuals)
{
    string answer = "";
  for (size_t i=0;i<actuals->children.size();i++) {
    //ParseTree* var = formals->children[i];
    //cout<<"IN JASMIN FORMALS var->children[0] = "<<var->children[0]<<endl;
    answer += decaf_type_to_jasmin_type(actuals->children[i]->stype);
  }
  return "(" + answer + ")";



}

string jasmin_type_from_tree(ParseTree * typetree)
{
  // typetree is a possibly NULL 
  // parsetree representing a type.
  // return the correct jasmin string for this type.

  if (!typetree)
    return "V";  // void if the tree is NULL
  //cout<<" in JASMIN_TYPE var->children[0] = "<<typetree->children[0]<<endl;
  return decaf_type_to_jasmin_type(get_type_name(typetree));
  
}

string decaf_type(ParseTree * exprTree)
{
  

  // tree represents an expression  (actual parameter, rvalue, etc)
  // return the decaf type (string) determined
  // during pass 3 of semantic type-checking
  // FIXME:
  return exprTree->stype;
}

string jasmin_binop_instruction(string op, string type)
{
  string instr;
  if (type=="Bool") {
    if (op=="&&")
      return "iand";
    else if (op=="||")
      return "ior";
    else {
      cout << "unimplemented binary operator " << op << " for type " << type << endl;
      exit(1);
    }
  }
  else if (type=="Integer" || type=="Double") {
    if (op=="+")
      instr = "add";
    else if (op=="-")
      instr = "sub";
    else if (op=="*")
      instr = "mul";
    else if (op=="/")
      instr = "div";
    else if (op=="%")
      instr = "div";
    else {
      cout << "unimplemented binary operator " << op << " for type " << type << endl;
      exit(1);
    }
    if (type=="Integer")
      return "i"+instr;
    else
      return "d"+instr;
  }
  // STILL NEED TO DO STRING + USER DEFINED TYPES
  
  

  
  cout << "unimplemented type " << type << " in jasmin_binop_instruction." << endl;
  exit(1);
}


string load_mnemonic(string typestr)
{
  // Given a type, return the correct load mnemonic (istore, dstore, astore)
  // FIXME!!  Right now only ints are allowed!!!
  
  //cout << "In store_mnemonic with " << typestr << endl;

  if (typestr == "Integer"||typestr == "Bool")
    return "iload";
  else if (typestr == "Double")
    return "dload";
  else 
    return "aload";// Might have to rearrange this to work with user defined types
}

string store_mnemonic(string typestr)
{

  //cout << "In store_mnemonic with " << typestr << endl;
  if (typestr == "Integer"||typestr == "Bool")
    return "istore";
  else if (typestr == "Double")
    return "dstore";
  else 
    return "astore"; // Might have to rearrange this to work with user defined types
}

int bool_to_binary(ParseTree * expr) {
  if (expr->token->text=="true")
    return 1;
  return 0;
}

void generate_variable_load(string varname)
{
  S_variable *var=(S_variable *)currentScope->lookup(varname);
  // FIXME:  Current assumption, a local variable!
  
  

  //currentScope->print(cout); cout << "__________" <<endl;
  // if (var->instanceClass)
  //   cout << "VAR IN generate_variable_load = " << var->name << endl;  
  //cout << var->globalType << "|" << var->localType <<endl;

  if (var->globalType==true && var->localType==true) {
    cout << var->varnumber <<endl;
    cout << "BOTH LOCAL AND GLOBAL" << var->name<<endl;
  }
  if (var->globalType) {
    //if (var->localType) 
      //cout << "Variable name = " << var->name<<endl;
    *currentFile << "   getstatic             " << filename.substr(0,filename.size()-6)
		 << "/" << var->name << " "
		 << decaf_type_to_jasmin_type(var->type->name) << endl;  
  }
  
  else if (var->localType) {
    // generate something like: astore var-number
    string_simplifier(load_mnemonic(var->type->name), var->varnumber);
  }
  else if (var->instanceClass) {
    *currentFile << "   aload_0" << endl
		 << "   getfield              " 
		 << currentClassName
		 << "/" << var->name << " "
		 << decaf_type_to_jasmin_type(var->type->name) << endl;
  }
  else{ 
    cout << "name = " << var->name << endl;
    cout << "Variable type not defined in generate_variable_load"<<endl;  
  }

}
bool is_boolean_operation(ParseTree * expr) {
  string op = expr->children[1]->token->text;
  return (op=="<"||op==">"||op=="<="||op==">="||op=="=="||op=="!=");
}


string gensym() {
  static int next = 0;
  stringstream s;
  s << next++;
  return "LABEL" + s.str();
}


void boolean_operation_print_helper_integer(string cmp) {
  string label = gensym();
  string label2 = gensym();
  *currentFile << "   " << cmp << "             " << label << endl
	       << "   ldc                   1" << endl
	       << "   goto                  " << label2 << endl
	       << label << ":" << endl
	       << "   ldc                   0"<<endl
	       << label2 << ":" << endl;
}

void boolean_operation_print_helper_double(string cmp) {
  string label = gensym();
  string label2 = gensym();
  *currentFile << "   dcmpl" << endl
    
	       << "   " << cmp << "                  " << label << endl
	       << "   ldc                   1" << endl
	       << "   goto                  " << label2 << endl
	       << label << ":" << endl
	       << "   ldc                   0"<<endl
	       << label2 << ":" << endl;
}

int exp(int num1,int num2) {
  if (num2==0)
    return 1;
  return num1 * exp(num1,num2-1);
}

// int string_to_int(string str) 
// {
//   int temp = 0;
  
//   for (size_t i=0;i<str.size();i++) 
//     temp += atoi(str.substr(0,1)) * exp(10,i);
//   return temp

// }

void generate_boolean_operation(ParseTree * expr);
void generate_unop(ParseTree * tree);
void generate_newarray(ParseTree * tree);
void generate_arrayref(ParseTree * tree);
void generate_new(ParseTree * tree);

void generate_const(ParseTree * expr) {
  if (expr->description.substr(9)=="Bool")
    string_simplifier("ldc",bool_to_binary(expr->children[0]));
  else if (expr->description.substr(9)=="Double")
    *currentFile << "   ldc2_w                "<<expr->children[0]->token->text<<endl;
  else
    *currentFile << "   ldc                   "
		 << expr->children[0]->token->text << endl;
}

void generate_call_no_params(ParseTree * tree, bool check) 
{
  if (currentClass)
    *currentFile << "   invokevirtual         ";
  else
    *currentFile << "   invokestatic          ";
  
  
  *currentFile <<  (currentClassName==""?filename.substr(0,filename.size()-6):currentClassName)
	       << "/" << tree->children[0]->token->text << "()"
	       << decaf_type_to_jasmin_type(tree->children[0]->stype)
	       << endl;
  //	       << (check?"   pop\n":"");
}

void generate_call_params(ParseTree * tree, bool check) 
{
  if (currentClass)
    *currentFile << "   aload_0"<<endl;

  for (size_t i=0; i<tree->children[1]->children.size();i++) 
    generate_expr(tree->children[1]->children[i]);
  
  if (currentClass)
    *currentFile << "   invokevirtual         ";
  else
    *currentFile << "   invokestatic          ";
  
  //cout<<"LALALA"<<tree->children[0]->stype<<endl;

  *currentFile <<  (currentClassName==""?filename.substr(0,filename.size()-6):currentClassName)
    //	       <<  filename.substr(0,filename.size()-6)
	       << "/" << tree->children[0]->token->text <<"(";
  
  for (size_t i=0; i<tree->children[1]->children.size();i++) 
    *currentFile << decaf_type_to_jasmin_type(tree->children[1]->children[i]->stype);
  
  *currentFile << ")" << decaf_type_to_jasmin_type(tree->children[0]->stype) 
	       << endl;
  //	       << (check?"   pop\n":"");
}

void generate_varaccess_load(ParseTree * tree) 
{
  // S_variable * var = 
  //   (S_variable *)currentScope->lookup(tree->children[0]->children[0]->token->text);
  // if (!var->localType) {
  //   cout << "VARACCESS NOT LOCAL IN generate_varaccess_load"<<endl;
  //   exit(1);
  // }

  generate_expr(tree->children[0]);
  //  string_simplifier("aload", var->varnumber);
  *currentFile << "   getfield              "
	       << currentClassName << "/" 
	       << tree->children[1]->token->text
	       << " " 
	       << decaf_type_to_jasmin_type(tree->children[1]->stype)
	       << endl;




}

void generate_expr(ParseTree * expr)
{
  // Post:  generate code that will compute expr and leave 
  // it on the stack.

  // if (expr->type==NONTERMINAL)
  //   cout << "tree->description = " <<expr->description <<endl;
  
  // terminals:
  if (expr->type==TERMINAL and expr->token->type==T_IntConstant) {
    *currentFile << "   sipush " << expr->token->text << endl;
  }
  else if (expr->type==TERMINAL and expr->token->type==T_Identifier) {
    generate_variable_load(expr->token->text);
  }
  
  else if (expr->type==NONTERMINAL and expr->description=="call") {
    if (!expr->children[1])
	generate_call_no_params(expr, false);
    else 
      generate_call_params(expr,false);
  }
  else if (expr->type==NONTERMINAL and expr->description=="methodcall") {
    generate_methodcall(expr);
  }
  else if (expr->type==NONTERMINAL and expr->description=="arrayRef") 
    generate_arrayref(expr);

  else if (expr->type==NONTERMINAL and expr->description=="bExpr") {
    generate_expr(expr->children[0]);
    generate_expr(expr->children[2]);
    if (is_boolean_operation(expr)) {
      generate_boolean_operation(expr);
    }
    else {
      *currentFile << "   " 
		   << jasmin_binop_instruction(expr->children[1]->token->text,
					       decaf_type(expr->children[0]))
		   << endl;
    }
  }
  else if (expr->type==NONTERMINAL and expr->description=="uExpr") 
    generate_unop(expr);
  
  else if (expr->type==NONTERMINAL && expr->description.substr(0,9)=="constant_") 
    generate_const(expr);
  
  else if (expr->type==NONTERMINAL and expr->description=="newarray") 
    generate_newarray(expr);

  else if (expr->type==NONTERMINAL and expr->description=="new") 
    generate_new(expr);

  else if (expr->type==NONTERMINAL and expr->description=="varAccess") 
    generate_varaccess_load(expr);

  else {
    cout << "UNIMPLEMENTED in generate_expr : " << expr->description <<endl;
    exit(1);
  }
}

void generate_arrayref(ParseTree * tree)
{
  generate_expr(tree->children[0]);
  generate_expr(tree->children[1]);

  string retType = "";
  retType = tree->children[1]->stype;
  if (retType == "Integer" || retType == "Bool")
    retType = "i";
  else if (retType == "Double")
    retType = "d";
  else 
    retType = "a";
  *currentFile << "   " << retType + "aload" << endl;
}


void generate_newarray(ParseTree * tree) 
{
  generate_expr(tree->children[1]);
  string tname = tree->children[2]->stype;
  
  *currentFile << "   newarray              " 
	       << decaf_type_to_full_jasmin_type(tname)
	       << endl;
}

void generate_new(ParseTree * tree) 
{ 
  *currentFile << "   new                   " << tree->children[1]->token->text 
	       << endl
	       << "   dup" << endl
	       << "   invokespecial         " << tree->children[1]->token->text 
	       << "/<init>()V" << endl;
}

void generate_unop(ParseTree * tree) 
{
  generate_expr(tree->children[1]);
  string unop = tree->children[0]->token->text;
  
  if (unop == "!") {
    *currentFile << "   ldc                   1" << endl
		 << "   ixor " << endl;
    return;
  }
  // Negating int or double
  string ttype = tree->children[1]->stype;
  if (ttype == "Integer") {
    *currentFile << "   ineg" << endl;
    return;
  }
  *currentFile << "   dneg" << endl;
}

void generate_boolean_operation(ParseTree * expr) {
  string op = expr->children[1]->token->text;
  string type = expr->children[0]->stype;

  // generate_expr(expr->children[0]);
  // generate_expr(expr->children[2]);

  if (type=="Integer") {
    if (op=="<")
      boolean_operation_print_helper_integer("if_icmpge");
    else if (op=="<=")
      boolean_operation_print_helper_integer("if_icmpgt");
    else if (op==">")
      boolean_operation_print_helper_integer("if_icmple");
    else if (op==">=")
      boolean_operation_print_helper_integer("if_icmplt");
  }
  if (type=="Double"){
    if (op=="<")
      boolean_operation_print_helper_double("ifge");
    else if (op=="<=")
      boolean_operation_print_helper_double("ifgt");
    else if (op==">")
      boolean_operation_print_helper_double("ifle");
    else if (op==">=")
      boolean_operation_print_helper_double("iflt");
    else if (op=="==")
      boolean_operation_print_helper_double("ifne");
    else if (op=="!=")
      boolean_operation_print_helper_double("ifeq");
    else {
      cout << "Invalid boolean operation used on Double" << endl;
      exit(1);
    }
      
  }
  
  else if (op=="==")
    boolean_operation_print_helper_integer("if_icmpne");
  else if (op=="!=")
    boolean_operation_print_helper_integer("if_icmpeq");

}

void generate_Print_expr(ParseTree * expr)
{
  // generates a print statement
  // for Print(expr).
  *currentFile << "   getstatic             java/lang/System/out "
   "Ljava/io/PrintStream;" << endl;
  generate_expr(expr);
  *currentFile << "   invokevirtual         java/io/PrintStream/println("
               << decaf_type_to_jasmin_type(decaf_type(expr)) << ")V" << endl;
}


void generate_Print(ParseTree * printstmt) {
  
  ParseTree *exprs = printstmt->children[1];
  for (size_t i=0;i<exprs->children.size();i++) 
    generate_Print_expr(exprs->children[i]);
  // followed by a println...
  // *currentFile << "   getstatic             java/lang/System/out Ljava/io/PrintStream;" << endl;
  // *currentFile << "   invokevirtual         java/io/PrintStream/println()V" << endl;
}

void generate_break(ParseTree * tree) {
  if (breakstr=="NULL") {
    cout << "Break statement not properly implemented" << endl;
    exit(1);
  }
  *currentFile << "   goto                  " << breakstr << endl;
}

void generate_return(ParseTree * tree) {
  generate_expr(tree->children[1]);
  string ret = "return";
  string retType = "";
  if (tree->children[1]) {
    retType = tree->children[1]->stype;
    if (retType == "Integer" || retType == "Bool")
      retType = "i";
    else if (retType == "Double")
      retType = "d";
    else 
      retType = "a";
  }
  ret = retType + ret;
  *currentFile << "   " << ret << endl;
  returning = true;
}

void generate_array_store(ParseTree * tree) {
  generate_expr(tree->children[0]->children[0]);
  generate_expr(tree->children[0]->children[1]);
  generate_expr(tree->children[2]);

  tree = tree->children[0];

  //generate_expr(tree->children[1]);
  string store = store_mnemonic(tree->children[0]->stype);
  string retType = "";
  if (tree->children[1]) {
    retType = tree->children[1]->stype;
    if (retType == "Integer" || retType == "Bool")
      retType = "i";
    else if (retType == "Double")
      retType = "d";
    else 
      retType = "a";
    
  }
  store = retType + store;
  *currentFile << "   " << store << endl;
}



void generate_main_java_function(string mainClassName)
{
  *currentFile << ".method                  public static main([Ljava/lang/String;)V" << endl;
  *currentFile << "   .limit stack          0" << endl;
  *currentFile << "   .limit locals         1" << endl;
  *currentFile << "   invokestatic          " << mainClassName << "/main()V" << endl;
  *currentFile << "   return                " << endl;
  *currentFile << ".end method " << endl << endl;
}

void generate_statement(ParseTree * statement);

void generate_open_if(ParseTree * tree) {
  ParseTree * commonif = tree->children[0];
  string gens = gensym();
  string gens2 = gensym();
  //cout << " open_common_if->description "<<commonif->description<<endl;
  generate_expr(commonif->children[1]);
  *currentFile << "   ifeq                  " << gens<<endl;
  generate_statement(tree->children[1]);
  if (tree->children.size()>2) {
    *currentFile << "   goto                  " << gens2 << endl;
    *currentFile << gens << ":" << endl;
    cout << "hey" <<endl;
    generate_statement(tree->children[3]);
    *currentFile << gens2 << ":" << endl;
  }
  else 
    *currentFile << gens << ":" << endl;
}

void generate_matched_if(ParseTree * tree) {
  ParseTree * commonif = tree->children[0];
  generate_expr(commonif->children[1]);
  string gens = gensym();
  string gens2 = gensym();
  *currentFile << "   ifeq                  " << gens<<endl;
  generate_statement(tree->children[1]);
  *currentFile << "   goto                  " << gens2 << endl;
  *currentFile << gens << ":" << endl;
  generate_statement(tree->children[3]);
  *currentFile << gens2 << ":" << endl;
}

void generate_matched_while(ParseTree * tree) {
  ParseTree * commonwhile = tree->children[0];
  string gens = gensym();
  string gens2 = gensym();
  breakstr = gens2;
  *currentFile << gens << ":" << endl;
  generate_expr(commonwhile->children[1]);
  *currentFile << "   ifeq                  " << gens2<<endl;
  generate_statement(tree->children[1]);
  *currentFile << "   goto                  " << gens << endl;
  *currentFile << gens2 << ":" << endl;
  breakstr = "NULL";
}

void generate_matched_for(ParseTree * tree) {
  ParseTree * commonfor = tree->children[0];
  string gens = gensym();
  string gens2 = gensym();
  breakstr = gens2;
  generate_statement(commonfor->children[1]);
  *currentFile << gens << ":" << endl;
  generate_expr(commonfor->children[2]);
  *currentFile << "   ifeq                  " << gens2<<endl;
  generate_statement(tree->children[1]);
  generate_statement(commonfor->children[3]);
  *currentFile << "   goto                  " << gens << endl;
  *currentFile << gens2 << ":" << endl;
  breakstr = "NULL";
}

void generate_varaccess_store(ParseTree * tree) 
{
  // tree->description = "varAccess"
  S_variable * var = 
    (S_variable *)currentScope->lookup(tree->children[0]->children[0]->token->text);
  if (!var->localType) {
    cout << "VARACCESS NOT LOCAL IN generate_varaccess_store"<<endl;
    exit(1);
  }
  generate_expr(tree->children[0]->children[0]);
  //  string_simplifier("aload", var->varnumber);
  generate_expr(tree->children[2]);
  *currentFile << "   putfield              "
	       << currentClassName << "/" 
	       << tree->children[0]->children[1]->token->text
	       << " " 
	       << decaf_type_to_jasmin_type(tree->children[0]->children[1]->stype)
	       << endl;
}

void generate_methodcall(ParseTree * tree)
{ // tree->description == "methodcall"
  //cout << "methodcall"<<endl;
  generate_expr(tree->children[0]);
  
  //decaf_type_to_jasmin_type(tree->children[0]->stype) 

//*currentFile << "   .line                 " << statement->children[0]->token->line<<endl;

  if (tree->children[2])
    for (size_t i=0;i<tree->children[2]->children.size();i++) 
      generate_expr(tree->children[2]->children[i]);

  S_class * tempClass = (S_class *)currentScope->lookup(tree->children[0]->stype);
  
  //cout << "hi"<<endl;

  string typet = "";
  S_function * tempVar = NULL;

  if (tempClass) {
    while (tempClass) {
      for (size_t i=0;i<tempClass->fields.size();i++) {
	
	tempVar = (S_function *)tempClass->fields[i];
	if (tempVar->kind()=="S_function")
	  if (tempVar->name == tree->children[1]->token->text) {
	    if (!tempVar->returnType) {
	      typet = "Void";
	      break;
	    }
	    else {
	      typet = tempVar->returnType->name;
	      //*currentFile	       << decaf_type_to_jasmin_type(typet) <<endl;
	      //tempClass=NULL;
	      break;
	    }
	  } 
      }
      tempClass=tempClass->parentClass;
    }
  }
  else
    typet = tree->children[0]->children[1]->stype;
  

  //cout << "TYPET = "<<typet<<endl;
  //  cout << "TYPET2 = "<<t<<endl;

  *currentFile << "   invokevirtual         " << tree->children[1]->stype
	       << "/" << tree->children[1]->token->text 
	       << (tree->children[2]?jasmin_actuals(tree->children[2]):"()")
	       << decaf_type_to_jasmin_type(typet) <<endl;
  
  
  //*currentFile << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<endl;
  // *currentFile << jasmin_type_from_tree(tree->children[0]) ;
  // cout << "yup3" <<endl;
  //*currentFile	       << decaf_type_to_jasmin_type(tree->children[0]->stype) <<endl;
  //cout << "yup3" <<endl;

}


void generate_statement(ParseTree * statement)
{  
  if (!statement)
    return;
  
  if (statement->description=="stmts") {
    for (size_t i=0;i<statement->children.size();i++)
      generate_statement(statement->children[i]);
  }
  else if (statement->description=="stmtblock") {
    generate_statement(statement->children[1]);
  }
  else if (statement->description=="print") {
    *currentFile << "   .line                 " << statement->children[0]->token->line<<endl;
    generate_Print(statement);
  }
  else if (statement->description=="break") {
    *currentFile << "   .line                 " << statement->children[0]->token->line<<endl;
    generate_break(statement);
  }
  else if (statement->description=="call") {
    *currentFile << "   .line                 " << statement->children[0]->token->line<<endl;
    if (!statement->children[1])
      generate_call_no_params(statement, true);
    else 
      generate_call_params(statement, true);
  }

  else if (statement->description=="methodcall") {
    
    generate_methodcall(statement);
  }

  else if (statement->description=="return") {
    *currentFile << "   .line                 " << statement->children[0]->token->line<<endl;
    generate_return(statement);
  }
  else if (statement->description=="bExpr") {
    if (statement->children[1]->type==TERMINAL){
      
      *currentFile << "   .line                 " << statement->children[1]->token->line<<endl;
    }
    if (statement->children[1]->token->text=="=") {
      if (statement->children[0]->type==TERMINAL) {
	
	S_variable * var = 
	  (S_variable *)currentScope->lookup(statement->children[0]->token->text);
	if (var->globalType) {
	  generate_expr(statement->children[2]);
	  *currentFile << "   putstatic             " << filename.substr(0,filename.size()-6)
		       << "/" << var->name << " "
		       << decaf_type_to_jasmin_type(var->type->name) << endl;
	  
	}
	else if (var->localType) {
	  generate_expr(statement->children[2]);
	  // generate something like: astore var-number
	  string_simplifier(store_mnemonic(var->type->name),var->varnumber);
	}
	else if (var->instanceClass) {
	  *currentFile << "   aload_0" << endl;
	  generate_expr(statement->children[2]);
	  *currentFile << "   putfield              " << currentClassName
		       << "/" << var->name << " "
		       << decaf_type_to_jasmin_type(var->type->name) << endl;
	  
	  
	}
	else 
	  cout << "Variable type not defined"<<endl;
      }
      
      else if (statement->children[0]->description=="arrayRef") {
	generate_array_store(statement);
	// Might need to call generate_expr ?
      }
      else if (statement->children[0]->description=="varAccess") {
	cout << "In statement var access"<<endl;
	generate_varaccess_store(statement);
      }
    }
  }
  else if (statement->description=="open_if") 
    generate_open_if(statement); 
  
  else if (statement->description=="matched_if") {
    *currentFile << "   .line                 " << statement->children[0]->children[0]->token->line<<endl;
    generate_matched_if(statement); 
  }
  else if (statement->description=="matched_while") {
    *currentFile << "   .line                 " << statement->children[0]->children[0]->token->line<<endl;
    generate_matched_while(statement); 
  }
  else if (statement->description=="matched_for") 
    generate_matched_for(statement); 
  
  else {
    cout << "UNIMPLEMENTED in generate_statement: " << statement->description << endl;
    exit(1);
  }
  return;
}

void generate_block(ParseTree * block)
{
  //cout << "hi1"<<endl;
  if (!block->children[1])
    return;
  Symtab *oldScope = currentScope;
  currentScope = block->symtab;
  ParseTree * statements = block->children[1];
  //cout << "hi2"<<endl;
  for (size_t i=0;i<statements->children.size();i++) {
    //cout << "hi i = "<<i<<endl;
    generate_statement(statements->children[i]);
  }
  currentScope = oldScope;
  //cout << "hi3"<<endl;
}
  
void generate_static_field(ParseTree * topTree)
{
  for (size_t i=0;i<top->children.size();i++) {
    ParseTree * tree = topTree->children[i];
    if (tree->description=="variabledecl") {
      *currentFile << ".field                   static ";
      *currentFile << tree->children[0]->children[1]->token->text << " " 
		   << decaf_type_to_jasmin_type(tree->children[0]->children[0]->stype)
		   << "\n";
    }
    
  }
}
void generate_preamble(string className, string superClassName, int line, bool field, ParseTree * implements, ParseTree * currTree)
{
  // write the jasmin header for a class.
  // superClassName might be the empty string if there is
  // no superclass.
  // 
    
  *currentFile << ".source                  " <<filename << endl
	       << ".class                   " << (field?"":"public ") << className << endl
               << ".super                   " 
               <<  (superClassName==""?"java/lang/Object":superClassName)
               << endl;
               
    
  
  if (implements)
    for (size_t i = 0; i<implements->children[1]->children.size();i++)
      *currentFile << ".implements              " 
		   << implements->children[1]->children[i]->token->text << endl;
  
  *currentFile << endl;

    

  if (field) {
    S_class * tempClass = (S_class *)currentScope->lookup(className);
    if (tempClass->kind() != "S_class") {
      cout << "Generate_Preamble provided name of non-class object"<<endl;
      exit(1); }
    for (size_t i=0;i<tempClass->fields.size();i++) {
      S_variable * tempVar = (S_variable *)tempClass->fields.at(i);
      if (tempVar->kind() == "S_variable"){
	if (tempVar->globalType) {
	  S_type * varType = tempVar->type;
	  *currentFile << ".field                   static" << tempVar->name << " "
		       << decaf_type_to_jasmin_type(varType->name) << endl;
	}
	else {
	  S_type * varType = tempVar->type;
	  *currentFile << ".field                   " << tempVar->name << " "
		       << decaf_type_to_jasmin_type(varType->name) << endl;
	}
      }
    }
    *currentFile << endl;
  }
  else   
    generate_static_field(top);
    
  
  
  

  *currentFile <<endl;


    // constructor!
  *currentFile << ".method                  " << (field?"":"public ") 
	       << "<init>()V" << endl;
  *currentFile << "   .limit stack          1" << endl;
  *currentFile << "   .limit locals         1" << endl;
  *currentFile << "   .line                 " << line << endl;
  *currentFile << "   aload_0" << endl;
  if (superClassName!="")
    *currentFile << "   invokespecial         " <<superClassName;
  else
    *currentFile << "   invokespecial         java/lang/Object";
  *currentFile << "/<init>()V" << endl;
  *currentFile << "   return" << endl;
  *currentFile << ".end method" << endl << endl;
}

void generate_function(ParseTree * function)
{
  // generate a jasmin method.
  // it will be static if currentClass is NULL.
  
  
  
  string functionName = function->children[1]->token->text;
  *currentFile << ".method                  public " 
	       << (currentClass==NULL?"static ":"")
               << functionName 
               << (function->children[2]?jasmin_formals(function->children[2]):"()")
               << jasmin_type_from_tree(function->children[0]) << endl;
  *currentFile << "   .limit stack          10" << endl; // FIXME
  *currentFile << "   .limit locals         10" << endl; // FIXME
  
  

  generate_block(function->children[3]);
  
  // need to generate a return here.
  // for void functions, it's easy.
  // for others???  Not so sure right now.
  // FIXME!!

  //*currentFile << "   return" << endl;

  

  if (!returning) {
    *currentFile << "   return" << endl;
  }
  returning = false;

  *currentFile << ".end method" << endl << endl;    
  
}


void generate_class_block(ParseTree * tree) {
  if (!tree)
    ;
  else if (tree->description == "funcdecl")
    generate_function(tree);
  
}

void generate_class(ParseTree * tree)
{ //tree->description == "classdecl"
  string oldClassName = currentClassName;
  currentClassName = tree->children[1]->token->text;
  
  S_class * oldClass = currentClass;
  currentClass = (S_class *)currentScope->lookup(currentClassName);
  ofstream * oldFile = currentFile;
  currentFile = new ofstream((currentClassName+".j").c_str());
  Symtab * oldScope = currentScope;
  //currentScope = tree->symtab;
  
  //cout << "In generate class 1" << endl;

  generate_preamble(currentClassName, (tree->children[2]?tree->children[2]->children[1]->token->text:""), tree->children[1]->token->line, true, tree->children[3], tree);

  //cout << "In generate class 2" << endl;
  
  if (tree->children[4])
    for (size_t i=0;i<tree->children[4]->children.size();i++) 
      generate(tree->children[4]->children[i]);

  
  //cout << "In generate class 3" << endl;
  
  currentFile = oldFile;
  currentClass = oldClass;
  currentScope = oldScope;

  const char * temp = ("jasmin "+currentClassName+".j").c_str();
  system(temp);

  currentClassName = oldClassName;

}

void generate_interface_method(ParseTree * tree) 
{
  
  string prototypeName = tree->children[1]->token->text;

  *currentFile << ".method                  public abstract " 
    //<< (currentClass==NULL?"static ":" ")
               << prototypeName 
	       << (tree->children[2]?jasmin_formals(tree->children[2]):"()")
	       << jasmin_type_from_tree(tree->children[0]) << endl
	       << ".end method" << endl << endl;


}

void generate_interface(ParseTree * tree)
{
  string interfaceName = tree->children[1]->token->text;

  //S_class * oldClass = currentClass;
  //currentClass = (S_class *)currentScope->lookup(className);
  ofstream * oldFile = currentFile;
  currentFile = new ofstream((interfaceName+".j").c_str());
  //  Symtab * oldScope = currentScope;
  
  *currentFile << ".source                  " << interfaceName << ".java" <<endl
	       << ".interface               public abstract " << interfaceName << endl
	       << ".super                   java/lang/Object" << endl //NEED TO FIX
	       << endl
	       << endl;
  ParseTree * interfaceTree = tree->children[2];
  
  if (interfaceTree) 
    for (size_t i = 0; i < interfaceTree->children.size(); i++) 
      generate_interface_method(interfaceTree->children[i]);
  
  
  
  
  currentFile = oldFile;

  const char * temp = ("jasmin "+interfaceName+".j").c_str();
  system(temp);

}


void generate(ParseTree * tree)
{
  if (!tree)
    return;
  //cout << "In generate with " << tree->description << endl;
  if (tree->symtab) {
    //cout << "In generate, symtab with " << tree->description << endl;
    currentScope = tree->symtab;}
  if (tree->description=="funcdecl")
    generate_function(tree);
  else if (tree->description=="classdecl") {
    cout << "class: "<<tree->children[1]->token->text<<endl;
    generate_class(tree);
  }
  else if (tree->description=="interface")
    generate_interface(tree);
  else 
    for (size_t i=0;i<tree->children.size();i++)
      generate(tree->children[i]);
}


extern FILE * yyin;


int main(int argc, char **argv)
{
  /* Make sure there's a given file name */
  if (argc != 2) {
    cout << "USAGE: " << argv[0] << " FILE" << endl;
    exit(1);
  }       
  filename = argv[1];
  yyin = fopen(argv[1], "r");
  /* and that it exists and can be read */
  if (!yyin) {
    cout << argv[1] << ": No such file or file can't be opened for reading."
         << endl;
    exit(1);
  }

  yyparse();
  currentClass = NULL;
  currentInterface = NULL;
  currentFunction = NULL;
  traverseTree(top,0,0);

  // Semantics:

  currentScope = NULL;
  openscope();
  //cout << "DONE Pass 0" << endl;
  pass1(top);
  //cout << "DONE Pass 1" << endl;
  pass2(top);
  //cout << "DONE Pass 2" << endl;
  pass3(top);

  //closescope();
  

  // Code generation
  
  

  string className = string(argv[1]);
  
  className = className.substr(0,className.size()-6);  // strip .decaf
  
  topClassFile = new ofstream((className+".j").c_str());
  currentClass = NULL;
  
  currentFile = topClassFile;
  generate_preamble(className, "", 0, false, NULL,top);


  generate(top);

  generate_main_java_function(className);

  

  const char * temp = ("jasmin "+className+".j").c_str();
  system(temp);
  
 

  return 0;
}

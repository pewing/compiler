#include <map>
#include "parsetree.h"


struct semantics {
  // pure virtual function:
  virtual string kind() = 0;
  // at least one pure virtual function makes the class abstract 
  // (not instantiable.)

};

struct S_type : public semantics {
  virtual string kind() { return "S_type"; }
  string name; // usually a decaf symbol, but sometimes 
  // a type.  name can't be void.
};

struct S_class;

struct S_variable : public semantics {
  S_type * type;
  S_class * instanceClass;
  bool globalType;
  bool localType;
  int varnumber;
  string name;
  
  virtual string kind() { return "S_variable"; }
  /* bool isGlobal() {return globalType;} */
  /* bool isLocal() {return localType;} */
};

/* S_variable::bool globalType = false; */
/* S_variable::bool localType = false; */
/* S_variable::bool instanceType = false; */

struct S_function : public semantics {
  virtual string kind() { return "S_function"; }
  string name;
  vector<S_variable *> formals;
  S_type * returnType;  // NULL for a void function
};

struct S_interface : public semantics {
  virtual string kind() { return "S_interface"; }
  vector<S_function *> functions;
};

struct S_class : public semantics {
  virtual string kind() { return "S_class"; }
  S_class * parentClass;  // extends 
  vector<S_interface *> interfaces; // implements
  vector<semantics *> fields;  // each has to be S_function or S_variable
};

struct S_builtin : public semantics {
  virtual string kind() { return "S_builtin"; }
};


typedef map <string, semantics *> Dictionary;

struct Symtab {
  // A class to represent symbol tables
  // Chained together to represent nested scopes.
 public:
  Dictionary dict;
  Symtab(Symtab *p);
  semantics * lookup(string key);
  semantics * local_lookup(string key);
  
  void insert(string key, semantics * item);

  void print(ostream & ostr);

  Symtab * parent;  // outer scope
};

Symtab *closescope();
void openscope();
void semantic_error(string err, int line);
string get_type_name(ParseTree *typetree);
string get_type_name_no_array(ParseTree * typetree);
void error_if_defined_locally(ParseTree *tree);
bool class_occurs(S_class * subclass, S_class * superclass);
bool matching_classes(S_class * sub, S_class * sup);
void makeClass(ParseTree * tree);
void makeInterface(ParseTree * tree);
void makePrototype(ParseTree * tree);
void makeFunction(ParseTree * tree);
S_variable * makeVariable(ParseTree *tree);
void pass1(ParseTree * tree);
void pass1_5(ParseTree * tree);
bool cycleChecker(S_class * sub, S_class * sup);
void pass2(ParseTree * tree);
void pass2funcdecl(ParseTree * tree);
void pass2classdecl(ParseTree * tree);
void pass2vardecl(ParseTree * tree);
void pass2fielddecl(ParseTree * tree);
void makeStmtBlock(ParseTree * tree);
void pass3(ParseTree * tree);
void pass3funcdecl(ParseTree * tree);
void pass3classdecl(ParseTree * tree);
void pass3binop(ParseTree * tree);
void pass3fielddecl(ParseTree * tree);
void pass3varaccess(ParseTree * tree);
void pass3arrayref(ParseTree * tree);
void pass3unop(ParseTree * tree);
void pass3call(ParseTree * tree);
void pass3methodcall(ParseTree * tree);
void pass3open(ParseTree * tree);
void pass3matched(ParseTree * tree);
void pass3token(ParseTree * tree);
void pass3return(ParseTree * tree);
void pass3expr(ParseTree * tree);
void pass3interface(ParseTree * tree);
void pass3newarray(ParseTree * tree);
void pass3print(ParseTree * tree);
void pass3pexpr(ParseTree * tree);



template <typename A, typename B, typename C>
  struct triple {
    A first;
    B second;
    C third;
  triple(const A & f, 
	 const B & s, 
	 const C & t) : first(f), second(s), third(t) { }
  };

typedef triple<S_class *, S_class *, int> sstriple;

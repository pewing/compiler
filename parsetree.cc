#include "parsetree.h"

ParseTree::ParseTree(string description) : symtab(NULL), type(NONTERMINAL), 
                                  description(description) {}


ParseTree::ParseTree(Token * tokp) : symtab(NULL), type(TERMINAL), 
				     token(tokp) {}

void ParseTree::addChild(ParseTree * tree) {
  children.push_back(tree);
}

string ParseTree::toString() {
  string answer = "";
  if (type==TERMINAL)
    answer +=  token->toString();
  else {
    answer += "(" + description;
    for (vector<ParseTree *>::iterator i=children.begin(); 
         i != children.end(); i++) {
      ParseTree *tree = *i;
      if (!tree) answer += " NULL";
      else answer += " " + tree->toString();
    }
    answer += ")";
  }
  return answer;
}

  
ParseTree *top;

extern Token *myTok;
extern int yylineno;
int yylex();


extern stack<Token *> opStack;

string base26(int x)
{
    char buf[2];
    buf[1] = 0;
    if (x<26) {
        buf[0] = string("abcdefghijklmnopqrstuvwxyz")[x];
	return string(buf);
    }
    return base26(x/26) + base26(x % 26);
}

string seq2str(int seq, int depth)
{
    // depth 1: A, B, C, D....
    // depth 2: 1, 2, 3, 4....
    // depth 3: a, b, c, d....
    // depth 4: 1, 2, 3, 4....
    char buf[100];
    int d = (depth - 1) % 4;
    if (d == 1 || d == 3) {
        sprintf(buf, "%d", seq);
        return string(buf);
    }
    string let = base26(seq-1);
    if (d==0) {
        string bob = "";
	for (string::iterator i = let.begin(); i!=let.end(); i++)
	    bob += toupper(*i);
	let = bob;
    }
    return let;
    
}

void traverseTree(ParseTree * tree, int depth, int seq) {
     // tree is a possibly null tree, but output always occurs
     for (int i=0;i<depth;i++)
     	 cout << "   ";
     if (seq) cout << seq2str(seq, depth) << " ";
     if (!tree) { 
         cout << "NULL" << endl; 
	 return; 
     }
     if (tree->type == TERMINAL) {
         cout << tree->token->toString() << endl; 
	 return; 
     }
     cout << tree->description << endl;
     for (size_t i=0;i<tree->children.size();i++)
     	 traverseTree(tree->children[i], depth+1, i+1);
}


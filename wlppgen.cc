// Starter code for CS241 assignments 9-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
using namespace std;

map<string, string> symTable; //symbol Table for identifiers
map<string, int> offsetTable; //offset Table for identifiers
string type, name, id;
map<string, string>::iterator found, it;
int idUse = 0; // to determine if it is a use of identifier
size_t isfound;
int offset;
int labelN = 0;
int labelI = 0;

// The set of terminal symbols in the WLPP grammar.
const char *terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID",
  "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM",
  "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI",
  "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW",
  "DELETE", "NULL"
};
int isTerminal(const string &sym) {
  int idx;
  for(idx=0; idx<sizeof(terminals)/sizeof(char*); idx++)
    if(terminals[idx] == sym) return 1;
  return 0;
}

// Data structure for storing the parse tree.
class tree {
  public:
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

void genCode(tree *t);

// Call this to display an error message and exit the program.
void bail(const string &msg) {
  // You can also simply throw a string instead of using this function.
  throw string(msg);
}

// Read and return wlppi parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  
  getline(cin, line);
  if(cin.fail())
    bail("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  stringstream ss;
  ss << line;
  while(!ss.eof()) {
    string token;
    ss >> token;
    if(token == "") continue;
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(int idx=0; idx<ret->tokens.size(); idx++) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); idx++) {
      ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
}

tree *parseTree;

// Compute symbols defined in t.
void genSymbolTable(tree *t) {
   
   if (t->tokens[0] == "type") {
      for(vector<string>::iterator it = t->tokens.begin()+1; it != t->tokens.end(); it++) {
         if (*it == "INT") type += "int";
         if (*it == "STAR") type += "*"; 
      }
   }
   
   if (t->tokens[0] == "ID") {
      name = t->tokens[1];
      if (!idUse) {
						   found = symTable.find(name);
						   if (found != symTable.end()) bail("ERROR: " + name + " was defined more than once");
						   symTable.insert(pair<string, string>(name, type));
						   type = "";
						   name = "";
      } else {
         found = symTable.find(name);
         if (found == symTable.end()) bail("ERROR: " + name + " was not defined");
         idUse = 0;
      }
   }
   
   if ((t->tokens[0] == "lvalue") || (t->tokens[0] == "factor")) {
      idUse = 1;
   }
   
   for(vector<tree *>::iterator it = t->children.begin(); it != t->children.end(); it++) {
      genSymbolTable(*it);
   }
}

string getType(tree *t) {
   
   string type = "";
   
   if (t->tokens[0] == "NUM") return "int";
   
   if (t->tokens[0] == "NULL") return "int*";
   
   if ((t->tokens[0] == "factor") || (t->tokens[0] == "lvalue")) {
      
      isfound = t->rule.find("AMP");
      if (isfound != string::npos) {
         if (getType(t->children[1]) == "int") return "int*";
         else bail("ERROR: Type Unmatch1");
      }
      
      isfound = t->rule.find("STAR");
      if (isfound != string::npos) {
         if (getType(t->children[1]) == "int*") return "int";
         else bail("ERROR: Type Unmatch2");
      }
      
      isfound = t->rule.find("NEW");
      if (isfound != string::npos) {
         if (getType(t->children[3]) == "int") return "int*";
         else bail("ERROR: Type Unmatch3");
      }
   }
   
   if (t->tokens[0] == "ID") {
      
      name = t->tokens[1];
      it = symTable.find(name);
      return it->second;
      
   }
   
   if (t->tokens[0] == "expr") {
      
      isfound = t->rule.find("PLUS");
      if (isfound != string::npos) {
         if ((getType(t->children[0]) == "int") && (getType(t->children[2]) == "int"))  return "int";
         if ((getType(t->children[0]) == "int*") && (getType(t->children[2]) == "int")) return "int*";
         if ((getType(t->children[0]) == "int") && (getType(t->children[2]) == "int*")) return "int*";
         else bail("ERROR: Type Unmatch4");
      }
      
      isfound = t->rule.find("MINUS");
      if (isfound != string::npos) {
         if ((getType(t->children[0]) == "int") && (getType(t->children[2]) == "int")) return "int";
         if ((getType(t->children[0]) == "int*") && (getType(t->children[2]) == "int*")) return "int";
         if ((getType(t->children[0]) == "int*") && (getType(t->children[2]) == "int")) return "int*";
         else bail("ERROR: Type Unmatch5");
      }
      
   }
   
   if (t->tokens[0] == "term") {
      int numToken = t->children.size();
      
      if (numToken == 3) {
         if ((getType(t->children[0]) == "int") && (getType(t->children[2]) == "int")) return "int";
         else bail("ERROR: Type Unmatch6");
      }
   }
   
   if (t->tokens[0] == "procedure") {
      if ((getType(t->children[5]) == "int*") || (getType(t->children[11]) == "int*")) bail("ERROR: Type Unmatch7");
      
   }
   
   if (t->tokens[0] == "dcls") {
   
      isfound = t->rule.find("BECOMES");
      if (isfound != string::npos) {
         if (getType(t->children[1]) != getType(t->children[3])) bail("ERROR: Type Unmatch8");
      }
   }
   
   if (t->tokens[0] == "statement") {
      
      isfound = t->rule.find("BECOMES");
      if (isfound != string::npos) {
         if (getType(t->children[0]) != getType(t->children[2])) bail("ERROR: Type Unmatch9");
      }
      
      isfound = t->rule.find("PRINTLN");
      if (isfound != string::npos) {
         if (getType(t->children[2]) != "int") bail("ERROR: Type Unmatch10");
      }
      
      isfound = t->rule.find("DELETE");
      if (isfound != string::npos) {
         if (getType(t->children[3]) != "int*") bail("ERROR: Type Unmatch11");
      }
   }
   
   if (t->tokens[0] == "test") {
      
      if (getType(t->children[0]) != getType(t->children[2])) bail("ERROR: Type Unmatch12");
   }
   
   for (vector<tree *>::iterator it = t->children.begin(); it != t->children.end(); it++) {
      type += getType(*it);
   }
      
   return type;
   
}

// Generate the symbols for the parse tree t
void genSymbols(tree *t) {
}

void genCodeLT(tree *t1, tree *t2) {
   genCode(t1);
   cout << "add $5, $3, $0" << endl;
   genCode(t2);
   string E1Type = getType(t1);
   string E2Type = getType(t2);
   if (E1Type == "int*") cout << "sltu $3, $5, $3" << endl;
   else cout << "slt $3, $5, $3" << endl;
}

void genCodeNE(tree *t1, tree *t2) {
   genCodeLT(t1, t2);
   cout << "add $6, $3, $0" << endl;
   genCodeLT(t2, t1);
   cout << "add $3, $6, $3" << endl;
}

// Generate the code for the parse tree t.
void genCode(tree *t) {
  
  if (t->tokens[0] == "procedure") {
     string par1 = t->children[3]->children[1]->tokens[1];
     string par2 = t->children[5]->children[1]->tokens[1];
     offset = offsetTable[par1];
     cout << "sw $1, " << offset << "($29)" << endl;
     offset = offsetTable[par2];
     cout << "sw $2, " << offset << "($29)" << endl;
     string ftype = getType(t->children[3]->children[1]);
     
     if (ftype == "int") {
        cout << "add $2, $0, $0" << endl;
     }
     
     cout << "lis $3" << endl;
		   cout << ".word init" << endl;
		   cout << "sw $31, -4($30)" << endl;
		   cout << "sub $30, $30, $4" << endl;
		   cout << "jalr $3" << endl;
		   cout << "add $30, $30, $4" << endl;
		   cout << "lw $31, -4($30)" << endl;
  }
  
  if (t->rule == "expr expr PLUS term") {
     string EType = getType(t->children[0]);
     string TType = getType(t->children[2]);
     genCode(t->children[0]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[2]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     
     if (EType == "int*") {
        cout << "mult $3, $4" << endl;
        cout << "mflo $3" << endl;
     }
     
     if (TType == "int*") {
        cout << "mult $6, $4" << endl;
        cout << "mflo $6" << endl;
     } 
     
     cout << "add $3, $3, $6" << endl;
     return;
  }
  
  if (t->rule == "expr expr MINUS term") {
     string EType = getType(t->children[0]);
     string TType = getType(t->children[2]);
     genCode(t->children[0]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[2]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     
     if ((EType == "int*") && (TType == "int")) {
        cout << "mult $3, $4" << endl;
        cout << "mflo $3" << endl;
     }
     
     cout << "sub $3, $6, $3" << endl;
     
     if ((EType == "int*") && (TType == "int*")) {
        cout << "div $3, $4" << endl;
        cout << "mflo $3" << endl;
     }
     
     return;
  }  
  
  if (t->rule == "term term STAR factor") {
     genCode(t->children[0]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[2]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     cout << "mult $3, $6" << endl;
     cout << "mflo $3" << endl;
     return;
  }
  
  if (t->rule == "term term SLASH factor") {
     genCode(t->children[0]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[2]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     cout << "div $6, $3" << endl;
     cout << "mflo $3" << endl;
     return;
  }

  if (t->rule == "term term PCT factor") {
     genCode(t->children[0]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[2]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     cout << "div $6, $3" << endl;
     cout << "mfhi $3" << endl;
     return;
  }
  
  if (t->rule == "factor NUM") {
     cout << "lis $3" << endl;
     cout << ".word " << t->children[0]->tokens[1] << endl;
     return;
  }
  
  if (t->rule == "factor NULL") {
     cout << "add $3, $0, $0" << endl;
     return;
  }
  
  if (t->rule == "factor ID") {
     name = t->children[0]->tokens[1];
     offset = offsetTable[name];
     cout << "lw $3, " << offset << "($29)" << endl;
     return;
  }
  
  if (t->rule == "factor STAR factor") {
     genCode(t->children[1]);
     cout << "lw $3, 0($3)" << endl;
     return;
  }
  
  if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
     genCode(t->children[3]);
     cout << "add $1, $3, $0" << endl;
		   cout << "sw $31, -4($30)" << endl;
		   cout << "sub $30, $30, $4" << endl;
		   cout << "jalr $13" << endl;
		   cout << "add $30, $30, $4" << endl;
		   cout << "lw $31, -4($30)" << endl;
		   return;
  }
  
  if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
     genCode(t->children[3]);
     cout << "add $1, $3, $0" << endl;
		   cout << "sw $31, -4($30)" << endl;
		   cout << "sub $30, $30, $4" << endl;
		   cout << "jalr $14" << endl;
		   cout << "add $30, $30, $4" << endl;
		   cout << "lw $31, -4($30)" << endl;
		   return;
  }
  
  if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
     genCode(t->children[2]);
     cout << "add $1, $3, $0" << endl;
     cout << "sw $31, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     cout << "jalr $10" << endl;
     cout << "add $30, $30, $4" << endl;
     cout << "lw $31, -4($30)" << endl;
     return;
  }
  
  if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
     genCode(t->children[0]);
     id = t->children[1]->children[1]->tokens[1];
     offset = offsetTable[id];
     cout << "lis $5" << endl;
     cout << ".word " << t->children[3]->tokens[1] << endl;
     cout << "sw $5, " << offset << "($29)" << endl;
     return;
  }
  
  if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
     genCode(t->children[0]);
     id = t->children[1]->children[1]->tokens[1];
     offset = offsetTable[id];
     cout << "sw $0, " << offset << "($29)" << endl;
     return;
  } 
  
  if (t->rule == "statement lvalue BECOMES expr SEMI") {
     genCode(t->children[2]);
     cout << "sw $3, -4($30)" << endl;
     cout << "sub $30, $30, $4" << endl;
     genCode(t->children[0]);
     cout << "add $30, $30, $4" << endl;
     cout << "lw $6, -4($30)" << endl;
     cout << "sw $6, 0($3)" << endl;
     return;
  }
  
  if (t->rule == "lvalue ID") {
     name = t->children[0]->tokens[1];
     offset = offsetTable[name];
     cout << "lis $3" << endl;
     cout << ".word " << offset << endl;
     cout << "add $3, $29, $3" << endl;
     return;
  }
  
  if (t->rule == "test expr LT expr") {
     genCodeLT(t->children[0], t->children[2]);
     return;
  }
  
  if (t->rule == "test expr NE expr") {
     genCodeNE(t->children[0], t->children[2]);
     return;
  }
  
  if (t->rule == "test expr GT expr") {
     genCodeLT(t->children[2], t->children[0]);
     return;
  }
  
  if (t->rule == "test expr EQ expr") {
     genCodeNE(t->children[0], t->children[2]);
     cout << "sub $3, $3, $11" << endl;
     cout << "beq $3, $0, 1" << endl;
     cout << "add $3, $3, $12" << endl;
     return;
  }
  
  if (t->rule == "test expr LE expr") {
     genCodeLT(t->children[2], t->children[0]);
     cout << "sub $3, $3, $11" << endl;
     cout << "beq $3, $0, 1" << endl;
     cout << "add $3, $3, $12" << endl;
     return;
  }
  
  if (t->rule == "test expr GE expr") {
     genCodeLT(t->children[0], t->children[2]);
     cout << "sub $3, $3, $11" << endl;
     cout << "beq $3, $0, 1" << endl;
     cout << "add $3, $3, $12" << endl;
     return;
  }
  
  if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
     labelN++;
     cout << "start" << labelN << ":" << endl;
     genCode(t->children[2]);
     cout << "beq $3, $0, end" << labelN << endl;
     int tempL = labelN;
     genCode(t->children[5]);
     cout << "beq $0, $0, start" << tempL << endl;
     cout << "end" << tempL << ":" << endl;
     return;
  }
  
  if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
     labelI++;
     genCode(t->children[2]);
     cout << "beq $3, $0, else" << labelI << endl;
     int tmpL = labelI;
     genCode(t->children[5]);
     cout << "beq $0, $0, endif" << tmpL << endl;
     cout << "else" << tmpL << ":" << endl;
     genCode(t->children[9]);
     cout << "endif" << tmpL << ":" << endl;
     return;
  }
  
  for (vector<tree *>::iterator it = t->children.begin(); it != t->children.end(); it++) {
      genCode(*it);
  }
    
}

int main() {
  // Main program.
  
  try {
    parseTree = readParse("S");
    genSymbolTable(parseTree);
    //getType(parseTree);
    //prologue
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $10" << endl;
    cout << ".word print" << endl;
    cout << "sub $29, $30, $4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "lis $12" << endl;
    cout << ".word 2" << endl;
    cout << "lis $13" << endl;
    cout << ".word new" << endl;
    cout << "lis $14" << endl;
    cout << ".word delete" << endl;
    offset = 0;
    for(map<string, string>:: iterator it = symTable.begin(); it != symTable.end(); it++) {
       offsetTable.insert(pair<string,int>(it->first, offset));
       offset -= 4;
    }
    cout << "lis $5" << endl;
    cout << ".word " << offset << endl;
    cout << "add $30, $30, $5" << endl;
    
    genCode(parseTree);
    cout << "jr $31" << endl;
    
    //print Symbol Table
    //string temp = getType(parseTree);
    //cout << temp << endl;
    /*for(map<string, string>::iterator it = symTable.begin(); it != symTable.end(); it++) {
       cerr << it->first << " " << it->second << endl;
    }*/
    
  } catch(string msg) {
    cerr << msg << endl;
  }
  if (parseTree) delete parseTree;
  return 0;
}

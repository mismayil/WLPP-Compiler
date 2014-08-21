#include <set>
#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

set<string> terms;
set<string> nonterms;
vector<string> prods;
string start;
vector<string> actions;
vector<string> derivations;
stack<string> symStack;
stack<int> stateStack;
string input;
string token, LHS, temp, rule, lexeme;
vector<string> matchToken;
vector<string> leftDerivates;
int error = 0;

struct Tree {
    string rule;
    list<Tree*> children;

    ~Tree() {
        for(list<Tree*>::iterator it=children.begin(); it != children.end(); it++) {  // delete all subtrees
            delete (*it);
        }
    }
};

void readsyms(vector<string> &t, istream& is) {
    int n;
    string temp;
    getline(is,temp);
    istringstream iss(temp);
    iss >> n;
    for(int i = 0; i < n; i++) {
        getline(is,temp);
        t.push_back(temp);
    }
}

void readsyms(set<string> &t, istream& is) {
    int n;
    string temp;
    getline(is,temp);
    istringstream iss(temp);
    iss >> n;
    for(int i = 0; i < n; i++) {
        getline(is,temp);
        t.insert(temp);
    }
}

void traverse(Tree *t) {
    //leftDerivates.push_back(t->rule);
    //cout << t->rule << endl;
    istringstream iss(t->rule);
    string temp;
    vector<string>::iterator it;
    iss >> temp;
    while (iss >> temp) {
       it = matchToken.begin();
       istringstream ss(*it);
       ss >> token;
       if (temp == token) {
          cout << *it<< endl;
          matchToken.erase(it);
       } else {
          
          for(list<Tree*>::iterator it=(t->children).begin(); it != (t->children).end(); it++) {
             istringstream is((*it)->rule);
             is >> token;
             if (temp == token) {
             cout << (*it)->rule << endl;
             traverse(*it);
             break;
             }
          }
       }
    }

}

void dump(set<string> &h) {
    cout << h.size() << endl;
    for(set<string>::iterator it=h.begin(); it != h.end(); it++) {
        cout << *it << endl;
    }
}

void popper(stack<Tree *> &myStack, list<string> &rhs, string rule) {
    Tree *n = new Tree();
    n->rule = rule;
    for(list<string>::iterator it=rhs.begin(); it != rhs.end(); it++){
        Tree *tmp = myStack.top();
        n->children.push_front(tmp);
        myStack.pop();
    }
    myStack.push(n);
}

Tree* lrdo() {
    stack<Tree*> myStack;
    string l; // lhs symbol
    do {
        string f;
        for (vector<string>::iterator it = derivations.begin(); it != derivations.end(); it++) {
           f = *it;
						     list<string> r; // rhs symbols
						     istringstream iss(f);
						     iss >> l; // lhs symbol
						     string s;
						     while(iss >> s) {
						         if(nonterms.count(s) > 0) r.push_back(s); // only non-terminals
						     }
						     popper(myStack, r, f); // reduce rule
        }
    } while(start != l);
    return myStack.top();
}

int transit(int state, string token) {
		 ostringstream os;
			os << state;
			string transition = os.str() + " " + token;
			int nextState = -1;
			int stateNum;
			size_t found;
			string temp;
			
			for (vector<string>::iterator it = actions.begin(); it != actions.end(); it++) {
		
						stringstream ss(*it);
						ss >> stateNum;
						
						if (stateNum == state) {
									found = (*it).find(transition); 
		
									if (found != string::npos) {
								
												istringstream is(*it);
												is >> temp;
												is >> temp;
												is >> temp;
												is >> nextState;
												break;
						   }
		   }
		 }
		 
		 return nextState;
}

int reduce(int state, string token) {

			string keyword = token + " reduce";		   
   int ruleNum = transit(state, keyword);
   int numWords = 0;
   if (ruleNum != -1) {
   
      rule = prods[ruleNum];
						istringstream iss(rule);
						iss >> LHS;
															
						while (iss >> temp) {
									numWords++;
						}
			} else return ruleNum;
			
			return numWords;
					
}

void generateRR(string input) {
   
			istringstream iss(input);
			symStack.push("BOF");
			stateStack.push(0);
			stateStack.push(transit(0, "BOF"));
			
			int counter = 1;
			int numTokens;
			iss >> temp;
			
			while (iss >> token) {
      
						while (1) {
						
									numTokens = reduce(stateStack.top(), token);
									if (numTokens == -1) break;			
									for (int i = 0; i < numTokens; i++) {
												symStack.pop();
												stateStack.pop();
									}
												
									symStack.push(LHS);
									stateStack.push(transit(stateStack.top(), LHS));
									derivations.push_back(rule);
			   }
			
						symStack.push(token);
			
						if (transit(stateStack.top(), token) == -1) {
									cerr << "ERROR at " << counter << endl;
									error = 1;
									return;
						}
						counter++;
						stateStack.push(transit(stateStack.top(), token));
			}
			derivations.push_back(prods[0]);
}

int main(){
    
    fstream lang("wlpp.lr1");
    readsyms(terms, lang);
    readsyms(nonterms, lang);
    getline(lang,start);
    readsyms(prods, lang); 
    getline(lang, temp);
    readsyms(actions, lang);
    input = "BOF ";
    matchToken.push_back("BOF BOF");
    while (1) {
       getline(cin, temp);
       if (cin.fail()) break;
       istringstream is(temp);
       is >> token;
       is >> lexeme;
       input += token + " ";
       matchToken.push_back(temp);
    }
    matchToken.push_back("EOF EOF");
    input += "EOF";
    generateRR(input);
    if (!error) {
							Tree *parsetree = lrdo(); // read reverse rightmost derivation
							cout << "S BOF procedure EOF" << endl;
							traverse(parsetree); // write forward leftmost derivation
							delete parsetree;
    }
    return 0;
}

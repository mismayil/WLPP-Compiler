
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
using namespace std;

//======================================================================
//========= Declarations for the scan() function =======================
//======================================================================

// Each token has one of the following kinds.

enum Kind {
    ID,
    NUM,
    ZERO,                        
    LPAREN,             
    RPAREN,
    LBRACE,
    RBRACE,
    RETURN,
    IF,
    ELSE,
    WHILE,
    PRINTLN,
    WAIN,
    BECOMES,
    INT,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PCT,
    COMMA,
    SEMI,
    NEW,
    DELETE,
    LBRACK,
    RBRACK,
    AMP,
    NUL,
    WHITESPACE                
};

// kindString(k) returns string a representation of kind k
// that is useful for error and debugging messages.
string kindString(Kind k);

// Each token is described by its kind and its lexeme.

struct Token {
    Kind      kind;
    string    lexeme;
    // toInt() returns an integer representation of the token.
    int       toInt();
};

// scan() separates an input line into a vector of Tokens.
vector<Token> scan(string input);


// States for the finite-state automaton that comprises the scanner.

enum State {
   ST_NULL,
   ST_START,
   ST_ZERO,
   ST_ID,
   ST_NUM,
   ST_LPAREN,
   ST_RPAREN,
   ST_LBRACE,
   ST_RBRACE,
   ST_BECOMES,
   ST_EQ,
   ST_EXCLAIM,
   ST_NE,
   ST_LT,
   ST_LE,
   ST_GT,
   ST_GE,
   ST_PLUS,
   ST_MINUS,
   ST_STAR,
   ST_SLASH,
   ST_PCT,
   ST_COMMA,
   ST_SEMI,
   ST_LBRACK,
   ST_RBRACK,
   ST_AMP,
   ST_WHITESPACE,
   ST_COMMENT
};

// The *kind* of token (see previous enum declaration)
// represented by each state; states that don't represent
// a token have stateKinds == NUL.

Kind stateKinds[] = {
   NUL,
   NUL,
   ZERO,
   ID,
   NUM,
   LPAREN,
   RPAREN,
		 LBRACE,
		 RBRACE,
		 BECOMES,
		 EQ,
		 NUL,
		 NE,
		 LT,
		 LE,
		 GT,
		 GE,
		 PLUS,
		 MINUS,
		 STAR,
		 SLASH,
		 PCT,
		 COMMA,
		 SEMI,
		 LBRACK,
		 RBRACK,
		 AMP,
		 WHITESPACE,
   WHITESPACE
};

State delta[ST_COMMENT+1][256];

#define whitespace "\t\n\r "
#define letters    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define digits     "0123456789"
#define oneToNine  "123456789"

void setT(State from, string chars, State to) {
    for(int i = 0; i < chars.length(); i++ ) delta[from][chars[i]] = to;
}

void initT(){
    int i, j;

    // The default transition is ST_NUL (i.e., no transition
    // defined for this char).
    for ( i=0; i<=ST_COMMENT; i++ ) {
        for ( j=0; j<256; j++ ) {
            delta[i][j] = ST_NULL;
        }
    }
    // Non-null transitions of the finite state machine.
    // NB: in the third line below, letters digits are macros
    // that are replaced by string literals, which the compiler
    // will concatenate into a single string literal.
    setT( ST_START,      whitespace,     ST_WHITESPACE );
    setT( ST_WHITESPACE, whitespace,     ST_WHITESPACE );
    setT( ST_START,      "0",            ST_ZERO       );
    setT( ST_START,      oneToNine,      ST_NUM        );
    setT( ST_NUM,        digits,         ST_NUM        );
    setT( ST_START,      letters,        ST_ID         );
    setT( ST_ID,         letters,        ST_ID         );
    setT( ST_ID,         digits,      	  ST_ID         );
    setT( ST_START,      ",",            ST_COMMA      );
    setT( ST_START,      "(",            ST_LPAREN     );
    setT( ST_START,      ")",            ST_RPAREN     );
    setT( ST_START,      "{",            ST_LBRACE     );
    setT( ST_START,      "}",            ST_RBRACE     );
    setT( ST_START,      "=",            ST_BECOMES    );
    setT( ST_BECOMES,    "=",            ST_EQ         );
    setT( ST_START,      "!",            ST_EXCLAIM    );
    setT( ST_EXCLAIM,    "=",      	     ST_NE         );
    setT( ST_START,      "<",            ST_LT         );
    setT( ST_LT,         "=",            ST_LE         );
    setT( ST_START,      ">",            ST_GT         );
    setT( ST_GT,         "=",            ST_GE         );
    setT( ST_START,      "+",            ST_PLUS       );
    setT( ST_START,      "-",            ST_MINUS      );
    setT( ST_START,      "*",            ST_STAR       );
    setT( ST_START,      "/",            ST_SLASH      );
    setT( ST_START,      "%",            ST_PCT        );
    setT( ST_START,      ",",            ST_COMMA      );
    setT( ST_START,      ";",            ST_SEMI       );
    setT( ST_START,      "[",            ST_LBRACK     );
    setT( ST_START,      "]",            ST_RBRACK     );
    setT( ST_START,      "&",            ST_AMP        );
    setT( ST_SLASH,      "/",            ST_COMMENT    );
    
    for ( j=0; j<256; j++ ) delta[ST_COMMENT][j] = ST_COMMENT;
}

static int initT_done = 0;

vector<Token> scan(string input){
    // Initialize the transition table when called for the first time.
    if(!initT_done) {
        initT();
        initT_done = 1;
    }

    vector<Token> ret;

    int i = 0;
    int startIndex = 0;
    State state = ST_START;
    string temp;
    
    if(input.length() > 0) {
        while(true) {
            State nextState = ST_NULL;
            if(i < input.length())
                nextState = delta[state][(unsigned char) input[i]];
            if(nextState == ST_NULL) {
                // no more transitions possible
                if(stateKinds[state] == NUL) {
                    throw("ERROR in lexing after reading " + input.substr(0, i));
                }
                nextState = delta[ST_START][(unsigned char) input[i]];
                if ((stateKinds[state] == ZERO) && ((stateKinds[nextState] == NUM) || (stateKinds[nextState] == ID))) {
                   throw("ERROR in lexing after reading " + input.substr(0, i+1));                 
                }
                if(stateKinds[state] != WHITESPACE) {
                    Token token;
                    temp = input.substr(startIndex, i-startIndex);
                    if (temp == "wain") token.kind = WAIN;
                    else if (temp == "int") token.kind = INT;
                    else if (temp == "if") token.kind = IF;
                    else if (temp == "else") token.kind = ELSE;
                    else if (temp == "while") token.kind = WHILE;
                    else if (temp == "println") token.kind = PRINTLN;
                    else if (temp == "return") token.kind = RETURN;
                    else if (temp == "NULL") token.kind = NUL;
                    else if (temp == "new") token.kind = NEW;
                    else if (temp == "delete") token.kind = DELETE;
                    else token.kind = stateKinds[state];
                      token.lexeme = input.substr(startIndex, i-startIndex);
                      ret.push_back(token);
                }
                startIndex = i;
                state = ST_START;
                if(i >= input.length()) break;
            } else {
                state = nextState;
                i++;
            }
        }
    }

    return ret;
}

int Token::toInt() {
    if(kind == NUM) {
        long long l;
        sscanf( lexeme.c_str(), "%lld", &l );
	       if (lexeme.substr(0,1) == "-") {
            if(l < -2147483648LL)
                throw("ERROR: constant out of range: "+lexeme);
	       } else {
	       unsigned long long ul = l;
        if(ul > 4294967295LL)
          throw("ERROR: constant out of range: "+lexeme);
	       }
        return l;
    }
    throw("ERROR: attempt to convert non-integer token "+lexeme+" to Int");
}

// kindString maps each kind to a string for use in error messages.

string kS[] = {
    "ID",
    "NUM",
    "NUM",                  
    "LPAREN",             
    "RPAREN",
    "LBRACE",
    "RBRACE",
    "RETURN",
    "IF",
    "ELSE",
    "WHILE",
    "PRINTLN",
    "WAIN",
    "BECOMES",
    "INT",
    "EQ",
    "NE",
    "LT",
    "GT",
    "LE",
    "GE",
    "PLUS",
    "MINUS",
    "STAR",
    "SLASH",
    "PCT",
    "COMMA",
    "SEMI",
    "NEW",
    "DELETE",
    "LBRACK",
    "RBRACK",
    "AMP",
    "NULL",
    "WHITESPACE"
};

string kindString( Kind k ){
    if ( k < ID || k > WHITESPACE ) return "INVALID";
    return kS[k];
}

//======================================================================
//======= A sample program demonstrating the use of the scanner. =======
//======================================================================

int main() {

    try {
        vector<string> srcLines;

        // Read the entire input file, storing each line as a
        // single string in the array srcLines.
        while(true) {
            string line;
            getline(cin, line);
            if(cin.fail()) break;
            srcLines.push_back(line);
        }

        // Tokenize each line, storing the results in tokLines.
        vector<vector<Token> > tokLines;

        for(int line = 0; line < srcLines.size(); line++) {
            tokLines.push_back(scan(srcLines[line]));
        }

        // Now we process the tokens.
        // Sample usage: print the tokens of each line.
        for(int line=0; line < tokLines.size(); line++ ) {
            for(int j=0; j < tokLines[line].size(); j++ ) {
                Token token = tokLines[line][j];
                cout << kindString(token.kind) << " " << token.lexeme << endl;
            }
        }
    } catch(string msg) {
        cerr << msg << endl;
    }

    return 0;
}

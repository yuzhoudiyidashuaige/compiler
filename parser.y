%{
    #include "Ast.h"
    #include <cstdio>
    #include <cstdlib>
    #define YYERROR_VERBOSE 1
	Block *programBlock; 
	extern int yylex();
    extern int yylineno;
	void yyerror(const char *s) { std::printf("Error: %s", s); }
    void printError(int lineno, string s) { std::printf(" at: line: %d\n", lineno); }
%}

%error-verbose

%output "parsing.cpp"

%union {
	Ast *node;
	Block *block;
	Exp *exp;
	Stmt *stmt;
	Identifier *identifier;
	VariableDeclaration *variableDeclaration;
	std::vector<VariableDeclaration*> *varibleDeclarationList;
	std::vector<Exp*> *ExpList;
	std::string *string;
	int token;
}

%token <string> IDENTIFIER CONSTANT_INT CONSTANT_FLOAT CONSTANT_CHAR CONSTANT_STRING
%token <token> PLUS MINUS MUL DIV AND OR NOT GETAD EQ NEQ LT GT LEQ GEQ RETURN IF ELSE WHILE BREAK SEMI LBRACE RBRACE COMMA COLON ASSIGN LPAREN RPAREN LBRACKET RBRACKET DOT BNOT MOD

/* according to c language accociativity and priorioty*/
%left OR
%left AND
%left EQ NEQ LT GT LEQ GEQ
%left PLUS MINUS
%left MUL DIV
%right NOT
%left LPAREN RPAREN LBRACKET RBRACKET

%type <identifier> identifier
%type <exp> const_value Exp
%type <stmt> Stmt var_decl func_decl
%type <varibleDeclarationList> func_decl_args
%type <ExpList> call_args
%type <block> program Stmts block
/* %type <token> comparison */

%start program

%%

program: 
    Stmts { 
        programBlock = $1; 
    };

Stmts: 
    Stmt {
        $$ = new Block(yylineno); 
        $$->StmtList.push_back($<stmt>1);
    }
    | Stmts Stmt {
        $1->StmtList.push_back($<stmt>2);
    };

Stmt: 
    var_decl SEMI
    | func_decl
    | Exp SEMI {
        $$ = new ExpStmt(*$1, yylineno);
    }
    | RETURN SEMI {
        $$ = new ReturnVoid(yylineno);
    }
    | RETURN Exp SEMI {
        $$ = new ReturnStmt(*$2, yylineno);
    }
    | BREAK SEMI {
        $$ = new BreakStmt(yylineno);
    }
    | IF LPAREN Exp RPAREN block {
        $$ = new IfStmt(*$3, *$5, yylineno);
    }
    | IF LPAREN Exp RPAREN block ELSE block {
        $$ = new IfElseStmt(*$3, *$5, *$7, yylineno);
    }
    | IF error ELSE block {
        printError(yylineno, "");
    }
    | WHILE LPAREN Exp RPAREN block {
        $$ = new WhileStmt(*$3, *$5, yylineno);
    }
    | WHILE error RPAREN {
        printError(yylineno, "");
    }
    | WHILE error RBRACKET {
        printError(yylineno, "");
    }
    | error SEMI {
        printError(yylineno, "");
    }
    ;

block: 
    LBRACE Stmts RBRACE {
        $$ = $2;
    }
    | LBRACE RBRACE {
        $$ = new Block(yylineno);
    };

var_decl:
    identifier identifier {
        $$ = new VariableDeclaration(*$1, *$2, yylineno);
    }
    | identifier identifier ASSIGN Exp {
        $$ = new VariableDeclaration(*$1, *$2, $4, yylineno);
    }
    | identifier identifier LBRACKET CONSTANT_INT RBRACKET { // array
        $$ = new VariableDeclaration(*$1, *$2, atoi($4->c_str()), yylineno);
    }
    | identifier identifier LBRACKET error RBRACKET {
        printError(yylineno, "");
    }
    | identifier error {
        printError(yylineno, "");
    }
    | error identifier {
        printError(yylineno, "");
    }
    ;

func_decl: 
    identifier identifier LPAREN func_decl_args RPAREN block {
        $$ = new FunctionDeclaration(*$1, *$2, *$4, *$6, yylineno); // TODO: delete $4 ?
    }
    | identifier identifier error RPAREN {
        printError(yylineno, "");
    }
    ;
    
func_decl_args:
    {
        $$ = new std::vector<VariableDeclaration*>();
    }
    | var_decl {
        $$ = new std::vector<VariableDeclaration*>();
        $$->push_back($<variableDeclaration>1);
    }
    | func_decl_args COMMA var_decl {
        $1->push_back($<variableDeclaration>3);
    };

identifier:
    IDENTIFIER { 
        $$ = new Identifier(*$1, yylineno);
    };

const_value: 
    CONSTANT_INT {
        $$ = new Int(atoi($1->c_str()), yylineno);
    }
    | CONSTANT_FLOAT {
        $$ = new Float(atof($1->c_str()), yylineno);
    }
    | CONSTANT_CHAR {
        $$ = new Char(*$1, yylineno);
    }
    | CONSTANT_STRING {
        $$ = new String(*$1, yylineno);
    };

call_args:
    {
        $$ = new std::vector<Exp*>();
    }
    | Exp {
        $$ = new std::vector<Exp*>();
        $$->push_back($1);
    }
    | call_args COMMA Exp {
        $1->push_back($3);
    };

Exp:
    identifier ASSIGN Exp {
        $$ = new Assignment(*$<identifier>1, *$3, yylineno);
    }
    | identifier LPAREN call_args RPAREN {
        $$ = new FunctionCall(*$1, *$3, yylineno);
    }
    | identifier {
        $<identifier>$ = $1;
    }
    | GETAD identifier {
        $$ = new getAddr(*$2, yylineno);
    }
    | GETAD identifier LBRACKET Exp RBRACKET {
        $$ = new getArrayAddr(*$2, *$4, yylineno);
    }
    | Exp MUL Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp DIV Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp PLUS Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp MINUS Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp AND Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp OR Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp LT Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp GT Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp EQ Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp NEQ Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp LEQ Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | Exp GEQ Exp {
        $$ = new BinaryOp($2, *$1, *$3, yylineno);
    }
    | LPAREN Exp RPAREN {
        $$ = $2;
    }
    | identifier LBRACKET Exp RBRACKET { // array element access
        $$ = new getArrayElement(*$1, *$3, yylineno);
    }
    | identifier LBRACKET Exp RBRACKET ASSIGN Exp { // array element access
        $$ = new ArrayElementAssign(*$1, *$3, *$6, yylineno);
    }
    | const_value
    | Exp ASSIGN error {
        printError(yylineno, "wrong assign Exp");
    }
    | Exp AND error {
        printError(yylineno, "");
    }
    | Exp OR error {
        printError(yylineno, "");
    }
    | Exp PLUS error {
        printError(yylineno, "");
    }
    | Exp MINUS error {
        printError(yylineno, "");
    }
    | Exp MUL error {
        printError(yylineno, "");
    }
    | Exp DIV error {
        printError(yylineno, "");
    }
    | Exp LT error {
        printError(yylineno, "");
    }
    | Exp GT error {
        printError(yylineno, "");
    }
    | Exp EQ error {
        printError(yylineno, "");
    }
    | Exp NEQ error {
        printError(yylineno, "");
    }
    | Exp LEQ error {
        printError(yylineno, "");
    }
    | Exp GEQ error {
        printError(yylineno, "");
    }
    Exp LBRACKET error RBRACKET {
        printError(yylineno, "");
    }
    ;
%%
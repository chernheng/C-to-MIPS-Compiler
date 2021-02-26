%code requires{
  #include "include/ast.hpp"
  #include <cassert>

  extern const Expression *g_root; // A way of getting the AST out

  //! This is to fix problems when generating C++
  // We are declaring the functions provided by Flex, so
  // that Bison generated code can call them.
  int yylex(void);
  void yyerror(const char *);
}

// Represents the value associated with any kind of
// AST node.
%union{
  const Program *programPtr;
  double number;
  std::string *string;
}

%token KW_UNSIGNED KW_WHILE KW_FOR KW_IF KW_ELSE
%token B_LCURLY B_RCURLY B_LSQUARE B_RSQUARE B_LBRACKET B_RBRACKET
%token COND_LTEQ COND_GREQ COND_EQ COND_NEQ COND_LT COND_GR
%token OP_EQUAL OP_TIMES OP_PLUS OP_XOR OP_MINUS OP_DIVIDE OP_MODULO OP_REF OP_OR
%token SEMI_COLON

%type <number> NUMBER
%type <string> NAME VAR_TYPE

%type <programPtr> MAIN_SEQ COMMAND_SEQ COMMAND
%type <programPtr> FUNCTION LOOP BRANCH STATEMENT SCOPE ASSIGNMENT
%type <programPtr> DECLARAION VAR_DECLARATION FUNC_DECLARATION FUNCTION_DEF

//================================================================
%token T_TIMES T_DIVIDE T_PLUS T_MINUS T_EXPONENT
%token T_LOG T_EXP T_SQRT
%token T_NUMBER T_VARIABLE

%type <expr> EXPR TERM UNARY FACTOR
%type <number> T_NUMBER
%type <string> T_VARIABLE T_LOG T_EXP T_SQRT FUNCTION_NAME

%start ROOT

%%

/* "Command" is a linked list element that holds a pointer to the current command/operation and a pointer to the next 
    "Command" element. Command elements are linked together to form a linked-list to handle a program with multiple 
    lines of code. "Command" is defined in ast_program.hpp (under includes/ast)
*/

ROOT : MAIN_SEQ { g_root = $1; }

MAIN_SEQ : DECLARATION              { $$ = new Command($1,nullptr); }    //int x; int f();
         | FUNCTION_DEF             { $$ = new Command($1,nullptr); }    //int f() { stmt }
         | DECLARATION MAIN_SEQ     { $$ = new Command($1,$2); }         //multiple lines
         | FUNCTION_DEF MAIN_SEQ    { $$ = new Command($1,$2); }

FUNCTION_DEF : VAR_TYPE NAME B_LBRACKET B_RBRACKET SCOPE          {}   // definition  (no arguments)

COMMAND_SEQ : COMMAND               { $$ = new Command($1,nullptr); }
            | COMMAND COMMAND_SEQ   { $$ = new Command($1,$2); }

COMMAND : VAR_DECLARATION           { $$ = $1; }
        | LOOP                      { $$ = $1; }
        | BRANCH                    { $$ = $1; }
        | STATEMENT                 { $$ = $1; }
        | FUNCTION                  { $$ = $1; }
        | SCOPE                     { $$ = $1; }

SCOPE : B_LCURLY B_RCURLY               { $$ = new Scope(nullptr); }    // empty scope (Scope is defined in ast_program.hpp)
      | B_LCURLY COMMAND_SEQ B_RCURLY   { $$ = new Scope($2); }

FUNCTION : NAME B_LBRACKET B_RBRACKET SEMI_COLON              {}   //call function (without storing return result) (no arguments)
         | VARIABLE NAME B_LBRACKET B_RBRACKET SEMI_COLON     {}   //call function (no arguments)

DECLARAION : VAR_DECLARATION        {}    // variable declaration
           | FUNC_DECLARATION       {}   // function declaration

VAR_DECLARATION : VAR_TYPE NAME SEMI_COLON                    {}     // int x
                | VAR_TYPE NAME OP_EQUAL NUMBER SEMI_COLON    {}     //int x=10;

STATEMENT : ASSIGNMENT SEMI_COLON   { $$ = $1; }

ASSIGNMENT : VARIABLE OP_EQUAL FUNCTION     { $$ = new AssignmentOperator($1,$3); }
           | VARIABLE OP_EQUAL MATH         { $$ = new AssignmentOperator($1,$3); }     // need to add parser support for math 

//======================================================================================================================         

/* TODO-3 : Add support for (x + 6) and (10 - y). You'll need to add production rules, and create an AddOperator or
            SubOperator. */
EXPR : TERM  {$$ = $1;}
     | EXPR T_PLUS TERM      { $$ = new AddOperator($1, $3); }
     | EXPR T_MINUS TERM  {$$ = new SubOperator($1, $3); }

/* TODO-4 : Add support (x * 6) and (z / 11). */
TERM : UNARY      { $$ = $1; }
     | TERM T_TIMES UNARY          { $$ = new MulOperator($1, $3); }
     | TERM T_DIVIDE UNARY          { $$ = new DivOperator($1, $3); }

/*  TODO-5 : Add support for (- 5) and (- x). You'll need to add production rules for the unary minus operator and create a NegOperator. */
UNARY : FACTOR        { $$ = $1; }
      | T_MINUS FACTOR {$$ = new NegOperator($2);}

/* TODO-2 : Add a rule for variable, base on the pattern of number. */
FACTOR : T_NUMBER     { $$ = new Number( $1 ); }
       | B_LBRACKET EXPR B_RBRACKET { $$ = $2; }
       | T_VARIABLE   {$$ = new Variable(*$1);}
       | T_LOG B_LBRACKET EXPR B_RBRACKET {$$ = new LogFunction($3);}
       | FACTOR T_EXPONENT UNARY {$$ = new ExpOperator($1,$3);}
       | T_EXP B_LBRACKET EXPR B_RBRACKET {$$ = new ExpFunction($3);}
       | T_SQRT B_LBRACKET EXPR B_RBRACKET {$$ = new SqrtFunction($3);}

/* TODO-6 : Add support log(x), by modifying the rule for FACTOR. */

/* TODO-7 : Extend support to other functions. Requires modifications here, and to FACTOR. */
FUNCTION_NAME : T_LOG { $$ = new std::string("log"); }
              | T_EXP { $$ = new std::string("exp");}
              | T_SQRT { $$ = new std::string("sqrt");}

%%

const Program *g_root; // Definition of variable (to match declaration earlier)

const Program *parseAST()
{
  g_root=0;
  yyparse();
  return g_root;
}

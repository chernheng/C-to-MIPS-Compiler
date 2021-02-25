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
  const Expression *expr;
  double number;
  std::string *string;
}

%token T_TIMES T_DIVIDE T_PLUS T_MINUS T_EXPONENT
%token B_LBRACKET B_RBRACKET
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

ROOT : PROGRAM { g_root = $1; }

PROGRAM : MAIN_SEQ   { $$ = $1; }

MAIN_SEQ : DECLARATION              { $$ = new Command($1,nullptr); }
         | FUNCTION                 { $$ = new Command($1,nullptr); }
         | DECLARATION MAIN_SEQ     { $$ = new Command($1,$2); }
         | FUNCTION MAIN_SEQ        { $$ = new Command($1,$2); }

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

FUNCTION : VAR_INT VARIABLE B_LBRACKET B_RBRACKET SCOPE     // need to do handle for functions 
         | VARIABLE B_LBRACKET B_RBRACKET   // need to handle arguments

DECLARAION : VAR_DECLARATION    // variable declaration
           | FUNC_DECLARATION   // function declaration

VAR_DECLARATION : VAR_TYPE NAME SEMI_COLON   // int x
                | VAR_TYPE NAME OP_EQUAL NUMBER     //int x=10;

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

const Expression *g_root; // Definition of variable (to match declaration earlier)

const Expression *parseAST()
{
  g_root=0;
  yyparse();
  return g_root;
}

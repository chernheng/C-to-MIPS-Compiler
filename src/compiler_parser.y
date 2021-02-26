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
%token OP_EQUAL OP_TIMES OP_PLUS OP_XOR OP_MINUS OP_DIVIDE OP_MODULO OP_REF OP_OR OP_NOT OP_LSHIFT OP_RSHIFT
%token SEMI_COLON

%type <string> NAME VAR_TYPE NUMBER

%type <programPtr> MAIN_SEQ COMMAND_SEQ COMMAND
%type <programPtr> FUNCTION LOOP BRANCH STATEMENT SCOPE ASSIGNMENT
%type <programPtr> DECLARAION VAR_DECLARATION FUNC_DECLARATION FUNCTION_DEF
%type <programPtr> MATH WHILE_LOOP FOR_LOOP CONDITION FACTOR VARIABLE ELSE_BLOCK TERM NEG BITWISE POINTER


%start ROOT

%left OP_OR
%left OP_XOR
%left OP_REF
%left OP_LSHIFT OP_RSHIFT
%left OP_PLUS OP_MINUS
%right OP_NOT MINUS REF DEREF

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

DECLARAION : VAR_DECLARATION        {}    // variable declaration
           | FUNC_DECLARATION       {}   // function declaration

VAR_DECLARATION : VAR_TYPE NAME SEMI_COLON                    {}     // int x
                | VAR_TYPE NAME OP_EQUAL NUMBER SEMI_COLON    {}     //int x=10;

LOOP : WHILE_LOOP STATEMENT     { $$ = new WhileLoop($1,$2); }
     | WHILE_LOOP SCOPE         { $$ = new WhileLoop($1,$2); }
    //  | FOR_LOOP STATEMENT       {}
    //  | FOR_LOOP SCOPE           {}

WHILE_LOOP : KW_WHILE B_LBRACKET CONDITION B_RBRACKET   { $$ = $3; }

BRANCH : KW_IF B_LBRACKET CONDITION B_RBRACKET STATEMENT                { $$ = new IfBlock($3,$5,nullptr); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET SCOPE                    { $$ = new IfBlock($3,$5,nullptr); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET STATEMENT ELSE_BLOCK     { $$ = new IfBlock($3,$5,$6); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET SCOPE ELSE_BLOCK         { $$ = new IfBlock($3,$5,$6); }

ELSE_BLOCK : KW_ELSE STATEMENT    { $$ = $2; }
           | KW_ELSE SCOPE        { $$ = $2; }

STATEMENT : ASSIGNMENT SEMI_COLON   { $$ = $1; }

ASSIGNMENT : VARIABLE OP_EQUAL FUNCTION     { $$ = new AssignmentOperator($1,$3); }
           | VARIABLE OP_EQUAL MATH         { $$ = new AssignmentOperator($1,$3); }     // need to add parser support for math 

CONDITION : FACTOR                          {}
          | FACTOR COND_EQ FACTOR           {}
          | FACTOR COND_NEQ FACTOR          {}
          | FACTOR COND_GREQ FACTOR         {}
          | FACTOR COND_LTEQ FACTOR         {}
          | FACTOR COND_GR FACTOR           {}
          | FACTOR COND_LT FACTOR           {}

MATH : TERM  {$$ = $1;}
     | MATH OP_PLUS TERM      { $$ = new AddOperator($1, $3); }
     | MATH OP_MINUS TERM  {$$ = new SubOperator($1, $3); }
     | MATH OP_XOR TERM  {$$ = new BitXOROperator($1, $3); } // ^
     | MATH OP_OR TERM  {$$ = new BitOROperator($1, $3); } // |
     | MATH OP_REF TERM  {$$ = new BitANDOperator($1, $3); } // &
     | MATH OP_LSHIFT TERM  {$$ = new LeftShiftOperator($1, $3); } 
     | MATH OP_RSHIFT TERM  {$$ = new RightShiftOperator($1, $3); }

TERM : NEG      { $$ = $1; }
     | TERM OP_TIMES NEG          { $$ = new MulOperator($1, $3); }
     | TERM OP_DIVIDE NEG          { $$ = new DivOperator($1, $3); }

NEG : FACTOR        { $$ = $1; }
    | OP_NOT FACTOR {$$ = new BitNOTOperator($2);}
    | OP_MINUS FACTOR %prec MINUS {$$ = new NegOperator($2);}
    | OP_REF FACTOR %prec REF {$$ = new RefOperator($2);}
    | OP_TIMES FACTOR %prec DEREF {$$ = new DerefOperator($2);}
    | FACTOR OP_PLUS OP_PLUS {$$ = new IncOperator($1);}
    | FACTOR OP_MINUS OP_MINUS {$$ = new DecOperator($1);}

FACTOR : VARIABLE     { $$ = $1; }    // variable
       | NUMBER   { $$ = new Number($1); }      // number
       | B_LBRACKET MATH B_RBRACKET { $$ = $2; }


VARIABLE : NAME   { $$ = new Variable($1); }    // variable


%%

const Program *g_root; // Definition of variable (to match declaration earlier)

const Program *parseAST()
{
  g_root=0;
  yyparse();
  return g_root;
}

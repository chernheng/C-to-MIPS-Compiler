%code requires{
  #include "include/ast.hpp"
  #include <cassert>

  extern const Program *g_root; // A way of getting the AST out

  //! This is to fix problems when generating C++
  // We are declaring the functions provided by Flex, so
  // that Bison generated code can call them.
  int yylex(void);
  extern FILE *yyin;
  void yyerror(const char *);
}

// Represents the value associated with any kind of
// AST node.
%union{
  const Program *programPtr;
  FunctionDefArgs *fnDefArgs;
  FunctionArgs *fnCallArgs;
  double number;
  std::string *string;
}

%token KW_UNSIGNED KW_WHILE KW_FOR KW_IF KW_ELSE KW_RETURN KW_BREAK KW_CONTINUE KW_ELIF
%token B_LCURLY B_RCURLY B_LSQUARE B_RSQUARE B_LBRACKET B_RBRACKET
%token COND_LTEQ COND_GREQ COND_EQ COND_NEQ COND_LT COND_GR COND_AND COND_OR
%token OP_EQUAL OP_TIMES OP_PLUS OP_XOR OP_MINUS OP_DIVIDE OP_MODULO OP_REF OP_OR OP_NOT OP_LSHIFT OP_RSHIFT OP_INC OP_DEC
%token SEMI_COLON NAME NUMBER VAR_TYPE COMMA

%type <string> NAME VAR_TYPE NUMBER

%type <programPtr> MAIN_SEQ COMMAND_SEQ COMMAND
%type <programPtr> FUNCTION LOOP BRANCH STATEMENT SCOPE ASSIGNMENT FLOW RETN STATE
%type <programPtr> DECLARATION VAR_DECLARATION FUNCTION_DEF FUNC_DECLARATION 
%type <programPtr> MATH WHILE_LOOP FOR_LOOP CONDITION FACTOR VARIABLE ELSE_BLOCK TERM NEG ADDSHIFT ELIF_BLOCK INCREMENT 
%type <fnDefArgs> DEF_ARGS
%type <fnCallArgs> CALL_ARGS


%start ROOT

%left COND_OR
%left COND_AND
%left OP_OR
%left OP_XOR
%left OP_REF
%left COND_EQ COND_NEQ
%left COND_GR COND_GREQ COND_LT COND_LTEQ 
%left OP_LSHIFT OP_RSHIFT
%left OP_PLUS OP_MINUS
%right OP_NOT COND_NOT MINUS REF DEREF OP_INC OP_DEC

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

FUNCTION_DEF : VAR_TYPE NAME B_LBRACKET B_RBRACKET SCOPE               { $$ = new FunctionDef($1,$2,nullptr,$5); }
             | VAR_TYPE NAME B_LBRACKET DEF_ARGS B_RBRACKET SCOPE      { $$ = new FunctionDef($1,$2,$4,$6); }   // definition  (no arguments)

DEF_ARGS : VAR_TYPE NAME                     { $$ = new FunctionDefArgs($1,$2,nullptr); }
         | VAR_TYPE NAME COMMA DEF_ARGS      { $$ = new FunctionDefArgs($1,$2,$4); }

COMMAND_SEQ : COMMAND               { $$ = new Command($1,nullptr); }
            | COMMAND COMMAND_SEQ   { $$ = new Command($1,$2); }

COMMAND : VAR_DECLARATION           { $$ = $1; }
        | LOOP                      { $$ = $1; }
        | BRANCH                    { $$ = $1; }
        | STATEMENT                 { $$ = $1; }
        | FLOW                      { $$ = $1; }
     //    | FUNCTION                  { $$ = $1; }
        | SCOPE                     { $$ = $1; }

SCOPE : B_LCURLY B_RCURLY               { $$ = new Scope(nullptr); }    // empty scope (Scope is defined in ast_program.hpp)
      | B_LCURLY COMMAND_SEQ B_RCURLY   { $$ = new Scope($2); }

FUNCTION : NAME B_LBRACKET B_RBRACKET             { $$ = new FunctionCall($1,nullptr); }   //call function (without storing return result) (no arguments)
         | NAME B_LBRACKET CALL_ARGS B_RBRACKET   { $$ = new FunctionCall($1,$3); }

CALL_ARGS : MATH                                  { $$ = new FunctionCallArgs($1,nullptr); }
          | FUNCTION                              { $$ = new FunctionCallArgs($1,nullptr); }
          | MATH COMMA CALL_ARGS                  { $$ = new FunctionCallArgs($1,$3); }
          | FUNCTION COMMA CALL_ARGS              { $$ = new FunctionCallArgs($1,$3); }

DECLARATION : VAR_DECLARATION        { $$ = $1; }    // variable declaration
           | FUNC_DECLARATION        { $$ = $1; }   // function declaration

VAR_DECLARATION : VAR_TYPE NAME SEMI_COLON                    { $$ = new DeclareVariable($1,$2,0); }     // int x
                | VAR_TYPE NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($1,$2,$4,0); }     //int x=10;
                | VAR_TYPE OP_TIMES NAME SEMI_COLON           { $$ = new DeclareVariable($1,$3,1); } //int *x;
                | VAR_TYPE OP_TIMES NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($1,$3,$5,1); } //int *x = &f;

FUNC_DECLARATION : VAR_TYPE NAME B_LBRACKET B_RBRACKET SEMI_COLON   { $$ = new DeclareFunction($1,$2); }

LOOP : WHILE_LOOP STATEMENT     { $$ = new WhileLoop($1,$2); }
     | WHILE_LOOP SCOPE         { $$ = new WhileLoop($1,$2); }
     | FOR_LOOP                 { $$ = $1; }       // ForLoop( initialization, condition, updateExpression, body);

FOR_LOOP : KW_FOR B_LBRACKET VAR_DECLARATION CONDITION SEMI_COLON STATE B_RBRACKET STATEMENT   { $$ = new ForLoop($3,$4,$6,$8); } //for(int i=0;i<0;i++)
         | KW_FOR B_LBRACKET VAR_DECLARATION CONDITION SEMI_COLON STATE B_RBRACKET SCOPE   { $$ = new ForLoop($3,$4,$6,$8);}
         | KW_FOR B_LBRACKET ASSIGNMENT SEMI_COLON CONDITION SEMI_COLON STATE B_RBRACKET STATEMENT   { $$ = new ForLoop($3,$5,$7,$9); } //for(int i=0;i<0;i++)
         | KW_FOR B_LBRACKET ASSIGNMENT SEMI_COLON CONDITION SEMI_COLON STATE B_RBRACKET SCOPE   { $$ = new ForLoop($3,$5,$7,$9);}

WHILE_LOOP : KW_WHILE B_LBRACKET CONDITION B_RBRACKET   { $$ = $3; }

BRANCH : KW_IF B_LBRACKET CONDITION B_RBRACKET STATEMENT                             { $$ = new IfBlock($3,$5,nullptr,nullptr); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET SCOPE                                 { $$ = new IfBlock($3,$5,nullptr,nullptr); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET STATEMENT ELSE_BLOCK                  { $$ = new IfBlock($3,$5,nullptr,$6); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET SCOPE ELSE_BLOCK                      { $$ = new IfBlock($3,$5,nullptr,$6); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET STATEMENT ELIF_BLOCK ELSE_BLOCK       { $$ = new IfBlock($3,$5,$6,$7); }
       | KW_IF B_LBRACKET CONDITION B_RBRACKET SCOPE ELIF_BLOCK ELSE_BLOCK           { $$ = new IfBlock($3,$5,$6,$7); }

ELIF_BLOCK : KW_ELIF B_LBRACKET CONDITION B_RBRACKET STATEMENT                       { $$ = new ElseIfBlock($3,$5,nullptr); }
           | KW_ELIF B_LBRACKET CONDITION B_RBRACKET SCOPE                           { $$ = new ElseIfBlock($3,$5,nullptr); }
           | KW_ELIF B_LBRACKET CONDITION B_RBRACKET STATEMENT ELIF_BLOCK            { $$ = new ElseIfBlock($3,$5,$6); }
           | KW_ELIF B_LBRACKET CONDITION B_RBRACKET SCOPE ELIF_BLOCK                { $$ = new ElseIfBlock($3,$5,$6); }

ELSE_BLOCK : KW_ELSE STATEMENT    { $$ = new ElseBlock($2); }
           | KW_ELSE SCOPE        { $$ = new ElseBlock($2); }

FLOW : RETN SEMI_COLON                       { $$ = $1; }
     | KW_BREAK SEMI_COLON                   { $$ = new BreakStatement(); }
     | KW_CONTINUE SEMI_COLON                { $$ = new ContinueStatement(); }

RETN : KW_RETURN                  { $$ = new ReturnStatement(); }
     | KW_RETURN MATH             { $$ = new ReturnStatement($2); }

STATEMENT : FUNCTION SEMI_COLON     { $$ = $1; }
          | ASSIGNMENT SEMI_COLON   { $$ = $1; }
          | INCREMENT SEMI_COLON    { $$ = $1; }

STATE     : ASSIGNMENT   { $$ = $1; }
          | INCREMENT    { $$ = $1; }

ASSIGNMENT : VARIABLE OP_EQUAL FUNCTION     { $$ = new AssignmentOperator($1,$3); }
           | VARIABLE OP_EQUAL MATH         { $$ = new AssignmentOperator($1,$3); }     // need to add parser support for math 


MATH : CONDITION  {$$ = $1;}
     | MATH OP_XOR MATH  {$$ = new BitXOROperator($1, $3); } // ^ 
     | MATH OP_OR MATH  {$$ = new BitOROperator($1, $3); } // |
     | MATH OP_REF MATH  {$$ = new BitANDOperator($1, $3); } // &
     | MATH COND_OR MATH  {$$ = new LogicalOR($1, $3); } // ||
     | MATH COND_AND MATH  {$$ = new LogicalAND($1, $3); } // &&

CONDITION : ADDSHIFT                          { $$ = $1; }
          | CONDITION COND_EQ CONDITION           { $$ = new EqualTo($1,$3); }
          | CONDITION COND_NEQ CONDITION          { $$ = new NotEqual($1,$3); }
          | CONDITION COND_GREQ CONDITION         { $$ = new GreaterEqual($1,$3); }
          | CONDITION COND_LTEQ CONDITION         { $$ = new LessEqual($1,$3); }
          | CONDITION COND_GR CONDITION           { $$ = new GreaterThan($1,$3); }
          | CONDITION COND_LT CONDITION           { $$ = new LessThan($1,$3); }

ADDSHIFT : TERM  {$$ = $1;}
     | ADDSHIFT OP_PLUS ADDSHIFT      { $$ = new AddOperator($1, $3); }
     | ADDSHIFT OP_MINUS ADDSHIFT  {$$ = new SubOperator($1, $3); }
     | ADDSHIFT OP_LSHIFT ADDSHIFT  {$$ = new LeftShiftOperator($1, $3); } 
     | ADDSHIFT OP_RSHIFT ADDSHIFT  {$$ = new RightShiftOperator($1, $3); }

TERM : NEG      { $$ = $1; }
     | TERM OP_TIMES NEG          { $$ = new MulOperator($1, $3); }
     | TERM OP_DIVIDE NEG         { $$ = new DivOperator($1, $3); }
     | TERM OP_MODULO NEG         { $$ = new ModuloOperator($1, $3);}

NEG : INCREMENT        { $$ = $1; }
    | OP_NOT NEG {$$ = new BitNOTOperator($2);}
    | COND_NOT NEG {$$ = new LogicalNOT($2);}
    | OP_MINUS NEG %prec MINUS {$$ = new NegOperator($2);}
    | OP_REF NEG %prec REF {$$ = new RefOperator($2);}
    | OP_TIMES NEG %prec DEREF {$$ = new DerefOperator($2);}

INCREMENT : FACTOR   { $$ = $1; }
          | INCREMENT OP_INC {$$ = new IncOperator($1);}
          | INCREMENT OP_DEC {$$ = new DecOperator($1);}
          | OP_INC INCREMENT {$$ = new IncAfterOperator($2);}
          | OP_DEC INCREMENT {$$ = new DecAfterOperator($2);}

FACTOR : VARIABLE     { $$ = $1; }    // variable
       | NUMBER   { $$ = new Number($1); }      // number
       | B_LBRACKET MATH B_RBRACKET { $$ = $2; }


VARIABLE : NAME   { $$ = new Variable($1); }    // variable



%%

const Program *g_root; // Definition of variable (to match declaration earlier)

const Program *parseAST(char* file)
{
  g_root=0;
  yyin = fopen(file,"r");
  yyparse();
  fclose(yyin);
  return g_root;
}

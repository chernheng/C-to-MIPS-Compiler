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
  DeclareArrayElement *arrDecElements;
  ArrayIndex *arrIndex;
  CaseBlock *caseptr;
  AccessStructElement *accStructElement;
  double number;
  std::string *string;
}

%token KW_UNSIGNED KW_WHILE KW_FOR KW_IF KW_ELSE KW_RETURN KW_BREAK KW_CONTINUE KW_ELIF KW_SWITCH KW_CASE KW_DEFAULT KW_SIZEOF KW_TYPEDEF KW_STRUCT
%token B_LCURLY B_RCURLY B_LSQUARE B_RSQUARE B_LBRACKET B_RBRACKET
%token COND_LTEQ COND_GREQ COND_EQ COND_NEQ COND_LT COND_GR COND_AND COND_OR
%token OP_EQUAL OP_TIMES OP_PLUS OP_XOR OP_MINUS OP_DIVIDE OP_MODULO OP_REF OP_OR OP_NOT OP_LSHIFT OP_RSHIFT OP_INC OP_DEC OP_QUESTION OP_SUM_ASN OP_DIFF_ASN OP_SPOINT
%token SEMI_COLON NAME NUMBER VAR_TYPE COMMA COLON HEX OP_PRODUCT_ASN OP_DIVIDE_ASN OP_MOD_ASN DOT DOUBLE FLOAT

%type <string> NAME VAR_TYPE NUMBER HEX DOUBLE FLOAT

%type <programPtr> MAIN_SEQ COMMAND_SEQ COMMAND
%type <programPtr> FUNCTION LOOP BRANCH STATEMENT SCOPE ASSIGNMENT FLOW RETN STATE SWITCH TYPE_DECLARATION SIZEOF OP_ASSIGNMENT
%type <programPtr> DECLARATION VAR_DECLARATION FUNCTION_DEF FUNC_DECLARATION ARR_INIT_VAL 
%type <programPtr> MATH WHILE_LOOP FOR_LOOP CONDITION FACTOR VARIABLE ELSE_BLOCK TERM NEG ADDSHIFT ELIF_BLOCK TERNARY VARIABLE_STORE
%type <programPtr> STRUCT_DECLARATION STRUCT_DEC_ELEMENT STRUCT
%type <fnDefArgs> DEF_ARGS
%type <fnCallArgs> CALL_ARGS
%type <caseptr> CASE
%type <arrDecElements> ARR_DEC_INDEX
%type <arrIndex> ARRAY_INDEX
%type <accStructElement> STRUCT_ELEMENT


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

STRUCT_DECLARATION : KW_STRUCT NAME B_LCURLY STRUCT_DEC_ELEMENT B_RCURLY SEMI_COLON  { $$ = new DeclareStruct($2,$4); }
                   | KW_TYPEDEF KW_STRUCT B_LCURLY STRUCT_DEC_ELEMENT B_RCURLY NAME SEMI_COLON { $$ = new DeclareStruct($6,$4); }

STRUCT_DEC_ELEMENT : NAME NAME SEMI_COLON                                  { $$ = new DeclareStructElement($1,$2,0,0,nullptr); }
                   | NAME NAME SEMI_COLON STRUCT_DEC_ELEMENT               { $$ = new DeclareStructElement($1,$2,0,0,$4); }
                   | NAME NAME NAME SEMI_COLON                             { $$ = new DeclareStructElement($2,$3,0,1,nullptr); }
                   | NAME NAME NAME SEMI_COLON STRUCT_DEC_ELEMENT          { $$ = new DeclareStructElement($2,$3,0,1,$5); }
                   | NAME OP_TIMES NAME SEMI_COLON                         { $$ = new DeclareStructElement($1,$3,1,0,nullptr); }
                   | NAME OP_TIMES NAME SEMI_COLON STRUCT_DEC_ELEMENT      { $$ = new DeclareStructElement($1,$3,1,0,$5); }
                   | NAME NAME OP_TIMES NAME SEMI_COLON                    { $$ = new DeclareStructElement($2,$4,1,1,nullptr); }
                   | NAME NAME OP_TIMES NAME SEMI_COLON STRUCT_DEC_ELEMENT { $$ = new DeclareStructElement($2,$4,1,1,$6); }

DECLARATION : VAR_DECLARATION        { $$ = $1; }    // variable declaration
           | FUNC_DECLARATION        { $$ = $1; }   // function declaration
           | TYPE_DECLARATION        { $$ = $1; }   // typdef declaration
           | STRUCT_DECLARATION      { $$ = $1; }   // struct declaration

TYPE_DECLARATION : KW_TYPEDEF NAME NAME SEMI_COLON               { $$ = new DeclareTypeDef($2,$3,0,0); } // typedef int t1;
                 | KW_TYPEDEF NAME NAME NAME SEMI_COLON          { $$ = new DeclareTypeDef($3,$4,0,1); } // typedef unsigned char c1;
                 | KW_TYPEDEF NAME OP_TIMES NAME SEMI_COLON      { $$ = new DeclareTypeDef($2,$4,1,0); } // typedef int *t1;
                 | KW_TYPEDEF NAME NAME OP_TIMES NAME SEMI_COLON { $$ = new DeclareTypeDef($3,$5,1,1); } // typedef unsigned char *c1;

VAR_DECLARATION : NAME NAME SEMI_COLON                    { $$ = new DeclareVariable($1,$2,0,0); }     // int x
                | NAME NAME ARR_DEC_INDEX SEMI_COLON      { $$ = new DeclareArray($1,$2,$3,nullptr,0); }    // array
                | NAME NAME ARR_DEC_INDEX OP_EQUAL B_LCURLY ARR_INIT_VAL B_RCURLY SEMI_COLON {$$ = new DeclareArray($1,$2,$3,$6,0);}
                | NAME NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($1,$2,$4,0,0); }     //int x=10;
                | NAME NAME OP_EQUAL TERNARY SEMI_COLON      { $$ = new DeclareVariable($1,$2,$4,0,0); }    
                | NAME OP_TIMES NAME SEMI_COLON           { $$ = new DeclareVariable($1,$3,1,0); } //int *x;
                | NAME OP_TIMES NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($1,$3,$5,1,0); } //int *x = &f;
                | NAME NAME NAME SEMI_COLON                    { $$ = new DeclareVariable($2,$3,0,1); }     // int x (unsigned)
                | NAME NAME NAME ARR_DEC_INDEX SEMI_COLON      { $$ = new DeclareArray($2,$3,$4,nullptr,1); }    // array (unsigned)
                | NAME NAME NAME ARR_DEC_INDEX OP_EQUAL B_LCURLY ARR_INIT_VAL B_RCURLY SEMI_COLON {$$ = new DeclareArray($2,$3,$4,$7,1);} // unsigned
                | NAME NAME NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($2,$3,$5,0,1); }     //int x=10; (unsigned)
                | NAME NAME NAME OP_EQUAL TERNARY SEMI_COLON      { $$ = new DeclareVariable($2,$3,$5,0,1); }    // unsigned
                | NAME NAME OP_TIMES NAME SEMI_COLON           { $$ = new DeclareVariable($2,$4,1,1); } //int *x; (unsigned)
                | NAME NAME OP_TIMES NAME OP_EQUAL MATH SEMI_COLON      { $$ = new DeclareVariable($2,$4,$6,1,1); } //int *x = &f; (unsigned)
                | KW_STRUCT NAME NAME SEMI_COLON       { $$ = new DeclareVariable($2,$3,nullptr,0,0); }

ARR_DEC_INDEX : B_LSQUARE NUMBER B_RSQUARE                       { $$ = new DeclareArrayElement($2,nullptr); }
              | B_LSQUARE NUMBER B_RSQUARE ARR_DEC_INDEX         { $$ = new DeclareArrayElement($2,$4); }

ARR_INIT_VAL : NUMBER                                   {$$ = new Array_Init($1,nullptr);}
             | NUMBER COMMA ARR_INIT_VAL                {$$ = new Array_Init($1,$3);}

FUNC_DECLARATION : NAME NAME B_LBRACKET B_RBRACKET SEMI_COLON   { $$ = new DeclareFunction($1,$2); }
                 | NAME OP_TIMES NAME B_LBRACKET B_RBRACKET SEMI_COLON     { $$ = new DeclareFunction($1,$3); }
                 | NAME NAME NAME B_LBRACKET B_RBRACKET SEMI_COLON    { $$ = new DeclareFunction($2,$3); }    // unsigned char f1();
                 | NAME NAME OP_TIMES NAME B_LBRACKET B_RBRACKET SEMI_COLON     { $$ = new DeclareFunction($2,$4); }
                 | NAME NAME B_LBRACKET DEF_ARGS B_RBRACKET SEMI_COLON      { $$ = new DeclareFunction($1,$2); }
                 | NAME OP_TIMES NAME B_LBRACKET DEF_ARGS B_RBRACKET SEMI_COLON { $$ = new DeclareFunction($1,$3); }
                 | NAME NAME NAME B_LBRACKET DEF_ARGS B_RBRACKET SEMI_COLON      { $$ = new DeclareFunction($2,$3); }   // unsigned char f1([args]);
                 | NAME NAME OP_TIMES NAME B_LBRACKET DEF_ARGS B_RBRACKET SEMI_COLON { $$ = new DeclareFunction($2,$4); }

FUNCTION_DEF : NAME NAME B_LBRACKET B_RBRACKET SCOPE                  { $$ = new FunctionDef($1,$2,nullptr,$5); }
             | NAME OP_TIMES NAME B_LBRACKET B_RBRACKET SCOPE         { $$ = new FunctionDef($1,$3,nullptr,$6); }     // pointer return type
             | NAME NAME NAME B_LBRACKET B_RBRACKET SCOPE             { $$ = new FunctionDef($2,$3,nullptr,$6); }     // unsigned return type
             | NAME NAME OP_TIMES NAME B_LBRACKET B_RBRACKET SCOPE    { $$ = new FunctionDef($2,$4,nullptr,$7); }  // unsigned pointer return type
             | NAME NAME B_LBRACKET DEF_ARGS B_RBRACKET SCOPE         { $$ = new FunctionDef($1,$2,$4,$6); }   // definition  (with arguments)
             | NAME OP_TIMES NAME B_LBRACKET DEF_ARGS B_RBRACKET SCOPE     { $$ = new FunctionDef($1,$3,$5,$7); }  // definition (wuth args) return pointer
             | NAME NAME NAME B_LBRACKET DEF_ARGS B_RBRACKET SCOPE      { $$ = new FunctionDef($2,$3,$5,$7); }     // unsigned return type
             | NAME NAME OP_TIMES NAME B_LBRACKET DEF_ARGS B_RBRACKET SCOPE     { $$ = new FunctionDef($2,$4,$6,$8); }  // unsigned pointer return type

FUNCTION : NAME B_LBRACKET B_RBRACKET             { $$ = new FunctionCall($1,nullptr); }   //call function (without storing return result) (no arguments)
         | NAME B_LBRACKET CALL_ARGS B_RBRACKET   { $$ = new FunctionCall($1,$3); }

CALL_ARGS : MATH                                  { $$ = new FunctionCallArgs($1,nullptr); }
          | MATH COMMA CALL_ARGS                  { $$ = new FunctionCallArgs($1,$3); }

DEF_ARGS : NAME NAME                     { $$ = new FunctionDefArgs($1,$2,nullptr,0); }
         | NAME OP_TIMES NAME            { $$ = new FunctionDefArgs($1,$3,nullptr,1); }
         | NAME OP_TIMES NAME COMMA DEF_ARGS       {$$ = new FunctionDefArgs($1,$3,$5,1);}
     //     | NAME NAME ARRAY_INDEX                    { $$ = new FunctionDefArgs($1,$2,nullptr,1); }
         | NAME NAME COMMA DEF_ARGS      { $$ = new FunctionDefArgs($1,$2,$4,0); }

COMMAND_SEQ : COMMAND               { $$ = new Command($1,nullptr); }
            | COMMAND COMMAND_SEQ   { $$ = new Command($1,$2); }

COMMAND : VAR_DECLARATION           { $$ = $1; }
        | LOOP                      { $$ = $1; }
        | BRANCH                    { $$ = $1; }
        | STATEMENT                 { $$ = $1; }
        | FLOW                      { $$ = $1; }
        | FUNCTION                  { $$ = $1; }
        | SCOPE                     { $$ = $1; }
        | SWITCH                    { $$ = $1;}

SCOPE : B_LCURLY B_RCURLY               { $$ = new Scope(nullptr); }    // empty scope (Scope is defined in ast_program.hpp)
      | B_LCURLY COMMAND_SEQ B_RCURLY   { $$ = new Scope($2); }


LOOP : WHILE_LOOP STATEMENT     { $$ = new WhileLoop($1,$2); }
     | WHILE_LOOP SCOPE         { $$ = new WhileLoop($1,$2); }
     | FOR_LOOP                 { $$ = $1; }       // ForLoop( initialization, condition, updateExpression, body);

FOR_LOOP : KW_FOR B_LBRACKET VAR_DECLARATION CONDITION SEMI_COLON STATE B_RBRACKET STATEMENT   { $$ = new ForLoop($3,$4,$6,$8); } //for(int i=0;i<0;i++)
         | KW_FOR B_LBRACKET VAR_DECLARATION CONDITION SEMI_COLON STATE B_RBRACKET SCOPE   { $$ = new ForLoop($3,$4,$6,$8);}
         | KW_FOR B_LBRACKET ASSIGNMENT SEMI_COLON CONDITION SEMI_COLON STATE B_RBRACKET STATEMENT   { $$ = new ForLoop($3,$5,$7,$9); } //for(int i=0;i<0;i++)
         | KW_FOR B_LBRACKET ASSIGNMENT SEMI_COLON CONDITION SEMI_COLON STATE B_RBRACKET SCOPE   { $$ = new ForLoop($3,$5,$7,$9);}

WHILE_LOOP : KW_WHILE B_LBRACKET CONDITION B_RBRACKET   { $$ = $3; }

SWITCH : KW_SWITCH B_LBRACKET MATH B_RBRACKET B_LCURLY CASE B_RCURLY       {$$ = new SwitchBlock($3,$6);}
       | KW_SWITCH B_LBRACKET MATH B_RBRACKET B_LCURLY B_RCURLY       {$$ = new SwitchBlock($3,nullptr);}

CASE : KW_CASE FACTOR COLON COMMAND_SEQ CASE          { $$ = new CaseBlock{$2,$4,$5,nullptr};}
     | KW_CASE FACTOR COLON COMMAND_SEQ KW_DEFAULT COLON COMMAND_SEQ   { $$ = new CaseBlock{$2,$4,nullptr,$7};} //end of the case
     | KW_CASE FACTOR COLON COMMAND_SEQ   { $$ = new CaseBlock{$2,$4,nullptr,nullptr};}

// ENUM : ENUM VARIABLE

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

STATEMENT : ASSIGNMENT SEMI_COLON   { $$ = $1; }
          | NEG SEMI_COLON    { $$ = $1; }
          | OP_ASSIGNMENT SEMI_COLON {$$ = $1; }

STATE     : ASSIGNMENT   { $$ = $1; }
          | NEG    { $$ = $1; }

ASSIGNMENT : VARIABLE_STORE OP_EQUAL MATH         { $$ = new AssignmentOperator($1,$3); }     // need to add parser support for math 
           | VARIABLE_STORE OP_EQUAL TERNARY         { $$ = new AssignmentOperator($1,$3); } 

OP_ASSIGNMENT : NEG OP_SUM_ASN MATH      { $$ = new AssignmentSumOperator($1,$3);}
              | NEG OP_DIFF_ASN MATH      { $$ = new AssignmentDiffOperator($1,$3);}
              | NEG OP_PRODUCT_ASN MATH      { $$ = new AssignmentProductOperator($1,$3);}
              | NEG OP_DIVIDE_ASN MATH      { $$ = new AssignmentDivideOperator($1,$3);} //not implemented
              | NEG OP_MOD_ASN MATH      { $$ = new AssignmentModOperator($1,$3);}//not implemented, cannot work with pointers

TERNARY : CONDITION OP_QUESTION MATH COLON MATH {$$ = new TernaryBlock($1,$3,$5);}
        | B_LBRACKET TERNARY B_RBRACKET {$$ = $2;}

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

NEG : FACTOR       { $$ = $1; }
    | OP_NOT NEG {$$ = new BitNOTOperator($2);}
    | COND_NOT NEG {$$ = new LogicalNOT($2);}
    | OP_MINUS NEG %prec MINUS {$$ = new NegOperator($2);}
    | OP_REF NEG %prec REF {$$ = new RefOperator($2);}
    | OP_TIMES NEG %prec DEREF {$$ = new DerefOperator($2);}
    | NEG OP_INC {$$ = new IncOperator($1);}
    | NEG OP_DEC {$$ = new DecOperator($1);}
    | OP_INC NEG {$$ = new IncAfterOperator($2);}
    | OP_DEC NEG {$$ = new DecAfterOperator($2);}

FACTOR : VARIABLE     { $$ = $1; }    // variable
       | NUMBER   { $$ = new Number($1); }      // number
       | HEX {$$ = new Number($1);}
       | DOUBLE {$$ = new Double($1);}
       | FLOAT {$$ = new Float($1);}
       | FUNCTION { $$ = $1;}
       | SIZEOF     { $$ = $1; }
       | B_LBRACKET MATH B_RBRACKET { $$ = $2; }

VARIABLE : NAME                    { $$ = new Variable($1); }    // variable
         | NAME ARRAY_INDEX        { $$ = new Array($1,$2); }
         | NAME DOT STRUCT_ELEMENT { $$ = new StructRead($1,$3); }

ARRAY_INDEX : B_LSQUARE MATH B_RSQUARE                 { $$ = new ArrayIndex($2,nullptr); }    // handle array index
            | B_LSQUARE MATH B_RSQUARE ARRAY_INDEX     { $$ = new ArrayIndex($2,$4); }

VARIABLE_STORE : NAME                        { $$ = new VariableStore($1,0); }    // store to variable or array
               | NAME ARRAY_INDEX            { $$ = new ArrayStore($1,$2); }
               | OP_TIMES NAME               { $$ = new VariableStore($2,1);}
               | NAME DOT STRUCT_ELEMENT     { $$ = new StructStore($1,$3); }

STRUCT_ELEMENT : NAME                        { $$ = new AccessStructElement($1,nullptr); }
               | NAME DOT STRUCT_ELEMENT     { $$ = new AccessStructElement($1,$3); }

SIZEOF : KW_SIZEOF B_LBRACKET NAME B_RBRACKET               { $$ = new FunctionSizeof($3,nullptr); }
       | KW_SIZEOF B_LBRACKET NAME ARR_DEC_INDEX B_RBRACKET { $$ = new FunctionSizeof($3,$4); }



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

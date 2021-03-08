%option noyywrap

%{
// Avoid error "error: `fileno' was not declared in this scope"
extern "C" int fileno(FILE *stream);

#include "compiler_parser.tab.hpp"
%}

Types (int)|(char)|(float)|(double)|(void)

%%
{Types}         { yylval.string= new std::string(yytext); return VAR_TYPE;}
unsigned        { return KW_UNSIGNED;}
"else if"       { return KW_ELIF;}
if              { return KW_IF;} 
else            { return KW_ELSE;}
while           { return KW_WHILE;}  
for             { return KW_FOR;} 
return          { return KW_RETURN;}
break           { return KW_BREAK;}
continue        { return KW_CONTINUE;}
switch          { return KW_SWITCH;}
case            { return KW_CASE;}
default         { return KW_DEFAULT;}

"{"             { return B_LCURLY;}
"}"             { return B_RCURLY;}
"["             { return B_LSQUARE;}
"]"             { return B_RSQUARE;}
[\(]             { return B_LBRACKET; }
[\)]             { return B_RBRACKET; }

"<="              { return COND_LTEQ;}
>=              { return COND_GREQ;}
==              { return COND_EQ;}
!=              { return COND_NEQ;}
[<]             { return COND_LT;}
[>]             { return COND_GR;}
"&&"            { return COND_AND;}
"||"            { return COND_OR;}
[!]             { return COND_NOT;}

[=]               { return OP_EQUAL;}
[*]             { return OP_TIMES; }
[+]             { return OP_PLUS; }
[\^]            { return OP_XOR; }
[-]             { return OP_MINUS; }
[\/]            { return OP_DIVIDE; }
[%]             { return OP_MODULO;}
[&]             { return OP_REF; }
[|]             { return OP_OR; }
[~]             { return OP_NOT; }
"<<"            { return OP_LSHIFT; }
">>"            { return OP_RSHIFT; }
"++"            { return OP_INC;}
"--"            { return OP_DEC;}

","             { return COMMA;}
"\:"             { return COLON;}
[;]             { return SEMI_COLON;}

[0-9]+([.][0-9]*)?      { yylval.string= new std::string(yytext); return NUMBER; }
[a-zA-Z_]+[a-zA-Z0-9_]* { yylval.string= new std::string(yytext); return NAME; } /*A variable name can only have letters (both uppercase and lowercase letters),
                                                                                         digits and underscore, and  first letter should be either a letter or an underscore */
 
[ \t\r\n]+		{;}

.               { fprintf(stderr, "Invalid token\n"); exit(1); }
%%

void yyerror (char const *s)
{
  fprintf (stderr, "Parse error : %s\n", s);
  exit(1);
}

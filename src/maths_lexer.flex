%option noyywrap

%{
// Avoid error "error: `fileno' was not declared in this scope"
extern "C" int fileno(FILE *stream);

#include "maths_parser.tab.hpp"
%}

Types [(int)(char)(float)(double)(void)]

%%
{Variable}      { yylval.string=new std::string(yytext); return VAR_TYPE;}
unsigned        {return KW_UNSIGNED;}
if              { return KW_IF;} 
else            { return KW_ELSE;}
while           { return KW_WHILE;}  
for             { return KW_FOR;} 

"{"             { return B_LCURLY;}
"}"             { return B_RCURLY;}
"["             { return B_LSQUARE;}
"]"             { return B_RSQUARE;}
[(]             { return B_LBRACKET; }
[)]             { return B_RBRACKET; }

<=              { return COND_LTEQ;}
>=              { return COND_GREQ;}
==              { return COND_EQ;}
!=              { return COND_NEQ;}
=               { return OP_EQUAL;}
[<]             { return COND_LT;}
[>]             { return COND_GR;}

[*]             { return OP_TIMES; }
[+]             { return OP_PLUS; }
[\^]            { return OP_EXPONENT; }
[-]             { return OP_MINUS; }
[\/]            { return OP_DIVIDE; }
[%]             { return OP_MODULO}

[;]             { return SEMI_COLON;}

[0-9]+([.][0-9]*)? { yylval.number=strtod(yytext, 0); return NUMBER; }
[a-zA-Z_]+[a-zA-Z0-9_]* { yylval.string=new std::string(yytext); return NAME; } /*A variable name can only have letters (both uppercase and lowercase letters),
                                                                                         digits and underscore, and  first letter should be either a letter or an underscore */
 
[ \t\r\n]+		{;}

.               { fprintf(stderr, "Invalid token\n"); exit(1); }
%%

void yyerror (char const *s)
{
  fprintf (stderr, "Parse error : %s\n", s);
  exit(1);
}

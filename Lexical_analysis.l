D            [0-9]
L            [a-zA-Z_]
H            [a-fA-F0-9]
E            ([Ee][+-]?{D}+)

%{
#include <stdio.h>
#include <iostream>
#include <string>
#include "Ast.h"
#include "parsing.hpp"

int column = 0;

void check_comment(void);
%}

/* automatically count line number */
%option yylineno 

%%
    /* eat comments */
"/*"            { check_comment();}
"//"[^\n]*      { } /* eat // */

    /* eat keywords */
"while"         {  return yylval.token = WHILE;    }
"break"         {  return yylval.token = BREAK;    }
"if"            {  return yylval.token = IF;       }
"else"          {  return yylval.token = ELSE;     }
"return"        {  return yylval.token = RETURN;   }


    /* eat identifiers */
{L}({L}|{D})*            {   yylval.string = new std::string(yytext, yyleng); return IDENTIFIER;}

    /* eat constants */
0[xX]{H}+                {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_INT; /* hexadecimal */ }
0[0-7]*                  {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_INT; /* octal       */ }
[1-9]{D}*                {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_INT; /* decimal     */ }

\'.\'|\'\\.\'            {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_CHAR;   /* character */  }
\"(\\.|[^"\\])*\"        {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_STRING; /* string    */  }


{D}+{E}                  {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_FLOAT; }
{D}*"."{D}+{E}?          {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_FLOAT; }
{D}+"."{D}*{E}?          {   yylval.string = new std::string(yytext, yyleng); return CONSTANT_FLOAT; }


    /* eat operators */
"&&"            {  return yylval.token = AND; }
"||"            {  return yylval.token = OR;  }
"<="            {  return yylval.token = LEQ; }
">="            {  return yylval.token = GEQ; }
"=="            {  return yylval.token = EQU; }
"!="            {  return yylval.token = NEQ; }
";"             {  return yylval.token = ';'; }
"{"             {  return yylval.token = '{'; }
"}"             {  return yylval.token = '}'; }
","             {  return yylval.token = ','; }
":"             {  return yylval.token = ':'; }
"="             {  return yylval.token = '='; }
"("             {  return yylval.token = '('; }
")"             {  return yylval.token = ')'; }
"["             {  return yylval.token = '['; }
"]"             {  return yylval.token = ']'; }
"."             {  return yylval.token = '.'; }
"&"             {  return yylval.token = GETAD; }
"!"             {  return yylval.token = NOT; }
"~"             {  return yylval.token = '~'; }
"-"             {  return yylval.token = MINUS; }
"+"             {  return yylval.token = PLUS; }
"*"             {  return yylval.token = MUL; }
"/"             {  return yylval.token = DIV; }
"%"             {  return yylval.token = '%'; }
"<"             {  return yylval.token = LT; }
">"             {  return yylval.token = GT; }
"|"             {  return yylval.token = '|'; }


[ \t\v\n\f]     { }
.               { printf("unknown token : %s in line: %d\n", yytext, yylineno); }

%%

int yywrap(void)
{
    return 1;
}


void check_comment(void)
{
    char ch, lastch;
    
    while (std::cin >> ch)      //find */
    {
        if (ch == '/' && lastch == '*')
            return;
        lastch = ch;
    }
    printf("illegal comment");
}


/*
 *
 * CS-252 Spring 2015
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

  static  void yyunput (int c,char *buf_ptr  );

  void myunputc(int c) {
    unput(c);
  }

%}

%%

\n 	{
  return NEWLINE;
}

[ \t] 	{
  /* Discard spaces and tabs */
}	

"&"     {
  return AND;
}

">" 	{
  return GREAT;
}

">>" 	{
  return GREATGREAT;
}

">&"     {
  return GREATAND;
}

">>&"     {
  return GREATGREATAND;
}

"<"     {
  return LESS;
}

"|"     {
  return PIPE;
}

\"[^\n\"]*\" {
      // do some operation to the yytext and yylval.string_val to remove the quotes 
      char * s = (char *) malloc(sizeof(char) * strlen(yytext));
      strcpy(s,yytext+1);
      s[strlen(s)-1] = 0;
      yylval.string_val = strdup(s);
      free(s);
      return WORD; 
}

[^ \t\n][^ \t\n]*  {
  /* Assume that file names have only alpha chars */
  yylval.string_val = strdup(yytext);
  return WORD;
}



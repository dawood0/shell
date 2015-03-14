
/*
 * CS-252 Spring 2015
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * You must extend it to understand the complete shell grammar.
 *
 */

%token	<string_val> WORD
%token 	NOTOKEN NEWLINE 
%token  GREAT GREATGREAT LESS AND
%token  GREATAND GREATGREATAND
%token  PIPE

%union	{
  char   *string_val;
}

%{
  //#define yylex yylex
#include <stdio.h>
#include "command.h"
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <regex.h>

  void yyerror(const char * s);
  int yylex();

%}

%%

goal: commands ;

commands: command | 
          commands command ;

command: simple_command ;

simple_command:	
command_with_pipe iomodifier background_opt NEWLINE {
  Command::_currentCommand.execute();
}
| NEWLINE 
| error NEWLINE { yyerrok; }
;

command_with_pipe:
command_with_pipe PIPE command_and_args
| command_and_args
;

command_and_args:
command_word arg_list {
  Command::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
}
;

arg_list:
arg_list argument
| /* can be empty */
;

argument:
WORD {
  expandWildCard(strdup(""),$1);
  //    Command::_currentSimpleCommand->insertArgument( $1 );
}
;

command_word:
WORD {
  Command::_currentSimpleCommand = new SimpleCommand();
  Command::_currentSimpleCommand->insertArgument( $1 );
}
;

iomodifier:
iomodifier iomodifier_opt 
|
;

iomodifier_opt: 
LESS WORD {
  if(!Command::_currentCommand._inputFile)
    Command::_currentCommand._inputFile = $2;
  else {
    yyerror("Ambiguous input redirect.\n");
    Command::_currentCommand._error = true;
  }
}
| GREAT WORD {
  if(!Command::_currentCommand._outFile)
    Command::_currentCommand._outFile = $2;
  else {
    yyerror("Ambiguous output redirect.\n");
    Command::_currentCommand._error = true;
  }
} 
| GREATGREAT WORD {
  if(!Command::_currentCommand._outFile){
    Command::_currentCommand._outFile = $2;
    Command::_currentCommand._append = true;
  }
  else {
    yyerror("Ambiguous output redirect.\n");
    Command::_currentCommand._error = true;
  }

}
| GREATAND WORD {
  if(!Command::_currentCommand._errFile){
    Command::_currentCommand._errFile = $2;
    Command::_currentCommand._outFile = $2;
  }
  else {
    yyerror("Ambiguous output redirect.\n");
    Command::_currentCommand._error = true;
  }

}
| GREATGREATAND WORD {
  if(!Command::_currentCommand._errFile){
    Command::_currentCommand._errFile = $2;
    Command::_currentCommand._outFile = $2;
    Command::_currentCommand._append = true;
  }
  else {
    yyerror("Ambiguous output redirect.\n");
    Command::_currentCommand._error = true;
  }
} ;

background_opt:
AND {
  Command::_currentCommand._background = 1;
} 
| /* can be empty */ 
;
%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif

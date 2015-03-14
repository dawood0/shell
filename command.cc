/*
 * CS252: Shell project
 * Aishwarya Ajay
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
//Added for open()
#include <sys/stat.h>
#include <fcntl.h>
//For wildcarding
#include <regex.h>
#include <dirent.h>
#include "command.h"

extern char **environ;

// From qsort man pages
int cmpstr(void const *a, void const *b) { 
  const char **ia = (const char **)a;
  const char **ib = (const char **)b;
  return strcmp(*ia, *ib);
}

SimpleCommand::SimpleCommand()
{ 
  // Creat available space for 5 arguments
  _numberOfAvailableArguments = 5;
  _numberOfArguments = 0;
  _arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
  if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
    // Double the available space
    _numberOfAvailableArguments *= 2;
    _arguments = (char **) realloc( _arguments,
				    _numberOfAvailableArguments * sizeof( char * ) );
  }
	
  _arguments[ _numberOfArguments ] = argument;

  // Add NULL argument at the end
  _arguments[ _numberOfArguments + 1] = NULL;
	
  _numberOfArguments++;
}

Command::Command()
{
  // Create available space for one simple command
  _numberOfAvailableSimpleCommands = 1;
  _simpleCommands = (SimpleCommand **)
    malloc( _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );

  _numberOfSimpleCommands = 0;
  _outFile = 0;
  _inputFile = 0;
  _errFile = 0;
  _background = 0;
  _append = false;
  _error = false;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
  if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
    _numberOfAvailableSimpleCommands *= 2;
    _simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
						  _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
  }
	
  _simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
  _numberOfSimpleCommands++;
}

void
Command::clear()
{
  for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
    for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
      free ( _simpleCommands[ i ]->_arguments[ j ] );
    }
		
    free ( _simpleCommands[ i ]->_arguments );
    delete  _simpleCommands[ i ]; // This is a class, so we must delete it. Don't mix free/delete.
  }

  if ( _outFile ) {
    free( _outFile );
  }

  if ( _inputFile ) {
    free( _inputFile );
  }

  if ( _errFile && _errFile != _outFile ) {
    free( _errFile );
  }

  _numberOfSimpleCommands = 0;
  _outFile = 0;
  _inputFile = 0;
  _errFile = 0;
  _background = 0;
  _append = false;
  _error = false;
}

void
Command::print()
{
  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   Simple Commands\n");
  printf("  --- ----------------------------------------------------------\n");
	
  for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
    printf("  %-3d ", i );
    for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
      printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
    }
    printf("\n");
  }

  printf( "\n\n" );
  printf( "  Output       Input        Error        Background\n" );
  printf( "  ------------ ------------ ------------ ------------\n" );
  printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
	  _inputFile?_inputFile:"default", _errFile?_errFile:"default",
	  _background?"YES":"NO");
  printf( "\n\n" );
	
}

void
Command::execute()
{
  // Don't do anything if there are no simple commands
  if ( _numberOfSimpleCommands == 0 || _error ) {
    clear();
    prompt();
    return;
  }
  
  pid_t pid;
	
  //Storing default I/O values
  int defaultin = dup(0);
  int defaultout = dup(1);
  int defaulterr = dup(2);

  // Checking if the curren command has input/output redirection 
  // and setting file descriptors accordingly
  int command_input = dup(0);
  int command_output = dup(1);
  int command_error = dup(2);
  
  if(_inputFile) {
    int f = open(_inputFile, O_RDONLY); 
    if( f < 0) exit(2); 
    dup2(f ,command_input); 
    close(f);
  }

  if(_outFile) {
    int f;
    if(_append)
      f = open(_outFile,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
    else
      f = open(_outFile,O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
    if( f < 0) exit(2); 
    dup2(f ,command_output); 
    close(f);
  }
  
  if(_errFile) {
    int f;
    if(_append)
      f = open(_errFile,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP);
    else
      f = open(_errFile,O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
    if( f < 0) exit(2); 
    dup2(f ,command_error); 
    close(f);
  }
  
  //Pipe
  int fd[2];
  int pin = dup(command_input);  

  for (int i = 0; i < _numberOfSimpleCommands; i++) {

    if( !strcmp(_simpleCommands[i]->_arguments[0],"exit") )
      {
	// Restore input, output, and error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	
	// Close uwnanted file descriptors
	close(command_input);
	close(command_output);
	close(command_error);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	close(pin);
	
	clear();
	
	printf("Good Bye!!\n");
	fflush(stdout);
	exit(0);
      }
    else if(!strcmp(_simpleCommands[i]->_arguments[0],"setenv"))
      {
	// Restore input, output, and error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	
	// Close uwnanted file descriptors
	close(command_input);
	close(command_output);
	close(command_error);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	close(pin);
	
	if(_simpleCommands[i]->_numberOfArguments == 3) {
	  char * string = (char * )malloc((strlen(_simpleCommands[i]->_arguments[1])+strlen(_simpleCommands[i]->_arguments[2])+2)*sizeof(char));
	  strcpy(string,_simpleCommands[i]->_arguments[1]);
	  strcat(string,(char *) "=");
	  strcat(string,_simpleCommands[i]->_arguments[2]);
	  putenv(string);
	}
	else
	  printf("Invalid usage\n");
	
	clear();
	prompt();
	return;
      }
    else if(!strcmp(_simpleCommands[i]->_arguments[0],"unsetenv"))
      {	
	// Restore input, output, and error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	
	// Close uwnanted file descriptors
	close(command_input);
	close(command_output);
	close(command_error);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	close(pin);

	if(_simpleCommands[i]->_numberOfArguments == 2) {

	  int j = 0;
	  bool shift = false;
	  int eq = 1;
	  while(environ[j]) {
	    //	    char * s =strstr(environ[j],"=");
	    
	    if(!shift) eq = strncmp(environ[j],
				    _simpleCommands[i]->_arguments[1],
				    strlen(_simpleCommands[i]->_arguments[1])
				    );
	    if(eq == 0) shift = true;
	    if(shift)
	      if(environ[j+1])
		strcpy(environ[j],environ[j+1]);
	      else
		environ[j] = NULL;
	    j += 1;
	  }
	}
	else
	  printf("Invalid usage\n");
	
	clear();
	prompt();
	return;
      }
    else if(!strcmp(_simpleCommands[i]->_arguments[0],"cd"))
      {	
	// Restore input, output, and error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);
	
	// Close uwnanted file descriptors
	close(command_input);
	close(command_output);
	close(command_error);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	close(pin);

	if(_simpleCommands[i]->_numberOfArguments == 2) 
	  chdir(_simpleCommands[i]->_arguments[1]);
	else 
	  chdir(getenv("HOME"));
	
	clear();
	prompt();
	return;
      }
    
    if(pipe(fd) == -1) { 
      perror("Piper error\n"); 
      clear();
      exit(2); 
    }
    
    pid = fork();
    
    if (pid < 0) {
      perror("Fork failed\n");
      clear();
      exit(2);
    }
    
    if(pid == 0) { // Child process
      
      dup2(pin,0); 
      
      if(i == _numberOfSimpleCommands - 1)
	dup2(command_output,1);
      else
	dup2(fd[1],1); 
      
      dup2(command_error,2);
      
      close(fd[0]);
      close(fd[1]);
      close(pin);
      close(command_input);
      close(command_output);
      close(command_error);
      
      if( !strcmp(_simpleCommands[i]->_arguments[0],"printenv") ){
	int j = 0;
	while(environ[j])
	  printf("%s\n",environ[j++]);
	clear();
	exit(0);
      }
      
      execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
      perror("Exec error\n");
      clear();
      exit(1);
    } 
    
    dup2(fd[0],pin);
    close(fd[0]);
    close(fd[1]);
  }
  
  if(!_background) waitpid(pid,0,0);
  
  // Restore input, output, and error
  dup2(defaultin, 0);
  dup2(defaultout, 1);
  dup2(defaulterr, 2);
  
  // Close uwnanted file descriptors
  close(command_input);
  close(command_output);
  close(command_error);
  close(defaultin);
  close(defaultout);
  close(defaulterr);
  close(pin);
  
  // Clear to prepare for next command
  clear();
  
  // Print new prompt
  prompt();
}
  
// Shell implementation
extern "C" void handleSig(int sig)
{
  switch(sig){
  case SIGINT:
    Command::_currentCommand.clear();
    printf("\n");
    Command::_currentCommand.prompt();
    break;
  case SIGCHLD:
    wait3(0,0,NULL);
    while(waitpid(-1, NULL, WNOHANG) > 0);
    break;
  }
}

void
Command::prompt()
{
  if(isatty(0)){
    printf("myshell>");
  }
  fflush(stdout);
}

void expandWildCard(char * prefix, char * suffix)
{ 
  //  printf("ewc1 : %s, %s\n",prefix,suffix);
  
  if(!suffix[0]) {
    //printf("arg : %s\n",prefix);
    Command::_currentSimpleCommand->insertArgument(strdup(prefix));
    return;
  }
  
  char * s = strchr(suffix, '/');
  char component[1024];

  if (s != NULL) {
    strncpy(component,suffix,s-suffix);
    component[s-suffix] = 0;
    suffix = s + 1;
    //    printf("comp1 : %s\n",component);
  } 
  else {
    strcpy(component, suffix);
    //    component[strlen(suffix)+1] = 0;
    suffix += strlen(suffix);
    //    printf("comp2 : %s\n",component);
  }
    
  char newPrefix[1024];

  if(strchr(component,'*') == NULL && strchr(component,'?') == NULL){
    //    Command::_currentSimpleCommand->insertArgument(arg);
    if(!strlen(prefix) && !strlen(component))
      sprintf(newPrefix, "/");
    else if(!strlen(prefix) && strlen(component))
      sprintf(newPrefix, "%s", component);
    else if(!strcmp(prefix,"/"))
      sprintf(newPrefix, "/%s", component);
    else
      sprintf(newPrefix, "%s/%s", prefix, component);

    expandWildCard(newPrefix, suffix);
    return;
  }
  
  
  char * rege = (char *) malloc(2*strlen(component) + 10);
  char * a = component;
  char * r = rege;
  // Replacing * and ? with regex characters 
  *r = '^'; r++;
  while(*a) {
    if (*a == '*') { *r='.'; r++; *r='*'; r++;}
    else if (*a == '?') { *r='.' ; r++;}
    else if (*a == '.') { *r='\\'; r++; *r='.'; r++;}
    else { *r=*a; r++;}
    a++;
  }
  *r='$'; r++; *r=0;
  // Compile RegEx
  regex_t re;
  int result = regcomp(&re, rege, REG_EXTENDED|REG_NOSUB);
  if(result != 0) {
    perror("RegEx Compile");
    Command::_currentCommand._error = true;
    return;
  }
  // Check files in Directory
  DIR * dir = NULL;

  if(strlen(prefix)) dir = opendir(prefix);
  else dir = opendir(".");

  if (dir == NULL) {
    perror("Cannot open directory");
    Command::_currentCommand._error = true;
    return;
  }
  
  struct dirent * ent;
  int maxEntries = 20;
  int nEntries = 0;
  char ** array = (char **) malloc(maxEntries*sizeof(char*));
  if (array == NULL) {
    perror("Insufficient memory");
    Command::_currentCommand._error = true;
    return;	  
  }
  regmatch_t match;
  while( (ent = readdir(dir)) != NULL) {
    if(regexec(&re, ent->d_name, 1, &match, 0) == 0 && 
       !((component[0] == '*' || component[0] == '?') && ent->d_name[0] == '.' )) {
      if (nEntries == maxEntries) {
	maxEntries *= 2;
	array = (char **) realloc(array, maxEntries*sizeof(char*));
	if (array == NULL) {
	  perror("Insufficient memory");
	  Command::_currentCommand._error = true;
	  return;	  
	}
      }
      array[nEntries] = strdup(ent->d_name);
      nEntries += 1;
    }
  }
  closedir(dir);

  qsort(array, nEntries, sizeof(char*), cmpstr);
  //  sortArrayStrings(array, nEntries);
  for (int i = 0; i < nEntries; i++) {
    if(!strlen(prefix))
      sprintf(newPrefix, "%s", array[i]);
    else
      sprintf(newPrefix, "%s/%s", prefix, array[i]);
    //    printf("np2 : %s\n",newPrefix);
    struct stat buf;
    int st = stat(newPrefix, &buf);
    if(!S_ISDIR(buf.st_mode) && *suffix != 0)
      continue;
    else
      expandWildCard(newPrefix, suffix);

  } 

  regfree(&re);
  free(rege);
  free(array);
}



Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int main(int argc, char **argv)
{
  struct sigaction si, sc;
  si.sa_handler = handleSig;
  si.sa_flags = SA_RESTART;
  sc.sa_handler = handleSig;
  sc.sa_flags = SA_RESTART;
  sigaction(SIGINT, &si, NULL);
  sigaction(SIGCHLD, &sc, NULL);
  Command::_currentCommand.prompt();
  yyparse();
  
  return 0;
}


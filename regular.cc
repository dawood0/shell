#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>

std::vector<char *> expandWildCard(char * string){
  std::vector<char *> a;
  int length = strlen(string);
  char * regExp = (char*)malloc(1024*sizeof(char));
  int j = 1;
  regExp[0] = '^';
  for(int i = 0; i < length; i++) {
    if(string[i] == '*')
      if(i) {
	strcat(regExp,"[:print:]*");
      }
      else {
	strcat(regExp,"[^.][^[:blank:]]*");
      }
    else if (string[i] == '?')
      if(i) {
	strcat(regExp,"[:print:]?");
      }
      else {
	strcat(regExp,"[^.]?");
      }
    else {
      j = strlen(regExp);
      regExp[j] = string[i];
    }
  }
  j = strlen(regExp);
  regExp[j] = '$';
  printf("%s\n",regExp);

  DIR * dip;
  struct dirent * dit;

  if((dip = opendir(".")) == NULL) {
    printf("A\n");
    exit(0);
  }

  regex_t re;
  int result = regcomp( &re, regExp,  REG_EXTENDED|REG_NOSUB);

  if( result != 0 ) {
    printf("Aa\n");
    exit(0);
  }
  regmatch_t match;
  while((dit = readdir(dip)) != NULL) {
    result = regexec( &re, dit->d_name, 1, &match, 0 );
    if(!result)
      printf("%s, %d\n",dit->d_name, result);
  }

  exit(0);
  a.push_back(regExp);
  return a;
  char * tok;
  tok = strtok(string,"/");
  while(tok != NULL){
    printf("%s\n",tok);
    tok = strtok(NULL,"/");
  }
  a.push_back("AAA");
  return a;

  
  

  /*
  //char * regExp = (char*)malloc(strlen(string)*sizeof(char));
  sprintf(regExp,"^%s$",string);
  regex_t re;
  int result = regcomp( &re, regExp,  REG_EXTENDED|REG_NOSUB);

  if( result != 0 ) {
    exit(0);
  }

  regmatch_t match;
  while((dit = readdir(dip)) != NULL) {
    result = regexec( &re, dit->d_name, 1, &match, 0 );
    printf("%s, %d\n",dit->d_name, result);
  }

  free(regExp);
  */
  return a;
}


int main(int argc, char ** argv){
  char string[1024];
  strcpy(string,"*.c");
  std::vector<char *> a = expandWildCard(string);
  int i = 0;
  for(i = 0; i < a.size(); i++)
    printf("%s\n",a[i]);
  a.pop_back();
  return 0;
}

// CS252 Fall 2015
//
// Shell Project
// tty.h
//
// Line Editor Library
//
// Contains the tty editor functions for the Shell Project. Invoked
// only in tty mode. History is stored in the home folder similar to bash.
// Use a history file name prefixed with a '.' if you want it to be hidden.

#ifndef _tty
#define _tty

#define TTY_MAX_LINEBUFFER 2048

#ifdef __cplusplus
extern "C" {
#endif
void ttyinit(const char *prompt, const char *histfile); // Activate tty mode. prompt is your shell prompt. histfile is your history file.
void ttyteardown();                                     // Deactivate tty mode.
char *ttygetline(int lineMax);                          // Get a line with up to lineMax available chars.
void ttyprompt();                                       // Print the shell prompt.
#ifdef __cplusplus
}
#endif
#endif

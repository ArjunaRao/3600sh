/*
 * CS3600, Spring 2013
 * Project 1
 * (c) 2013 Alan Mislove
 *
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void prompt();
int get_line(FILE *fp, char *buffer, size_t buflen);
int organize_args(char **arglist, char *line, int args);
int io_redirection(char **arglist);
void reset_redirection(int old_i, int old_o, int old_e);
int execute(char **arglist, int holdup);
void do_exit();

#endif

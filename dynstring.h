#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _dynstring_h
#define _dynstring_h

#define BUFFER_SIZE 65536

typedef char *string;

int count_lines(FILE*);
void split(const string*, string*, const char);
void strsort(string*, const int);
string shift (FILE*, const int);
string readline(FILE*, const int);

#endif

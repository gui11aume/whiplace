#ifndef _dynstring_h
#define _dynstring_h

#define BUFFER_SIZE 65536

typedef char *string;

typedef struct {
   string uffer;
   int size;
   int pos;
} buffin;


void strsort(string*, int);
string shift (FILE*, int);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynstring.h"

#ifndef _whiplace_h
#define _whiplace_h


/* Array of key-value pairs. */
struct keyval {
   string *keys;
   string *values;
   int nkeys;
};

/* Search-tree node. */
struct keynode {
    char branch;    // Whether node branches.
    int key;        // The key index, or -1.
    string chars;   // Characters to match.
    struct keynode **children;
};


struct keyval get_key_values (FILE*);
struct keynode *newnode(void);
void build_tree(struct keynode*, int, const int,
            const string*, const int);
int find_char(const char, const string);
int whip(const string, struct keynode*);
void whiplace(FILE*, FILE*, FILE*);

#endif

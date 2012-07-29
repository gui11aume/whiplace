#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynstring.h"

#ifndef _WHIPLACE_H
#define _WHIPLACE_H

#define IO_BUFFER_SIZE 65536

/* Key-value pairs. */
struct keyval {
   char **keys;
   char **values;
   int nitems;
};

/***********************************************************************
  Trie nodes contain a single character sub-key, a key index and a list 
  of child nodes. A tail node is the last node in a path that matches a 
  key. Non tail nodes have an invalid 'keyid' value equal to -1, tail   
  nodes have a valid 'keyid' index. Tail nodes do not need to be leaves 
  because they can have child nodes if the key prefixes another key.    
***********************************************************************/
struct trienode {
    char subkey;
    int  keyid;        
    struct node **childnodes;
};

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynstring.h"

/*
whiplace: multiple stream replacement
USAGE:
  whiplace keyfile targetfile
*/

/* Array of key-value pairs. */
struct keyval {
   string *keys;
   string *values;
   int nkeys;
};

/* Search-tree node. */
struct keynode {
   char branch;   // Whether node branches.
   int key;       // The key index, or -1.
   char *chars;   // Characters to match.
   struct keynode **children;
};


/* Function declarations. */
struct keyval get_key_values (FILE*);
struct keynode *newnode(void);
void build_tree(struct keynode*, int, const int,
      const string*, const int);
int find_char(const char, const string);
int whip(const string, struct keynode*);




struct keyval get_key_values (FILE *keyfile) {
/* Read in key-value pairs from file, one pair per line. */

   const int nkeys = count_lines(keyfile);
   string *keys = (string * const) malloc((nkeys+1) * sizeof(string));
   string *values = (string * const) malloc((nkeys+1) * sizeof(string));
   if ((keys == NULL) || (values == NULL)) {
      fprintf(stderr, "memory error\n");
      exit(EXIT_FAILURE);
   }

   string line;
   int j, i = 0;

   /* Read and chomp lines from key file. */
   while ((line = readline(keyfile, 1)) != NULL) {

      /* Allocate memory for key + value. */
      char *curPtr = (string) malloc((strlen(line) + 1) * sizeof(char));
      if (curPtr == NULL) {
         fprintf(stderr, "memory error\n");
         exit(EXIT_FAILURE);
      }

      strcpy(curPtr, line);
      keys[i++] = curPtr;

   }

  /* Sentinels */
   keys[i] = NULL;
   values[i] = NULL;

   fclose(keyfile);

  /* Sort ans split the key-values strings. */
   strsort(keys, nkeys);
   split(keys, values, '\t');

   struct keyval kv = { 
      .keys = keys, 
      .values = values,
      .nkeys = nkeys
   };

   return kv;

}


struct keynode *newnode(void) {
/*
 * Node creation function. Proper initialization is
 * required to later reclaim the allocated memory.
 */
   struct keynode *new = NULL; // Initialization.
   new = malloc(sizeof(struct keynode));
   if (new != NULL) {
      new->branch = 1;
      new->key = -1;
      new->chars = NULL;
      new->children = NULL;
   }
   else {
      fprintf(stderr, "memory error\n");
      exit(EXIT_FAILURE);
   }
   return new;
}


void build_tree(struct keynode *thisnode, int down, const int up,
      const string *keys, const int depth) {
/* Recursively build a key search-tree of keynodes. */

  /* Temp arrays. */
   int min[256], max[256];
  /* Allocate too much. Will realloc() later. */
   thisnode->chars = (string) malloc(256);

  /*
   * If a key finishes here, the key index
   * is specified (and the key is skipped).
   */
   if (keys[down][depth] == '\0') {
      thisnode->key = down++;
   }

   /* -- DRY function for memory allocation. -- */
   void allocate(int k) {
     /* Allocate memory for characters and children. */ 
      thisnode->children = \
         (struct keynode **) calloc(k+1, sizeof(struct keynode *));
      if (thisnode->children == NULL) {
         fprintf(stderr, "memory error\n");
         exit(EXIT_FAILURE);
      } 
     /* Add the sentinels. */
      thisnode->chars[k] = '\0';
      thisnode->children[k] = NULL;
   }
   /* -- End of DRY function. -- */


  if (down > up) { 
  /* This is a leaf node. */
      allocate(0);
   }
   else {
  /* Not a leaf node. */

      int i, j = 0;
      thisnode->chars[0] = keys[down][depth];
      min[0] = max[0] = down;

     /* Gather and count letters at given depth. */
      for (i = down + 1 ; i < up + 1 ; i++) {
         if (thisnode->chars[j] !=  keys[i][depth]) {
           /* New character. */
            j++;
            thisnode->chars[j] = keys[i][depth];
            min[j] = max[j] = i;
         }
         else {
           /* Increment total count for character. */
            ++max[j];
         }
      }

     /* Allocate memory for (j+1) children. */
      allocate(j+1);

     /* Depth-first recursion. */
      for (i = 0 ; i < j + 1 ; i++) {
         thisnode->children[i] = newnode();
         build_tree(thisnode->children[i], min[i], max[i],
            keys, depth + 1);
      }
   }

  /* Node merging on the way back from recursion. */

   if (strlen(thisnode->chars) == 1) {
      (*thisnode).branch = 0;
      const struct keynode *child = thisnode->children[0];

     /* Merge this node if it has only one child. */
      if ((child->branch == 0) && (child->key == -1)) {
        /* Concatenate characters in tmp array. */
         strcpy(thisnode->chars + 1, child->chars);

        /* Reallocate memory. */
         thisnode->chars = \
             realloc(thisnode->chars, strlen(thisnode->chars)+1);

        /* Point to grand-child and erase child. */
         struct keynode *grandChild = child->children[0];
         free(thisnode->children[0]);
         thisnode->children[0] = grandChild;
      }
   }
}



int find_char(const char c, const string sorted_keys) {
/*
 * Find char c in sorted string by bisection. Return
 * its index in sorted_keys if match, -1 otherwise.
 */

   int down = 0;
   int up = strlen(sorted_keys) - 1;
   int diff;

   while (up > down) {
      diff = sorted_keys[(up+down)/2] - c; 
      if (diff < 0) {
        /* The key is too small, we can it. */
         down = (up + down) / 2 + 1;
      }
      else if (diff > 0) {
        /* The key is too large, we can also skip. */
         up = (up + down) / 2 - 1;
      }
      else {
        /* Found the match. */
         return (up + down) / 2;
      }
   }
  /*
   * Now up and down are either equal, or up < down.
   * In the first case we need to check one last key,
   * in the second, we're already done.
   */
   if ((up == down) && (sorted_keys[up] == c)) {
       return up;
   }
   /* No match. */
   return -1;
}


int keycomp (string stream, string key) {
/* 
 * Compare a key to a stream. Return -1, if no match is
 * found in the stream, the length of the key otherwise.
 */

   int i = 0;

   while (key[i] == stream[i]) {
      i++;
      if (key[i] == '\0') {
        /* Matched until end of key. */
         return i;
      }
   }

  /* Did not match. */
   return -1;

}


int whip(const string stream, struct keynode *node) {
/*
 * Match the current position of the stream with
 * the key tree. Return -1 if no match is found,
 * and the key index otherwise.
 */

   char c = stream[0];
   int j, i = 0;
   int charmatch, keymatch = -1;
   do {
      if ((*node).key > -1) {
         keymatch = (*node).key;
      }
      /* Find char match. */
      if ((*node).branch) {
        /* Node is a branch: find where to go next */
         charmatch = find_char(c, (*node).chars);
         j = 1;
      }
      else {
        /* Node is a stem: check whether it matches stream. */
         j = keycomp(stream + i, (*node).chars);
         charmatch = 0;
      }
      if ((charmatch > -1) && (j > -1)) {
         node = (*node).children[charmatch];
         i += j; // 1 if branch, key length otherwise.
         c = stream[i];
      }
   }
  /*
   * Continue until no character matches. Leaf 
   * nodes can't match, which stop the loop.
   */
   while ((charmatch > -1) && (j > -1));

   return keymatch;

}



void whiplace (string keyfname, string stream, string out) {

   FILE *keyf = fopen(keyfname, "r");
   FILE *streamf = stream == NULL ? stdin : fopen(stream, "r");
   FILE *outf = out == NULL ? stdout : fopen(out, "w");

   if (keyf == NULL) {
      fprintf(stderr, "cannot open key file %s\n", keyfname);
      exit(EXIT_FAILURE);
   }

   if (streamf == NULL) {
      fprintf(stderr, "cannot open stream %s\n", stream);
      exit(EXIT_FAILURE);
   }

   if (outf == NULL) {
      fprintf(stderr, "cannot open file %s for writing\n", out);
      exit(EXIT_FAILURE);
   }

   int i;

  /* Get and sort keys + values. */
   const struct keyval kv = get_key_values(keyf);

  /* Check key duplicates. */
   for (i = 0 ; i < kv.nkeys -2; i++) {
      if (strcmp(kv.keys[i], kv.keys[i+1]) == 0) {
         fprintf(stderr, "key '%s' duplicated\n", kv.keys[i]);
         exit(EXIT_FAILURE);
      }
   }

  /* Build the key tree. */
   struct keynode * const root = newnode();
   build_tree(root, 0, kv.nkeys-1, kv.keys, 0);

  /* Let it whip! */
   string buffer = (string) shift(streamf, 0);

   while (buffer[0] != '\0') {
      i = whip(buffer, root);
      if (i > -1) {
        /* Found a match. */
         fprintf(outf, "%s", kv.values[i]);
         buffer = (string) shift(streamf, strlen(kv.keys[i]));
      }
      else {
         fprintf(outf, "%c", buffer[0]);
         buffer = (string) shift(streamf, 1);
      }
   }
  /* Wrap up. */
   fflush(outf);
   close(keyf);
   close(streamf);
   close(out);

   exit(EXIT_SUCCESS);

}

int main (int argc, string argv[]) {

   string USAGE = "\n"
"whiplace: multiple stream replacement\n\n"
"USAGE:\n"
"   whiplace keyfile targetfile\n\n";


  /* Options and arguments processing. */

   if ((argc < 2) || (argc > 4)) {
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   string keyfname = NULL;

   keyfname = argv[1];
   string stream = argc > 2 ? argv[2] : NULL;
   string out = argc > 3 ? argv[3] : NULL;

  /* (End of option parsing). */

   whiplace(keyfname, stream, out);

   return 0;

}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynstring.h"


#define USAGE "\n\
whiplace: multiple stream replacement\n\
\n\
USAGE:\n\
   whiplace keyfile targetfile\n\n"


/* Array of key-value pairs. */
struct keyval {
   string *keys;
   string *values;
   int nkeys;
};

/* Search-tree node. */
struct keynode {
   int key; // The key index, if any.
   char *chars;
   struct keynode **children;
};



struct keynode build_tree(struct keynode *thisnode, int down, int up,
      string *keys, int depth) {
/* Recursively build a key search-tree of keynodes. */

  /* Temp arrays. */
   char chars[256];
   int min[256], max[256];

  /*
   * If a key finishes here, the key index
   * is specified (and the key is skipped).
   */
   (*thisnode).key = (keys[down][depth] == '\0') ? down++ : -1;

   if (down > up) { 
  /* This is a leaf node. */

      (*thisnode).chars = (char *) malloc(sizeof(char));
      (*thisnode).chars[0] = '\0';

      (*thisnode).children = \
         (struct keynode **) malloc(sizeof(struct keynode *));
      (*thisnode).children[0] = NULL;

   }
   else {
  /* Not a leaf node. */

      int i, j = 0;
      chars[0] = keys[down][depth];
      min[0] = max[0] = down;

     /* Gather letters at given depth. */
      for (i = down + 1 ; i < up + 1 ; i++) {
         if (chars[j] !=  keys[i][depth]) {
           /* New character. */
            j++;
            chars[j] = keys[i][depth];
            min[j] = max[j] = i;
         }
         else {
           /* Increment total count for character. */
            ++max[j];
         }
      }

     /* Now we know how much memory to allocate for children. */
      (*thisnode).chars = (char *) malloc((j+2) * sizeof(char));
      (*thisnode).children = \
         (struct keynode **) malloc((j+2) * sizeof(struct keynode *));

      chars[j+1] = '\0';
      strcpy((*thisnode).chars, chars);

     /* Depth-first recursion. */
      for (i = 0 ; i < j + 1 ; i++) {
         (*thisnode).children[i] = \
            (struct keynode *) malloc (sizeof(struct keynode *));
         build_tree((*thisnode).children[i], min[i], max[i],
            keys, depth + 1);
      }
     /* Sentinel. */
      (*thisnode).children[i] = NULL;
   }

}


struct keyval get_key_values (FILE *keyfile) {
/* Read in key-value pairs from file, one pair per line. */

   const int nkeys = count_lines(keyfile);
   string *keys = (string *) malloc((nkeys+1) * sizeof(string));
   string *values = (string *) malloc((nkeys+1) * sizeof(string));

   char line[BUFFER_SIZE];
   int j, i = 0;

   while (fgets(line, sizeof(line), keyfile) != NULL) {

      int llen = strlen(line);

      if (llen >= BUFFER_SIZE - 1) {
        /* Too unlikely to fix (for now). */
         fprintf(stderr, "key-value line too long: %s", line);
         exit(EXIT_FAILURE);
      }
     /* Chomp. */
      if (line[llen-1] == '\n') {
         line[--llen] = '\0';
      }

      char *curPtr = (string) malloc((llen + 1) * sizeof(char));
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

int find(char c, string sorted_keys) {
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


int match(string stream, struct keynode *node) {
/*
 * Match the current position of the stream with
 * the key tree. Return -1 if no match is found,
 * and the key index otherwise.
 */

   char c = stream[0];
   int i = 0;
   int charmatch, keymatch = -1;

   do {
      if ((*node).key > -1) {
         keymatch = (*node).key;
      }
      /* Find char match. */
      charmatch = find(c, (*node).chars);
      if (charmatch > -1) {
         node = (*node).children[charmatch];
         c = stream[++i];
      }
   }
  /*
   * Continue until no character matches. Leaf 
   * nodes can't match, which stop the loop.
   */
   while (charmatch > -1);

   return keymatch;

}


int main (int argc, string argv[]) {

   int i;
   string keyfname = NULL;

   /* Options and arguments processing. */

   if ((argc < 2) || (argc > 3)) {
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   keyfname = argv[1];
   FILE *keyfile = fopen(keyfname, "r");
   FILE *streamf = argc == 2 ? stdin : fopen(argv[2], "r");

   if (!keyfile) {
      fprintf(stderr, "cannot open file %s\n", keyfname);
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   if (streamf == NULL) {
      fprintf(stderr, "cannot open file %s\n", argv[2]);
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   /* (End of option parsing). */

   /* Get and sort key+values. */
   struct keyval kv = get_key_values(keyfile);
   close(keyfile);

   /* Build the key tree. */
   struct keynode *root = \
         (struct keynode *) malloc(sizeof(struct keynode));
   build_tree(root, 0, kv.nkeys-1, kv.keys, 0);

   struct keynode *r = root;


   /* whiplace. */
   string buffer = (string) shift(streamf, 0);

   while (buffer[0] != '\0') {
      i = match(buffer, root);
      if (i > -1) {
         /* Match found. */
         fprintf(stdout, "%s", kv.values[i]);
         buffer = (string) shift(streamf, strlen(kv.keys[i]));
      }
      else {
         fprintf(stdout, "%c", buffer[0]);
         buffer = (string) shift(streamf, 1);
      }
   }

   /* Wrap up. */
   close(streamf);
   fflush(stdout);

   exit(EXIT_SUCCESS);
}

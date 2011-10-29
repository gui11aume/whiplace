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

   /* Working arrays. */
   char chars[256];
   int min[256], max[256];
   struct keynode *pointers[256];

  /*
   * If a key finishes here, the key index
   * is specified (and the key is skipped).
   */
   (*thisnode).key = (keys[down][depth] == '\0') ? down++ : -1;

   int i, j = 0;
   char character = '\0';

   /* Gather letters at given depth (void if thisnode is a leaf). */
   for (i = down ; i < up + 1 ; i++) {
      if (character !=  keys[i][depth]) {
         /* New character. */
         character = keys[i][depth];
         min[j] = i;
         max[j] = i;
         chars[j] = character;
         pointers[j] = \
               (struct keynode *) malloc (sizeof(struct keynode *));
         j++;
      }
      else {
         ++max[j-1];
      }
   }


   (*thisnode).chars = (char *) malloc((j+1) * sizeof(char));
   (*thisnode).children = \
         (struct keynode **) malloc((j+1) * sizeof(struct keynode *));

   strcpy((*thisnode).chars, chars);

   for (i = 0 ; i < j ; i++) {
      (*thisnode).children[i] = pointers[i];
      /* Depth-first recursion. */
      build_tree(pointers[i], min[i], max[i], keys, depth+1);
   }
   /* Sentinel. */
   (*thisnode).children[i] = NULL;

}



int count_keys(FILE *keyfile) {
/* 
 * Use 'fgets' to run through the lines of keyfile.
 * We need a big buffer to ensure that we hit an end of line
 * on every call. The cost in memory is 65 Kb, which are freed
 * upon return. This is affordable. This code is almost as
 * fast as UNIX 'wc -l', and much faster than reading characters
 * one by one.
 */

   int lc = 0;
   char buffer[BUFFER_SIZE];

   /* Count lines and reset. */
   while(fgets(buffer, sizeof(buffer), keyfile) != NULL) lc++;
   fseek(keyfile, 0, SEEK_SET);

   return lc;

}



void split(string* keys, string* values) {
/* 
 * Split key-value pairs on tab. Keys and values are
 * read and sorted together as a single line for speed
 * and memory efficiency. They are separated on the
 * first tab by replacing it with '\0' and the value
 * pointer is assigned to the next character.
 */

   int i, j, notab;

   /* Run through the keys. */
   for (i = 0 ; keys[i] != NULL ; i++) {
      notab = 1;
      for (j = 0 ; j < strlen(keys[i]) ; j++) {
         if (keys[i][j] == '\t') {
            /* Found the tab. */
            keys[i][j] = '\0';
            values[i] = keys[i] + j+1;
            notab = 0;
            break;
         }
      }
      if (notab) {
         /* Ooops, forgot the tab? */
         fprintf(stderr, "no tab in line %d (%s)\n", i+1, keys[i]);
         fprintf(stderr, USAGE);
         exit(EXIT_FAILURE);
      }     
   }
}


int keycomp (string key, string stream) {
/* 
 * Compare a key to a stream.Return -1, 0 or 1
 * with 0 if akey match is found in the stream.
 */

   int i = 0;

   while (key[i] == stream[i]) {
      i++;
      if (key[i] == '\0') {
         /* Match until end. */
         return 0;
      }
   }

   return key[i] < stream[i] ? -1 : 1;

}


struct keyval get_key_values (FILE *keyfile) {
/* Read in key-value pairs from file, one pair per line. */

   const int nkeys = count_keys(keyfile);
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
   split(keys, values);


   struct keyval kv = { 
      .keys = keys, 
      .values = values,
      .nkeys = nkeys
   };

   return kv;

}


int match(string stream, struct keynode *node) {
   int j, i = 0;
   int match;
   char t, c = stream[i];
   int last_match = -1;
   while(1) {
      if ((*node).key > -1) {
         last_match = (*node).key;
      }
      if ((*node).children[0] == NULL) {
         break;
      }
      j = 0;
      match = -1;
      while ((t = (*node).chars[j++]) != '\0') {
         if (c == t) {
            match = j-1;
            break;
         }
      }
      if (match > -1) {
         node = (*node).children[match];
         c = stream[++i];
      }
      else {
         break;
      }
   }
   return last_match;
}


int bisect(string stream, struct keyval kv) {

   int down = 0;
   int up = kv.nkeys-1;
   int tested = (up + down) / 2;

   int match = -1;

   while (up > down) {
      switch (keycomp(kv.keys[(up+down)/2], stream)) {
         case  0:
            /* Found a match. */
            match = (up + down) / 2;
            /* No break: we go on. */
         case -1:
           /* 
            * The key is too small, we can skip this
            * position already.
            */
            down = (up + down) / 2 + 1;
            break;
         case  1:
            /* The key is too large, we can also skip. */
            up = (up + down) / 2 - 1;
            break;
      }
   }
  /*
   * Now up and down are either equal, or up < down.
   * In the first case we need to check one last key,
   * in the second, we're already done.
   */
   if ((up == down) && (keycomp(kv.keys[up], stream) == 0)) {
       match = up;
   }

   return match;

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

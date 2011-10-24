#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynstring.h"

#define BUFFER_SIZE 65536


#define USAGE "\n\
whiplace: multiple stream replacement\n\
\n\
USAGE:\n\
   whiplace keyfile targetfile\n\
\n\
whiplace performs 'stream' replacement, ie it replaces\n\
the first ocurrence of any key by a specified value.\n\
\n\
keyfile must consist of one key-value pair per line,\n\
separated by a single tab. Every occurrence of a key in\n\
targetfile will be replaced by the corresponding value.\n\
Specifying no value leads to deletion of the key, and\n\
specifying no key is ignored.\n\
\n\
A key is masked when another key prevents any match with\n\
it. For example 'abcd' is masked by 'abc', but not by\n\
'bcd' because 'abcd' can occur in the stream before\n\
'bcd'.\n\
You can check whether your keyfile contains masked keys\n\
by using\n\
\n\
   whiplace --key-check keyfile\n\
\n\
which will output a list of masked key - masking key,\n\
with one pair per line.\n\
\n\
Note that key collision like 'abcd' and 'bcd' are not\n\
checked for and can give unexpected results if overlooked.\n\n"



typedef struct {
   string *keys;
   string *values;
   int nkeys;
} keyval;


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
/* Split key-value pairs on tab. */

   int i, j;
   for (i = 0 ; keys[i] != NULL ; i++) {
      for (j = 0 ; keys[i][j] != '\t' ; j++) {
         if (keys[i][j] == '\0') {
            /* Line i misses tab (ill-formated). */
            fprintf(
               stderr,
               "missing tab in keyfile line %d (%s)\n",
               i+1, keys[i]
            );
            fprintf(stderr, USAGE);
            exit(EXIT_FAILURE);
         }
      }
      keys[i][j] = '\0';
      values[i] = keys[i] + j+1;
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
         return 0;
      }
   }

   return key[i] < stream[i] ? -1 : 1;

}


keyval get_key_values (const string fname) {
/* Read in key-value pairs from file, one pair per line. */

   FILE *keyfile = fopen(fname, "r");

   if (!keyfile) {
      fprintf(stderr, "cannot open file %s\n", fname);
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   const int nkeys = count_keys(keyfile);
   string *keys = (string *) malloc((nkeys+1) * sizeof(string));
   string *values = (string *) malloc((nkeys+1) * sizeof(string));

   char line[BUFFER_SIZE];
   int j, i = 0;

   while (fgets(line, sizeof(line), keyfile) != NULL) {

      int llen = strlen(line);
      if (llen >= BUFFER_SIZE) {
         /* Too unlikely to fix (for now). */
         fprintf(stderr, "%d characters! Is this a joke?\n", BUFFER_SIZE);
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


   keyval kv = { 
      .keys = keys, 
      .values = values,
      .nkeys = nkeys
   };

   return kv;

}



int keycheck(string *keys) {
/* 
 * Check that no key is masked by another. Return -1
 * if OK, the index of the masked key otherwise.
 */

   int i;
   for (i = 0 ; keys[i+1] != NULL ; i++) {
      if (keycomp(keys[i], keys[i+1]) == 0) {
         return i;
      }
   }

   return -1;

}

int replace(int index, keyval kv) {
/*
 * Print the replacement value and return the length
 * of the key (for skipping it from the stream).
 */

   printf("%s", kv.values[index]);
   return strlen(kv.keys[index]);

}


int bisect(string stream, keyval kv) {

   int down = 0;
   int up = kv.nkeys-1;

   while (up > down) {
      switch (keycomp(kv.keys[(up+down)/2], stream)) {
         case -1:
            down = (up + down) / 2 + 1;
           /*
            * We just tested the key at position (up + down) /2
            * and know it is too small. So we know the smallest
            * possible key is one rank higher.
            */
            break;
         case  1:
            up = (up + down) / 2 - 1;
            /* Same rationale as above. */
            break;
         case  0:
            /* Found a match. */
            return replace((up + down)/2, kv);
      }
   }
  /*
   * Now up and down are either equal, or up < down.
   * In the first case we need to check one last key,
   * in the second, we're already done.
   */
   if ((up == down) && (keycomp(kv.keys[up], stream) == 0)) {
       return replace(up, kv);
   }

   /* Still here? Then no match was found. */
   printf("%c", stream[0]);
   return 1;

}

void whiplace(FILE *streamf, keyval kv) {

   char buffer[BUFFER_SIZE];

   /* Initial read. */
   int j = fread(buffer, 1, BUFFER_SIZE, streamf);

   if (j < BUFFER_SIZE && feof(streamf)) {
     /* 
      * Hit the end of the file before the fun started.
      * Write EOF in 'buffer' to stop the for loop.
      */
      buffer[j] = EOF;
   }

   int i = 0;
   
   while (buffer[i] != EOF) {

      /* Most of the action happens in the following line. */
      i += bisect(buffer + i, kv); 

      /* Refill buffer when more than half way through it. */
      if (i > BUFFER_SIZE / 2) {

         /* Shift buffer */
         for (j = i ; j < BUFFER_SIZE ; j++) {
            buffer[j-i] = buffer[j];
         }

         /* Fill in buffer */
         j = fread(buffer + j - i, 1, i, streamf);
         if (j < i && feof(streamf)) {
           /*
            * Oops, hit the end of file: write EOF
            * in 'buffer' to stop the for loop.
            */
            buffer[i+j-2] = EOF;
         }

         /* Buffer is full and we're back to square 1. */
         i = 0;

      }

   } /* Hit EOF in buffer... We're done. */

}


int main (int argc, string argv[]) {

   int i, j;
   char checkkeys = 0;
   string keyfile = NULL;
   FILE *streamf = stdin;

   /* Options and arguments processing. */
   for (i = 1 ; i < argc ; i++) {
      if (strcmp(argv[i], "--key-check") == 0) {
         checkkeys = 1;
      }
      else if (keyfile == NULL) {
         keyfile = argv[i];
      }
      else {
         streamf = fopen(argv[i], "r");
         if (streamf == NULL) {
            fprintf(stderr, "cannot open file %s\n", argv[i]);
            fprintf(stderr, USAGE);
            exit(EXIT_FAILURE);
         }
      }
   }

   if (keyfile == NULL) {
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   keyval kv = get_key_values(keyfile);

   if (checkkeys) {
      /* Key check requested. */
      i = 0;
      while ((j = keycheck(kv.keys + i)) > -1) {
         printf("%s\t%s\n", kv.keys[i+j], kv.keys[i+j+1]);
         i += j+1;
      }
      exit(EXIT_SUCCESS);
   }
   else {
      /* Key masking. */
      i = 0;
      while ((j = keycheck(kv.keys + i)) > -1) {
         kv.keys[i+j+1] = kv.keys[i+j];
         kv.values[i+j+1] = kv.values[i+j];
         i += j+1;
      }
   }

   whiplace(streamf, kv);

   exit(EXIT_SUCCESS);
}

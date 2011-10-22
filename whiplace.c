#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynstring.h"

#define BUFFER_SIZE 65536
#define MIN_LENGTH 16


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
/* Split key-value pairs on the '~' character. */

   int i, j;
   for (i = 0 ; keys[i] != NULL ; i++) {
      for (j = 0 ; keys[i][j] != '~' ; j++) {
         if (keys[i][j] == '\0') {
            /* Line i misses '~' (ill-formated). */
            fprintf(
               stderr,
               "missing '~' in key-value file line %d (%s)\n",
               i, keys[i]
            );
            exit(EXIT_FAILURE);
         }
      }
      keys[i][j] = '\0';
      values[i] = keys[i] + j+1;
   }

}


int keycomp (string key, string stream) {
/*  Compare a key to a stream.Return -1, 0 or 1  */
/*  with 0 if akey match is found in the stream. */

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
      exit(EXIT_FAILURE);
   }

   const int nkeys = count_keys(keyfile);
   string *keys = (string *) malloc((nkeys+1) * sizeof(string));
   string *values = (string *) malloc((nkeys+1) * sizeof(string));

   char line[BUFFER_SIZE];
   int j, i = 0;

   while (fgets(line, sizeof(line), keyfile) != NULL) {
      int size = MIN_LENGTH;
      char *curPtr = (string) malloc(size * sizeof(char));
      if (curPtr == NULL) {
         fprintf(stderr, "memory error\n");
         exit(EXIT_FAILURE);
      }
      j = 0;
      while ((curPtr[j] = line[j++]) != '\n') {
         /* Get characters until we need more space. */
         if (j > size - 1) {
            /* Double string upon need. */
            curPtr = (string) str_extend(curPtr, &size);
         }
      }

      /* Remove the newline '\n' character if present. */
      curPtr[j-1] = '\0';

      keys[i++] = curPtr;
   }

   /* Sentinels */
   keys[i] = NULL;
   values[i] = NULL;

   fclose(keyfile);


   split(keys, values);
   /* Sort the key-values strings. */
   strsort(keys, nkeys);

   if ((i = keycheck(keys)) > 0) {
      fprintf(stderr, "key '%s' masked by key '%s'\n",
            keys[i-1], keys[i]);
      exit(EXIT_FAILURE);
   }

   keyval kv = { 
      .keys = keys, 
      .values = values,
      .nkeys = nkeys
   };

   return kv;

}



int keycheck(string *keys) {
/* 
 * Check that no key is masked by another. Return 0
 * if OK, the index of the masked key otherwise.
 */

   int i;
   for (i = 0 ; keys[i+1] != NULL ; i++) {
      if (keycomp(keys[i], keys[i+1]) != 0) {
         return i;
      }
   }

   return 0;

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

   string keyfile = argv[1];
   string targetfile = argv[2];

   FILE *streamf = fopen(targetfile, "r");
   if (streamf == NULL) {
      fprintf(stderr, "cannot open file %s\n", targetfile);
      exit(EXIT_FAILURE);
   }

   keyval kv = get_key_values(keyfile);
   whiplace(streamf, kv);

   exit(EXIT_SUCCESS);
}

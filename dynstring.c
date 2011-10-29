#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynstring.h"


int count_lines(FILE *keyfile) {
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

  /* Reset, just in case. */ 
   fseek(keyfile, 0, SEEK_SET);

  /* Count lines and reset. */
   while(fgets(buffer, sizeof(buffer), keyfile) != NULL) lc++;
   fseek(keyfile, 0, SEEK_SET);

   return lc;

}

void split(const string *keys, string *values, const char c) {
/* 
 * Split key-value pairs on char c. Keys and values are
 * read and sorted together as a single line for speed
 * and memory efficiency. They are separated on the
 * first char c by replacing it with '\0' and the value
 * pointer is assigned to the next character.
 */

   int i, j, no_c;

   /* Run through the keys. */
   for (i = 0 ; keys[i] != NULL ; i++) {
      no_c = 1;
      for (j = 0 ; j < strlen(keys[i]) ; j++) {
         if (keys[i][j] == c) {
           /* Found char c. */
            keys[i][j] = '\0';
            values[i] = keys[i] + j+1;
            no_c = 0;
            break;
         }
      }
      if (no_c) {
         /* Ooops... */
         fprintf(stderr, "no '%c' character in line %d (%s)\n",
               c, i+1, keys[i]);
         exit(EXIT_FAILURE);
      }     
   }
}


int strings_comp(const void *a, const void *b) { 
/* Comparison function for arrays of strings. */

   const char **ia = (const char **)a;
   const char **ib = (const char **)b;

   return strcmp(*ia, *ib);

}


void strsort (string *str_array, const int array_size) {
/* Wrapper for sorting arrays of strings in place. */

   qsort(str_array, array_size, sizeof(char *), strings_comp);

}


string shift (FILE *streamf, const int i) {
/* Stream the content of streamf. */

   static char buffer[BUFFER_SIZE];
   static int pos = BUFFER_SIZE;

   if (pos > BUFFER_SIZE / 2 && !feof(streamf)) {

      /* Shift buffer. */
      strncpy(buffer, buffer + pos, BUFFER_SIZE - pos);

      /* Fill in buffer */
      int j = fread(buffer + BUFFER_SIZE - pos, 1, pos, streamf);

      if (feof(streamf)) {
         buffer[BUFFER_SIZE-pos+j] = '\0';
      }

      /* Buffer is full and we're back to square 1. */
      pos = 0;
   }

   /* Return the current buffer. */
   pos += i;
   return buffer + pos;

}

string readline(FILE *file, const int chomp) {

   char line[BUFFER_SIZE];
   const string s = fgets(line, sizeof(line), file);

   const int llen = strlen(line);

   if (llen >= BUFFER_SIZE / 2 - 1) {
     /* Too unlikely to fix (for now). */
      fprintf(stderr, "line too long: %s", line);
      exit(EXIT_FAILURE);
   }

   if (chomp) {
      if (line[llen - 1] == '\n') {
         line[llen - 1] = '\0';
      }
   }

   return s; // NULL if EOF.

}

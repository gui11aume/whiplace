#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynstring.h"


int strings_comp(const void *a, const void *b) { 
/* Comparison function for arrays of strings. */

   const char **ia = (const char **)a;
   const char **ib = (const char **)b;

   return strcmp(*ia, *ib);

}


void strsort (string *str_array, int array_size) {
/* Wrapper for sorting arrays of strings in place. */

   qsort(str_array, array_size, sizeof(char *), strings_comp);

}


string shift (FILE *streamf, int i) {
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

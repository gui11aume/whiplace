#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dynstring.h"

#define BUFFER_SIZE 65536
#define MIN_LENGTH 16


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

   /* Reset, count lines and reset. */
   fseek(keyfile, 0, SEEK_SET);
   while(fgets(buffer, sizeof(buffer), keyfile) != NULL) lc++;
   fseek(keyfile, 0, SEEK_SET);

   return lc;

}


void split(string* key_value_array, int nstrings) {
/* slkj;s */

   int i, j;

   for (i = 0; i < nstrings ; i++) {
      j = 0;
      while (key_value_array[i][j++] != ' ') {
         ;
      }
      key_value_array[i][j] = '\0';
   }

}

string *get_key_values (const string fname) {
/* Read in key-value pairs from file, one pair per line. */

   FILE *keyfile = fopen(fname, "r");

   if (!keyfile) {
      exit(EXIT_FAILURE);
   }

   int nkeys = count_keys(keyfile);
   string *key_value_array = (string *) malloc((nkeys+1) * sizeof(string));

   char line[BUFFER_SIZE];
   int j, i = 0;

   while (fgets(line, sizeof(line), keyfile) != NULL) {
      int size = MIN_LENGTH;
      char *curPtr = (string) malloc(size * sizeof(char));
      if (curPtr == NULL) exit(EXIT_FAILURE);
      j = 0;
      while ((curPtr[j] = line[j++]) != '\0') {
         /* Get characters until we need more space. */
         if (j > size - 1) {
            /* Double string upon need. */
            curPtr = (string) str_extend(curPtr, &size);
         }
      }
      curPtr[j] = '\0';
      key_value_array[i++] = curPtr;
   }

   key_value_array[i] = NULL;
   fclose(keyfile);


   /* Sort the key-values strings. */
   strsort(key_value_array, nkeys);

   /* Split key-values on first space */
   /* TODO: find another separator. */
   split(key_value_array, nkeys);

   return (key_value_array);

}


int main (void) {
   char **keyArray = get_key_values("input.txt");
   size_t sz = sizeof(keyArray);

   int i;
   for (i = 0 ; i < sz ; i++) {
      printf("%s", keyArray[i]);
   }

   char c[] = {'a', 'b', 'c', 'd', 'e', '\0'};
   printf("%s\n", c);
   c[3] = '\0';
   printf("%s\n", c);

   exit(EXIT_SUCCESS);
}

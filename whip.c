#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 65536
#define MIN_LENGTH 16


int keycount(FILE*);
int getkeys (FILE*, char**);
int string_cmp(void*, void*);
*char memclaim(*char, *int);


char *keyArray[] = { "acorn", "good", "hear", "person",
   "soft", "with", "zeta" };
char *valueArray[] = { "foo", "bad", "listen", "deer",
   "hard", "withou", "delta" };



int keycount(FILE *keyfile) {
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


char **getkeys (char *fname) {

   FILE *keyfile = fopen(fname, "r");

   if (!keyfile) {
      exit(EXIT_FAILURE);
   }

   int nkeys = keycount(keyfile);
   char **keyArray = (char **) malloc(nkeys * sizeof(char *));

   char line[BUFFER_SIZE];
   int j, i = 0;

   while (fgets(line, sizeof(line), keyfile) != NULL) {
      int size = MIN_LENGTH;
      char *curPtr = (char *) malloc(size * sizeof(char));
      if (curPtr == NULL) exit(EXIT_FAILURE);
      j = 0;
      while ((curPtr[j] = line[j++]) != '\0') {
         /* Get characters until we need more space */
         if (j > size - 1) {
            /* Double string upon need. */
            curPtr = memclaim(curPtr, &size);
         }
      }
      curPtr[j] = '\0';
      keyArray[i++] = curPtr;
   }

   fclose(keyfile);

   return (keyArray);

   /* Sort the keys. */
   qsort(keyArray, nkeys, sizeof(char *), string_comp);

}

int keycheck(char **keyArray, int array_length) {
/* Check that no key is masked by another. */
   int i;
   for (i = 0 ; i < array_length ; i++) {
      if (comp(keyArray[i], keyArray[i+1]) != 0) {
         return i;
      }
   }
   return 0;
}


int keycomp (char *key, char *stream) {
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


int replace(int index) {
/* Print the replacement value and return the length */
/* of the key (for skipping it from the stream).     */

   printf("%s", valueArray[index]);
   return strlen(keyArray[index]);  // <<<<<<<<<<<< NEEDED ????

}


int bisect(char *stream) {

   int down = 0;
   int up = 6;    // <<<<<<<<<<<< THIS HAS TO BE DYNAMIC.

   while (up > down) {
      switch (comp(keyArray[(up+down)/2], stream)) {
         case -1:
            down = (up + down) / 2 + 1;
            /* We just tested the key at position (up + down) /2 */
            /* and know it is too small. So we know the smallest */
            /* possible key is one rank higher. */
            break;
         case  1:
            up = (up + down) / 2 - 1;
            /* Same rationale as above. */
            break;
         case  0:
            return replace((up + down)/2);
      }
   }
   /* Now up and down are either equal, or up < down.  */
   /* In the first case we need to check one last key, */
   /* in the second, we're already done. */
   if ((up == down) && (comp(keyArray[up], stream) == 0)) {
       return replace(up);
   }

   /* Still here? Then no match was found. */
   printf("%c", stream[0]);
   return 1;

}

int main(void) {

   char buffer[BUFFER_SIZE];
   FILE *f = fopen("input.txt", "r");

   if (!f) {
      /* Can't read. */
      exit(EXIT_FAILURE);
   }

   int i = keycheck(keyArray, 6); // <<<<<<<<<<<<<<<< DYNAMIC!!!
   if (i > 0) {
      exit(EXIT_FAILURE);
   }

   /* Initial read. */
   int j = fread(buffer, 1, BUFFER_SIZE, f);

   if (j < BUFFER_SIZE && feof(f)) {
      /* Hit the end of the file before the fun started. */
      /* Write EOF in 'buffer' to stop while loop. */
      buffer[j] = EOF;
   }

   i = 0;
   
   while (buffer[i] != EOF) {

      /* Increase 'i' while bisecting. */
      i += bisect(buffer + i);
      
      /* Refill buffer when more than half way. */
      if (i > BUFFER_SIZE / 2) {

         /* Shift buffer */
         for (j = i ; j < BUFFER_SIZE ; j++) {
            buffer[j-i] = buffer[j];
         }

         /* Fill in buffer */
         j = fread(buffer + j - i, 1, i, f);
         if (j < i && feof(f)) {
            /* Oops, hit the end of file */
            /* Write EOF in 'buffer' to stop while loop. */
            buffer[i+j-2] = EOF;
         }

         /* Buffer is full and we're back to square 1. */
         i = 0;

      }

   }

   return 0;

}

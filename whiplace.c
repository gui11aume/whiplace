#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096

char *keyArray[] = { "acorn", "good", "hear", "person", "soft", "with", "zeta" };
char *valueArray[] = { "foo", "bad", "listen", "deer", "hard", "withou", "delta" };


int keycheck(char **keyArray, int array_length) {
/* Check that no key is masked by another. */
   int i;
   for (i = 0 ; i < array_length ; i++) {
      if (comp(keyArray[i], keyArray[i+1]) != 0) {
         return (i);
      }
   }
   return (0);
}


int comp (char *key, char *stream) {
/*  Compare a key to a stream.Return -1, 0 or 1  */
/*  with 0 if akey match is found in the stream. */

   int i = 0;

   while (key[i] == stream[i]) {
      i++;
      if (key[i] == '\0') {
         return (0);
      }
   }

   return (key[i] < stream[i] ? -1 : 1);

}


int replace(int index) {
/* Print the replacement value and return the length */
/* of the key (for skipping it from the stream).     */

   printf("%s", valueArray[index]);
   return (strlen(keyArray[index]));

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
            return(replace((up + down)/2));
      }
   }
   /* Now up and down are either equal, or up < down.  */
   /* In the first case we need to check one last key, */
   /* in the second, we're already done. */
   if ((up == down) && (comp(keyArray[up], stream) == 0)) {
       return (replace(up));
   }

   /* Still here? Then no match was found. */
   printf("%c", stream[0]);
   return (1);

}

int main(void) {

   char buffer[BUFFER_SIZE];
   FILE *f = fopen("input.txt", "r");

   if (!f) {
      /* Can't read. */
      return (-1);
   }

   int i = keycheck(keyArray, 6); // <<<<<<<<<<<<<<<< DYNAMIC!!!
   if (i > 0) {
      return (-1);
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

   return (0);

}

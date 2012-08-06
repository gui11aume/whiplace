#include "array_lookup.h"

void exit_mem_fail(void) {
   fprintf(stderr, "memory error\n");
   exit(EXIT_FAILURE);
}

void exit_no_sep(char *s) {
   fprintf(stderr, "character separator not found in line '%s'\n", s);
   exit(EXIT_FAILURE);
}

void exit_dup_key(char *s) {
   fprintf(stderr, "key '%s' is duplicated\n", s);
   exit(EXIT_FAILURE);
}

int cmp(const void *a, const void *b) {
   const char **ia = (const char **) a;
   const char **ib = (const char **) b;
   return strcmp(*ia, *ib);
}


array_lookup finalize_array_lookup(array_lookup lookup, const char c) {
// Sort items in alphabetical order and assign 'values' pointers.
// The first occurence of 'c' is replaced by '\0' and the pointer in
// 'values' is assigned to the next character. Pointers in 'keys' and
// 'values' at the same index point to different positions of the same
// string.

   // Sort items in place.
   qsort(lookup.keys, lookup.item_nb, sizeof(char *), cmp);

   int i, j;

   // Iterate over 'keys' array.
   for (i = 0 ; lookup.keys[i] != NULL ; i++) {
      for (j = 0 ; lookup.keys[i][j] != c ; j++)
         if (lookup.keys[i][j] == '\0') exit_no_sep(lookup.keys[i]);
      lookup.keys[i][j] = '\0';
      lookup.values[i] = lookup.keys[i] + j+1;
   }

   // Check key duplication.
   for (i = 0 ; i < lookup.item_nb-1 ; i++) {
      if (strcmp(lookup.keys[i], lookup.keys[i+1]) == 0)
         exit_dup_key(lookup.keys[i]);
   }

   return lookup;

}

array_lookup generate_array_lookup_from_file (FILE *f) {

   char iob[IO_BUFFER_SIZE];
   int item_nb = 0;

   // Count lines. Hope to hit '\n' on every call to 'fgets'.
   // If not, overestimate 'item_nb' and assign too much memory.
   // TODO: remove next line if can.
   // fseek(f, 0, SEEK_SET);
   rewind(f)
   while(fgets(iob, IO_BUFFER_SIZE, f) != NULL) item_nb++;

   // Allocate array memory (keep one slot for sentinel).
   char **keys = (char **) malloc((item_nb+1) * sizeof(char *));
   char **values = (char **) malloc((item_nb+1) * sizeof(char *));

   if ((keys == NULL) || (values == NULL)) exit_mem_fail();

   int len, i = 0;

   // Read file one more time and copy lines to 'keys' array.
   // TODO: remove next line if can.
   //fseek(f, 0, SEEK_SET);
   rewind(f)
   while (fgets(iob, IO_BUFFER_SIZE, f) != NULL) {

      len = strlen(iob);
      // Chomp (replace '\n' by '\0').
      if (iob[len-1] == '\n') iob[--len] = '\0';
      // Allocate data memory.
      char *temp = (char *) malloc((len+1) * sizeof(char));

      if (temp == NULL) exit_mem_fail();

      strcpy(temp, iob);
      keys[i++] = temp;

   }

   // Add sentinels.
   keys[i] = NULL;
   values[i] = NULL;

   array_lookup lookup = { 
      .item_nb  = item_nb,
      .keys    = keys, 
      .values  = values,
   };
   
   return finalize_array_lookup(lookup, '\t');

}

void dealloc_array_lookup(array_lookup *lookup) {

   int i;
   for (i = 0 ; i < lookup->item_nb + 1 ; i++) {
      free(lookup->keys[i]);
   }
   free(lookup->keys);
   free(lookup->values);

   lookup->item_nb = 0;
   lookup->keys = NULL;
   lookup->values = NULL;

}


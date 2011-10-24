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

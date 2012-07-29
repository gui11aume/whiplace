#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IO_BUFFER_SIZE 65536

typedef
struct
/*********************************************************************
  Lookup of 'char' arrays with the following attributes.              
    'item_nb': number of items in the lookup                          
    'keys'   : array of key strings                                   
    'values' : array of value strings                                 
                                                                      
  For a given index 'i', 'keys[i]' and values[i]' point to different  
  positions of the same 'char' array, separated by '\0'.              
                                                                      
*********************************************************************/
{
   int    item_nb;
   char   **keys;
   char   **values;
}
array_lookup;


array_lookup generate_array_lookup_from_file (FILE *);
void dealloc_array_lookup(array_lookup *);



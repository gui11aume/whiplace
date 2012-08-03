#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "array_lookup.h"

typedef
struct rt_node
/************************************************************************
  Radix trie node with the following attributes.                         
     'subkey' : substring on a key path.                                 
     'data'   : char pointer on data for tail node, 'NULL' otherwise     
     'childen': array of pointers to child nodes.                        
                                                                         
  The 'children' array is terminated by a 'NULL' slot. A node with a     
  single 'NULL' child node slot is a leaf.                               
                                                                         
  A tail node is the last node in a path that matches a  key. Non tail   
  nodes have a 'data' pointer set to 'NULL'. Tail nodes do not need to   
  be leaves because they can have child nodes in case a key prefixes     
  another key.                                                           
                                                                         
************************************************************************/
{
              char   subkey[256];
              char   *data;
    struct rt_node   **children;
}
rt_node;


typedef
struct
{
              char   **keys;
              char   **values;
               int   lower;
               int   upper;
               int   depth;
}
key_range;


rt_node *create_orphan_node() {

   rt_node *orphan = (rt_node *) malloc(sizeof(rt_node));
   rt_node **no_child = (rt_node **) malloc(sizeof(rt_node *));
   *no_child = NULL;

   memset(orphan->subkey, '\0', 256);
   orphan->data = NULL;
   orphan->children = no_child;

   return orphan;

}


int add_node(rt_node *parent, rt_node *child) {
// Return 1 upon success, 0 upon failure.

   // Set i to current children count.
   int i; for (i = 0 ; parent->children[i] != NULL ; i++);

   // Allocate new 'children' array and set the sentinel.
   rt_node **children = (rt_node **) malloc((i+2) * sizeof(rt_node *));
   if (children == NULL) return 0;

   children[i] = child;
   children[i+1] = NULL;
   while (--i >= 0) children[i] = parent->children[i];

   free(parent->children);
   parent->children = children;

   return 1;

}

int rt_match(char *key, rt_node *root, char **data) {
// Match 'key' string in radix trie.
// PARAMETERS:
//    'key' : Key to be matched down the radix trie.
//    'root': Node to start matching from.
//    'data': Void pointer updated to data of matched node.
// RETURN:
//    Number of characters matched.
// SIDE EFFECTS:
//    Update char pointer 'data' in place with data from matched node.

   rt_node *parent = root, *child = NULL;
   int i, j, match_length = 0;

   for (i = 0 ; key[i] != '\0' ; i++) {
      // Look for character in children.
      for (j = 0 ; (child = parent->children[j]) != NULL ; j++) {
         if (child->subkey == key[i]) {
            // Found it. Save match if tail node.
            if (child->data != NULL) {
               *data = child->data;
               match_length = i+1;
            }
            parent = child;
            break;
         }
      }
      if (child == NULL) break; 
   }

   return match_length;

}

void build_rt(key_range range, rt_node *node) {
// Each node comprises key indices between 'lower' and 'upper'.
// If 'lower' and 'upper' are equal, the node comprises only one
// key and is a leaf.


   // Get first character in key range.
   char c = range.keys[range.lower][range.depth];

   // Check if 'node' is a tail (there can be only one '\0').
   if (c == '\0') {
      // Copy corresponding value.
      int nchar = strlen(range.values[range.lower]);
      node->data = (char *) malloc((nchar+1) * sizeof(char));
      strcpy(node->data, range.values[range.lower]);
      // Skip key and return if it is a leaf.
      range.lower++;
      if (range.lower == range.upper) return;
      else c = range.keys[range.lower][range.depth];
   }

   key_range child_range = {
      .keys   = range.keys,
      .values = range.values,
      .lower  = range.lower,
      .upper  = range.lower,
      .depth  = range.depth + 1,
   };

   void recursion(void) {
      rt_node *child = create_orphan_node();
      child->subkey = c;
      add_node(node, child);
      build_rt(child_range, child);
   }

   int i;

   for (i = range.lower ; i < range.upper ; i++) {
      if (c == range.keys[i][range.depth]) child_range.upper++;
      else {
         // Found a new character. Recursion and continue.
         recursion();
      
         c = range.keys[i][range.depth];
         child_range.lower = child_range.upper = i;

      }
   }

   recursion();

   return;

}

int main(void) {
   FILE *f = fopen("testfile.txt", "r");
   array_lookup lookup = generate_array_lookup_from_file(f);
   rt_node *root = create_orphan_node();

   key_range range = {
      .keys   = lookup.keys,
      .values = lookup.values,
      .lower  = 0,
      .upper  = lookup.item_nb,
      .depth  = 0,
   };

   build_rt(range, root);
   dealloc_array_lookup(&lookup);

   char *data = NULL;
   char key[56] = "ACD";
   int m = rt_match(key, root, &data);
   printf("%s->%s\n", key, data);

}

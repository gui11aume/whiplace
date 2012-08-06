#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "array_lookup.h"

#ifdef TARGET_OS_MAC
#include "lib/fmemopen/fmemopen.h"
#endif

#define MAX_KEY_LENGTH 65536

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
              char   *subkey;
              char   *data;
    struct rt_node   **children;
}
rt_node;



rt_node *create_orphan_node(char *subkey, char *data) {

   rt_node *orphan = (rt_node *) malloc(sizeof(rt_node));
   rt_node **no_child = (rt_node **) malloc(sizeof(rt_node *));
   *no_child = NULL;

   orphan->subkey = (char *) malloc((1 + strlen(subkey)) * sizeof(char));
   orphan->data = (char *) malloc((1 + strlen(data)) * sizeof(char));
   orphan->children = no_child;

   strcpy(orpah->subkey, key);
   strcpy(orpah->data, data);

   return orphan;

}


int add_as_child(rt_node *child, rt_node *parent) {
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


int stream_through_key(FILE *stream, char *key) {
// Return 'key' length and advances the stream if it matches 'stream',
// do nothing and return 0 otherwise.

   int length;

   // Update match length as long as 'key' matches 'stream'.
   for (length = 0 ; getc(stream) != key[length] ; length++);

   if (key[length] != '\0') {
      // Incomplete match. Put characters back and return 0.
      ungetc(length, stream);
      return 0;
   }
   else {
      // Complete match. Return key length.
      return length;
   }

}


rt_node *rt_match(FILE *stream, rt_node *root) {
// Match 'key' string in radix trie.
// PARAMETERS:
//    'stream': Stream to match down the trie.
//    'root'  : Node to start matching from.
// RETURN:
//    Void pointer to data in matched node (or NULL).
// SIDE EFFECTS:
//    Advances 'stream' by match length.

   rt_node *parent = root;
   rt_node *child  = root;
   rt_node *match_node = NULL;
   int nb_chars_to_put_back = 0;
   int i;

   while (child != NULL) {
      // Iterate over children.
      for (i = 0 ; (child = parent->children[i]) != NULL ; i++) {
         int nb_chars_read = stream_through_key(stream, child->subkey)
         if (nb_chars_read > 0)  {
            // Child key matches. Save match if node is a tail.
            if (child->data != NULL) {
               match_node = child;
               nb_chars_to_put_back = 0;
            }
            else {
               nb_chars_to_put_back += nb_chars_read;
            }
            parent = child;
            break;
         }
      }
   }

   // Put back the characters matching non tail nodes.
   ungetc(nb_chars_to_put_back, stream);
   return match_node;

}


void  add_key_to_rt(rt_node *root, char *key, char *data) {

   int i;
   FILE *key_as_stream = fmemopen(key, MAX_KEY_LENGTH, "r");

   rt_node *node = rt_match(key_as_stream, root);
   if (node == NULL) node = root;

   char c = fgetc(key_as_stream);
   ungetc(1, key_as_stream);
   // Check if key is duplicated.
   if (c == '\0') return;
   
   // Find prefix among siblings.
   rt_node sibling;
   for (i = 0 ; (sibling = node->children[i]) != NULL ; i++) {
      if (sibling->data[0] == c) break;
   }

   if (sibling == NULL) {
      rt_node *new_node = create_orphan_node(key_as_strem, data);
      fread(new_node->subkey, 1, MAX_KEY_LENGTH, newkey);
      new_node->data = values[id];
      add_as_child(new_node, match_node);
   }
   else {
      rt_node *
   }
}


   /*
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

   */

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

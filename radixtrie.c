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
   strcpy(orphan->subkey, subkey);

   if (data != NULL) {
      orphan->data =(char *) malloc((1 + strlen(data)) * sizeof(char));
      strcpy(orphan->data, data);
   }
   else {
      orphan->data = NULL;
   }
          
   orphan->children = no_child;

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
   fpos_t init_pos;
   fgetpos(stream, &init_pos);
   fpos_t match_pos = init_pos;

   // Update match length as long as 'key' matches 'stream'.
   for (length = 0 ; fgetc(stream) == key[length] ; length++) {
      fgetpos(stream, &match_pos);
   }

   if (key[length] != '\0') {
      // Incomplete match. Put characters back and return 0.
      fsetpos(stream, &init_pos);
      return 0;
   }
   else {
      // Complete match. Return key length.
      fsetpos(stream, &match_pos);
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
   int nb_chars_in_excess = 0;
   int nb_chars_read;
   int i;

   while (child != NULL) {
      // Iterate over children.
      for (i = 0 ; (child = parent->children[i]) != NULL ; i++) {
         nb_chars_read = stream_through_key(stream, child->subkey);
         if (nb_chars_read > 0)  {
            // Child key matches. Save match if node is a tail.
            if (child->data != NULL) {
               match_node = child;
               nb_chars_in_excess = 0;
            }
            else {
               nb_chars_in_excess += nb_chars_read;
            }
            parent = child;
            break;
         }
      }
   }

   // Put back the characters matching non tail nodes.
   fseek(stream, - nb_chars_in_excess, SEEK_CUR);
   return match_node;

}


void  add_key(rt_node *root, char *suff, char *data) {

   // Find node where to add key suffix.
   FILE *keystream = fmemopen(suff, MAX_KEY_LENGTH, "r");
   rt_node *node = rt_match(keystream, root);
   if (node == NULL) node = root;
   suff += ftell(keystream);
   fclose(keystream);

   int i;
   int j;
   char *subkey;
   char pref[MAX_KEY_LENGTH];
   rt_node *child;

   // If 'suff' shares prefix with a child, add internal node.
   for (i = 0 ; (child = node->children[i]) != NULL ; i++) {

      subkey = child->subkey;
      for (j = 0 ; suff[j] != '\0' &&  suff[j] != subkey[j]; j++) {
         pref[j] = suff[j];
      }
      pref[j] = '\0';

      if (j > 0) {
         child->subkey += j;
         suff += j;
         rt_node *internal_node = create_orphan_node(pref, NULL);
         add_as_child(child, internal_node);
         node->children[i] = internal_node;
         node = internal_node;
         break;
      }
   }

   rt_node *leaf = create_orphan_node(suff, data);
   add_as_child(leaf, node);
   
}


int main(void) {
   FILE *f = fopen("testfile.txt", "r");
   array_lookup lookup = generate_array_lookup_from_file(f);

   rt_node *root = create_orphan_node("", NULL);
   int i;
   for (i = 0 ; i < lookup.item_nb ; i++) {
      add_key(root, lookup.keys[i], lookup.values[i]);
   }

   dealloc_array_lookup(&lookup);

   FILE *stream = fmemopen("A", 256, "r");
   rt_node *match = rt_match(stream, root);
   printf("%s\n", match->data);

}

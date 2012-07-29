#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
whiplace: multiple stream replacement
USAGE:
  whiplace keyfile targetfile
*/

void exit_memory_failure(void) {
   fprintf(stderr, "memory error\n");
   exit(EXIT_FAILURE);
}

void exit_no_sep_failure(char *s) {
   fprintf(stderr, "character separator not found in line %s\n", s);
   exit(EXIT_FAILURE);
}

int cmp_wrapper(const void *a, const void *b) {
   const char **ia = (const char **) a;
   const char **ib = (const char **) b;
   return strcmp(*ia, *ib);
}


void symsplit(char **keys, char **values, const char c) {
// Symbolic split of key-value pairs on char 'c'. Keys and values are
// The first occurence of 'c' is replaced by '\0' and the pointer in
// 'values' is assigned to the next character. Pointers in 'keys' and
// 'values' at the same index point to different positions of the same
// string.

   int i, j;

   // Iterate over 'keys' array.
   for (i = 0 ; keys[i] != NULL ; i++) {
      for (j = 0 ; keys[i][j] != c ; j++)
         if (keys[i][j] == '\0') exit_no_sep_failure(keys[i]);
      keys[i][j] = '\0';
      values[i] = keys[i] + j+1;
   }

}

struct keyval get_key_values (FILE *f) {
// Read in key-value pairs from file, one pair per line.

   char iob[IO_BUFFER_SIZE];
   int nitems = 0;

   // Count lines. Hope to hit '\n' on every call to 'fgets'.
   // If not, overestimate 'nitems' and assign too much memory.
   fseek(f, 0, SEEK_SET);
   while(fgets(iob, IO_BUFFER_SIZE, f) != NULL) nitems++;

   char **keys = (char **) malloc((nitems+1) * sizeof(char *));
   char **values = (char **) malloc((nitems+1) * sizeof(char *));

   if ((keys == NULL) || (values == NULL)) exit_memory_failure();

   int len, i = 0;

   // Read file again and copy lines to 'keys'.
   fseek(f, 0, SEEK_SET);
   while (fgets(iob, IO_BUFFER_SIZE, f) != NULL) {

      len = strlen(iob);
      // Chomp (replace '\n' by '\0').
      if (iob[len-1] == '\n') iob[--len] = '\0';
      char *temp = (char *) malloc((len+1) * sizeof(char));

      if (temp == NULL) exit_memory_failure();

      strcpy(temp, iob);
      keys[i++] = temp;

   }

   // Add sentinels.
   keys[i] = NULL;
   values[i] = NULL;

   // Split the key/value on tab character and sort.
   symsplit(keys, values, '\t');
   qsort(keys, nitems, sizeof(char *), cmp_wrapper);

   struct keyval kv = { 
      .keys    = keys, 
      .values  = values,
      .nitems  = nitems
   };

   return kv;

}


struct trienode *create_node(void) {
   struct trienode *new_node = NULL; // Required to reclaim memory.
   new_node = malloc(sizeof(struct trienode));
   if (new == NULL) exit_memory_failure();
   new_node->key_index = -1;
   new_node->chars = NULL;
   new_node->children = NULL;

   return new;
}


void build_trie(struct trienode *thisnode, int down, const int up,
      const char **keys, const int depth) {
// 'thisnode': current node.
//       'up': upper limit of key index for descent of current node.
//     'down': lower limit of key index for descent of current node.
//     'keys': whiplace key set.
//    'depth': depth of current node in the trie.

   // Temp arrays.
   int min[256], max[256];
   // Allocate too much. Will realloc() later.
   thisnode->chars = (char *) malloc(256 * sizeof(char));

   // Specify key index if a key finishes here (and skip the key).
   if (keys[down][depth] == '\0') thisnode->key = down++;

   // Here function.
   void allocate(int k) {
      // Allocate memory for characters and children.
      thisnode->children = \
         (struct trienode **) calloc(k+1, sizeof(struct trienode *));
      if (thisnode->children == NULL) exit_memory_failure();
      // Add the sentinels.
      thisnode->chars[k] = '\0';
      thisnode->children[k] = NULL;
   }


  if (down > up) { 
  // This is a leaf node.
      allocate(0);
  }
  else {
  // This is a branch node.

      int i, j = 0;
      thisnode.chr = keys[down][depth];
      min[0] = max[0] = down;

      // Gather and count letters at given depth.
      for (i = down + 1 ; i < up + 1 ; i++) {
         if (thisnode.chr !=  keys[i][depth]) {
            // New character.
            j++;
            thisnode->chars[j] = keys[i][depth];
            min[j] = max[j] = i;
         }
         else {
           /* Increment total count for character. */
            ++max[j];
         }
      }

      // Allocate memory for (j+1) children.
      allocate(j+1);

      // Depth-first recursion.
      for (i = 0 ; i < j + 1 ; i++) {
         thisnode->children[i] = newnode();
         build_tree(thisnode->children[i], min[i], max[i],
            keys, depth + 1);
      }
   }
}



int find_char(const char c, const string sorted_keys) {
/*
 * Find char c in sorted string by bisection. Return
 * its index in sorted_keys if match, -1 otherwise.
 */

   int down = 0;
   int up = strlen(sorted_keys) - 1;
   int diff;

   while (up > down) {
      diff = sorted_keys[(up+down)/2] - c; 
      if (diff < 0) {
        /* The key is too small, we can it. */
         down = (up + down) / 2 + 1;
      }
      else if (diff > 0) {
        /* The key is too large, we can also skip. */
         up = (up + down) / 2 - 1;
      }
      else {
        /* Found the match. */
         return (up + down) / 2;
      }
   }
  /*
   * Now up and down are either equal, or up < down.
   * In the first case we need to check one last key,
   * in the second, we're already done.
   */
   if ((up == down) && (sorted_keys[up] == c)) {
       return up;
   }
   /* No match. */
   return -1;
}


int keycomp (string stream, string key) {
/* 
 * Compare a key to a stream. Return -1, if no match is
 * found in the stream, the length of the key otherwise.
 */

   int i = 0;

   while (key[i] == stream[i]) {
      i++;
      if (key[i] == '\0') {
        /* Matched until end of key. */
         return i;
      }
   }

  /* Did not match. */
   return -1;

}


int whip(const string stream, struct trienode *node) {
/*
 * Match the current position of the stream with
 * the key tree. Return -1 if no match is found,
 * and the key index otherwise.
 */

   char c = stream[0];
   int j, i = 0;
   int charmatch, keymatch = -1;
   do {
      if ((*node).key > -1) {
         keymatch = (*node).key;
      }
      /* Find char match. */
      if ((*node).branch) {
        /* Node is a branch: find where to go next */
         charmatch = find_char(c, (*node).chars);
         j = 1;
      }
      else {
        /* Node is a stem: check whether it matches stream. */
         j = keycomp(stream + i, (*node).chars);
         charmatch = 0;
      }
      if ((charmatch > -1) && (j > -1)) {
         node = (*node).children[charmatch];
         i += j; // 1 if branch, key length otherwise.
         c = stream[i];
      }
   }
  /*
   * Continue until no character matches. Leaf 
   * nodes can't match, which stop the loop.
   */
   while ((charmatch > -1) && (j > -1));

   return keymatch;

}



void whiplace (FILE *keyf, FILE *streamf, FILE *outf) {

   int i;

  /* Get and sort keys + values. */
   const struct keyval kv = get_key_values(keyf);

  /* Check key duplicates. */
   for (i = 0 ; i < kv.nitems -2; i++) {
      if (strcmp(kv.keys[i], kv.keys[i+1]) == 0) {
         fprintf(stderr, "key '%s' duplicated\n", kv.keys[i]);
         exit(EXIT_FAILURE);
      }
   }

  /* Build the key tree. */
   struct trienode * const root = newnode();
   build_tree(root, 0, kv.nitems-1, kv.keys, 0);

  /* Let it whip! */
   string buffer = (string) shift(streamf, 0);

   while (buffer[0] != '\0') {
      i = whip(buffer, root);
      if (i > -1) {
        /* Found a match. */
         fprintf(outf, "%s", kv.values[i]);
         buffer = (string) shift(streamf, strlen(kv.keys[i]));
      }
      else {
         fprintf(outf, "%c", buffer[0]);
         buffer = (string) shift(streamf, 1);
      }
   }

   return;

}

int main (int argc, string argv[]) {

   string USAGE = "\n"
"whiplace: multiple stream replacement\n\n"
"USAGE:\n"
"   whiplace keyfile targetfile\n\n";


  /* Options and arguments processing. */

   if ((argc < 2) || (argc > 4)) {
      fprintf(stderr, USAGE);
      exit(EXIT_FAILURE);
   }

   string keyfname = NULL;

   keyfname = argv[1];
   string fname = argc > 2 ? argv[2] : NULL;
   string outfname = argc > 3 ? argv[3] : NULL;

   FILE *keyf = fopen(keyfname, "r");
   FILE *streamf = (fname == NULL) ? stdin : fopen(fname, "r");
   FILE *outf = (outfname == NULL) ? stdout : fopen(outfname, "w");

   if (keyf == NULL) {
      fprintf(stderr, "cannot open key file %s\n", keyfname);
      exit(EXIT_FAILURE);
   }

   if (streamf == NULL) {
      fprintf(stderr, "cannot open stream %s\n", fname);
      exit(EXIT_FAILURE);
   }

   if (outf == NULL) {
      fprintf(stderr, "cannot open file %s for writing\n", outfname);
      exit(EXIT_FAILURE);
   }

  /* (End of option parsing). */

   whiplace(keyf, streamf, outf);

  /* Wrap up. */
   fflush(outf);
   fclose(keyf);
   fclose(streamf);
   fclose(outf);

   exit(EXIT_SUCCESS);

}

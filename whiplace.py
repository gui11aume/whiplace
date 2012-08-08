#!/usr/bin/en python
# -*- coding:utf-8 -*-

import sys
import gc

def commonprefix(string1, string2):
   """Return the common prefix between two strings, and the two
   corresponding suffixes."""
   num = 0
   for (a,b) in zip(string1, string2):
      if a == b: num += 1
      else: break
   return (string1[:num], string1[num:], string2[num:])

class KeyDuplicated(Exception):
   pass

class RadixTrieNode(object):
   """Nodes of a radix trie have 3 attributes:
       'subkey'  : the characters on a key path
       'data'    : replacement characters ('None' if node is not tail)
       'children': list of 'RadixTrieNode' children objects.

      A leaf node is a terminal node on the tree. A tail node
      corresponds to a complete key path. Every leaf is a tail, but
      not every tail is a leaf, because a key can prefix another."""

   def __init__(self, subkey='', data=None):
      """Radix trie nodes are instantiated with no child."""
      self.subkey = subkey
      self.data = data
      self.children = []

   def fork(node):
      """A method for readability. Use 'fork' to add a node that
      shares a prefix with current node to the trie."""
      (pref, suff_1, suff_2) = commonprefix(self.subkey, node.subkey)
      node.subkey = suff_2
      self.subkey = pref
      self.children = [RadixTrieNode(suff_1, self.data), node]
      self.data = None


class RadixTrie(object):
   """A radix trie is a collection of nodes linked by parent-child
   relationships. This class contais the methods to build and search
   the trie."""

   def __init__(self):
      """Radix tries are instantiated with an empty 'root' node."""
      self.root = RadixTrieNode()

   def key_path(self, key, node, tail=None):
      """Recursive search for key path. Return a 3-tuple with:
          'key' : the unmatched suffix
          'node': the deepest node matching initial 'key' prefix
          'tail': the deepest tail matching initial 'key' prefix"""
      for child in node.children:
         if not key.startswith(child.subkey): continue
         key = key[len(child.subkey):]
         if child.data is not None: tail = child
         return self.key_path(key, child, tail)
      return (key, node, tail)

   def add(self, key, data):
      """Add a key/data pair to the radix trie."""
      (suf, node, tail) = self.key_path(key, self.root)
      new_node = RadixTrieNode(suf, data)
      try:
         k = [child.subkey[0] for child in node.children].index(suf[0])
      except ValueError:
         node.children.append(new_node)
      else:
         node.children[k].fork(new_node)

   @classmethod
   def from_items(self, items):
      """Conveniently build a radix trie from key/data items."""
      # The list 'append' method has a bug in Python.
      # See http://bugs.python.org/issue4074
      # Disabling garbage collection while doing multiple 'append'
      # of complex objects can increase performance.
      gc_was_initially_enabled = gc.isenabled()
      try:
         gc.disable()
         radix_trie = RadixTrie()
         # Check that no key is duplicated.
         keys = [item[0] for item in items]
         if len(keys) != len(set(keys)):
            from collections import Counter
            key = Counter(keys).most_common(1).pop()[0]
            raise KeyDuplicated(key)
         # Not sorting can cause 'add' to crash for some input.
         for (key, data) in sorted(items):
            radix_trie.add(key, data)
      finally:
         if gc_was_initially_enabled: gc.enable()
      return radix_trie


if __name__ == '__main__':
   """Invoke as 'python whiplace.py keyfile streamfile'."""
   try:
      key_fname = sys.argv[1]
   except IndexError:
      sys.stderr.write("please provide key file\n")
      sys.exit(1)
   try:
      inputf = open(sys.argv[2])
   except IndexError:
      inputf = sys.stdin
   # Read key/data pairs from key file, one per line. This assumes
   # that '\n' is not a key character. This assumption will be reused
   # when reading the stream line by line.
   # The key/data pairs are split on the tab character '\t'.
   with open(key_fname) as f:
      items = [l.rstrip().split('\t') for l in f]
      try:
         trie = RadixTrie.from_items(items)
      except KeyDuplicated as key:
         sys.stderr.write("key %s is duplicated\n" % key)
         sys.exit(1)
      del(items)
   with inputf as f:
      for key in f:
         while key:
            (suffix, node, tail) = trie.key_path(key, trie.root)
            if tail is not None:
               sys.stdout.write(tail.data)
               key = suffix
            else:
               sys.stdout.write(key[0])
               key = key[1:]

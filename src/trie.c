#include "trie.h"
#include "printing.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIE_RADIX 26
#define ASCII_TO_IDX(c) c - 97
#define INT16_MAX 32767

typedef struct node node_t;
struct node {
  char *key;
  void *value;
  node_t *children[TRIE_RADIX];
};

typedef struct trie {
  node_t *root;
} trie_t;

static inline int isleaf(node_t *node) {
  // A NULL node is not considered a leaf node
  if (node == NULL) {
    return 0;
  }

  for (int i = 0; i < TRIE_RADIX; i++) {
    if (node->children[i] != NULL) {
      return 0;
    }
  }

  return 1;
}

static node_t *node_create(char *key, void *value) {
  node_t *node = (node_t *)calloc(1, sizeof(node_t));
  if (node == NULL) {
    goto error;
  }

  node->key = key;
  node->value = value;

  return node;

error:
  free(node);
  return NULL;
}

void node_destroy(node_t *node) { free(node); }

trie_t *trie_create() {

  trie_t *t = (trie_t *)calloc(1, sizeof(trie_t));

  if (t == NULL) {
    goto error;
  }

  t->root = node_create(NULL, NULL);
  return t;

error:
  return NULL;
}

void _trie_destroy(node_t *node) {
  if (isleaf(node)) {
    node_destroy(node);
  } else {
    int i;
    for (i = 0; i < TRIE_RADIX; i++) {
      if (node->children[i] != NULL) {
        _trie_destroy(node->children[i]);
        node->children[i] = NULL;
      }
    }
    node_destroy(node);
  }

  return;
}

void trie_destroy(trie_t *trie) {
  _trie_destroy(trie->root);
  free(trie);
  trie = NULL;
}

int trie_insert(trie_t *trie, char *key, void *value) {
  if (trie == NULL)
    return 0;

  node_t *iter = trie->root;

  /**@MODIFIED: */
  // len of char is needed to define the number of leftover chars when insert is
  // done.
  int len = strlen(key);

  // Only allow alphabet characters
  for (int i = 0; key[i] != '\0'; i++) {
    if (!isalpha(key[i])) {
      goto error;
    }
  }

  // Find the child indices
  for (int i = 0; key[i] != '\0'; i++) {
    int *nr = malloc(sizeof(int));
    *nr = len - i;
    // We only use lowercase letters (case-insensitive)
    if (iter->children[ASCII_TO_IDX(tolower(key[i]))] == NULL) {
      /**@MODIFIED: */
      // the parameter for node_create,
      // by defined the integer type that represent the number of leftover char.
      node_t *new = node_create(NULL, nr);
      if (new == NULL) {
        free(new);
        return -1;
      }
      iter->children[ASCII_TO_IDX(tolower(key[i]))] = new;
    }
    /**@MODIFIED: */
    // overwrites the node_t value to represent the number of leftover chars.
    iter->value = nr;
    iter = iter->children[ASCII_TO_IDX(tolower(key[i]))];
  }

  iter->key = key;
  iter->value = value;

  return 0;

error:
  return -1;
}

/**
 * @brief This function is used to find the index that contains the lowest
 * value.
 * @param[in] arr[] A array that contains the values that needs to be compared.
 * @return Returns the index that contains the lowest value.
 */
static int min_value(const int arr[]) {

  int minIndex = 0;
  for (int current = 0; current < TRIE_RADIX; ++current) {

    if (arr[minIndex] > arr[current]) {
      minIndex = current;
    }
  }
  return minIndex;
}

char *trie_find(trie_t *trie, char *key) {
  // If the length of @param key is less than 3, return NULL
  unsigned len_key = strlen(key);
  if (len_key < 3) {
    return NULL;
  }

  // Traverse the trie tree to find the node corresponding to the last character
  // from the @parameter: key.
  node_t *current = trie->root;
  for (int i = 0; key[i] != '\0'; ++i) {

    if (current->children[ASCII_TO_IDX(tolower(key[i]))] != NULL) {
      current = current->children[ASCII_TO_IDX(tolower(key[i]))];

    } else {
      return NULL;
    } // Return NULL if the node doesnt exist.
  }

  // Create an array to collect the child node's value.
  int valueCollector[TRIE_RADIX];
  for (int i = 0; i < TRIE_RADIX; ++i) {
    valueCollector[i] = INT16_MAX;
  }

  // Check all child on the current node.
  for (int i = 0; i < TRIE_RADIX; ++i) {

    if (current->children[i]) {

      // If the child hold the key, return it,
      if (current->children[i]->key != NULL) {
        return current->children[i]->key;
      }

      // Otherwise collect value of the child node.
      int *index = current->children[i]->value;
      valueCollector[i] = *index;
    }
  }

  // Find the index that's holds the smallest value in the collected array.
  int minValueIndex = min_value(valueCollector);

  /* If there is more characters below, use @minValueIndex as a path to find
   * other words. */
  if (current->value) {
    goto HaveMoreChars;
  } else if (current->key) {
    return current->key;
  }

HaveMoreChars:
  current = current->children[minValueIndex];
  int currentIndex = 0;

  // Traverse down to the end of the chosen path, to find the key.
  while (current->children[currentIndex] == NULL && currentIndex < 26) {

    /* If the next index have a defined key, its indicates as a null-terminal
     * word. */
    currentIndex++;
    if (current->children[currentIndex]) {
      if (current->children[currentIndex]->key) {
        return current->children[currentIndex]->key;
      }
      current = current->children[currentIndex];
      currentIndex = 0;
    }
  }

  // Return NULL, if no word is found.
  return NULL;
}

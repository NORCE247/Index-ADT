#include "common.h"
#include "index.h"
#include "map.h"
#include "printing.h"
#include "trie.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// Function prototypes
static search_result_t *multi_find(index_t *idx, list_t *tokens,
                                   const char *query);
static void toLowerCase(char *str);
static search_result_t *cmpSearchResult(search_result_t *main,
                                        search_result_t *sub, int subWordPos,
                                        int str_len, index_t *idx);

/**
 * @brief This structure represents an index entry for a document.
 * It contains information about the document along with related data structures
 * for indexing.
 *
 */
typedef struct index {
  char *documentName; ///< A string containing the name of the document
  char **stringArray; ///< Represent document contents.
  trie_t *trieTree;   ///< A trie structure used for autocompletion
  index_t *next;      ///< A pointer to the next index_t structure
  int size;           ///< The length of the stringArray
  map_t *map;         ///< Hashmap

} index_t;

/**
 * @brief This structure represent a iteration for the search results,
 * and holds the information that's describe the length and found word location.
 * These information is stored in search_hit_t and search_hit_t is stored in a
 * linked-list.
 *
 */
typedef struct search_result {
  list_iter_t *hitsArray; ///< A linked list containing search_hit_t
  index_t *index;         ///< A pointer to index_t
  search_result_t
      *next; ///< Points to the next structure, and works like a linked list
  int accessedCounter; ///< Represent the number of times a file have been
                       ///< visited in difference function, [0,1,2,3] = [create,
                       ///< get content,get length, get next result]

} search_result_t;

/**
 * @brief Creates a new search result structure.
 * @param[in] idx - index_t structure
 * @return  A pointer to the newly allocated search result structure
 */
static search_result_t *create_search_result_t(index_t *idx) {

  search_result_t *new = malloc(sizeof(search_result_t));
  if (new == NULL) {
    return NULL;
  }

  new->index = idx;
  new->hitsArray = NULL;
  new->accessedCounter = 0;
  return new;
}

index_t *index_create() {
  // Allocate
  index_t *index = malloc(sizeof(index_t));
  if (index == NULL)
    return NULL;

  // Initializes
  index->documentName = NULL;
  index->stringArray = NULL;
  index->size = 0;
  index->trieTree = NULL;
  index->next = NULL;

  return index;
}

void index_destroy(index_t *index) {
  if (index != NULL) {

    free(index->stringArray);
    trie_destroy(index->trieTree);
    index_destroy(index->next);
    free(index);

  } else {
    return;
  } // Avoid to deallocate a non-allocated memory block.
}

void index_add_document(index_t *idx, char *document_name, list_t *words) {

  /* Check if the current index_t is empty and can be use & store data */
  if (idx->documentName == NULL) {

    // Mark as non-empty
    idx->documentName = document_name;
    int len = list_size(words);

    // Allocate memory to store content, that can be use for search hits.
    idx->stringArray = malloc(sizeof(char *) * len + 1);
    idx->trieTree = trie_create();
    idx->map = map_create(compare_strings, djb2);

    // Insert words in the String array, Trie Tree, Hash Map.
    for (int i = 0; i < len; ++i) {

      // Pack out the string from linked list & insert into the array.
      char *word = list_popfirst(words);
      idx->stringArray[i] = word;
      idx->size++;

      // Create a null terminated string & Convert to lower case.
      char *nullTerminated = strdup(word);
      toLowerCase(nullTerminated);

      // Setup Autocomplete
      trie_insert(idx->trieTree, nullTerminated, NULL);

      // Create & store word location in search_hit_t, and store it in the
      // hashmap.
      search_hit_t *hit = malloc(sizeof(search_hit_t));
      hit->location = i;
      hit->len = 0;
      map_put(idx->map, (char *)nullTerminated, hit);
    }

  } else {

    // Find empty index_t structure.
    while (idx->next != NULL) {
      idx = idx->next;
    }

    // Create a new index_t & store the data by recursion.
    index_t *new = index_create();
    idx->next = new;
    index_add_document(idx->next, document_name, words);
  }
}

search_result_t *index_find(index_t *idx, const char *query) {
  // Return NULL if there is no document to access.
  if (idx == NULL || idx->documentName == NULL || query == NULL) {
    return NULL;
  }

  /* Get the number of string in query */
  list_t *tokens = list_create(NULL);
  parse_word((char *)query, tokens);

  /**@section MULTI STRING SEARCH **/
  if (list_size(tokens) > 1) {
    search_result_t *result = multi_find(idx, tokens, query);
    return result;
  }

  /**@section SINGLE STRING SEARCH **/
  search_result_t *searchResult = create_search_result_t(idx);
  bool found = false;

  /* Convert to lower case */
  char *word = strdup(query);
  toLowerCase(word);

  /* Check if there is any search hits */
  list_t *hits = map_get(idx->map, (char *)word);
  if (hits != NULL) {
    found = true;
    searchResult->hitsArray = list_createiter(hits);
  }

  if (found) {

    /* Store the current dokument contents, and other existing files contents */
    searchResult->index = idx;
    if (idx->next != NULL) {
      searchResult->next = index_find(idx->next, query);
    }

  } else {

    /* Search other file if no query is found */
    if (searchResult == NULL) {
      searchResult = index_find(idx->next, query);
    } else {
      searchResult->next = index_find(idx->next, query);
    }
  }

  return searchResult;
}

char *autocomplete(index_t *idx, char *input, size_t size) {

  // Define the suggestion word that are found in the Trie-Tree.
  char *suggestion = trie_find(idx->trieTree, input);

  /* If there is no suggestion word, check the next document. */
  if (suggestion == NULL && idx->next != NULL) {
    suggestion = autocomplete(idx->next, input, size);
  }

  return suggestion;
}

char **result_get_content(search_result_t *res) {
  if (res == NULL) {
    return NULL;
  }
  char **content = NULL;

  /* Check if the current file's content have been accessed. */
  if (res->accessedCounter == 0) {

    content = res->index->stringArray;
    res->accessedCounter = 1; // Mark as accessed

    // if its accessed, then check the next file.
  } else {
    content = result_get_content(res->next);
  }

  return content;
}

int result_get_content_length(search_result_t *res) {
  if (res == NULL) {
    return 0;
  }
  int length;

  /* Check if the content length on the current file have been accessed. */
  if (res->accessedCounter == 1) {
    res->accessedCounter = 2; // Mark as accessed
    length = res->index->size;

    // if its accessed, then check the next file.
  } else {
    length = result_get_content_length(res->next);
  }

  return length;
}

search_hit_t *result_next(search_result_t *res) {
  if (res == NULL || res->hitsArray == NULL) {
    return NULL;
  }
  search_hit_t *hitsData = NULL;

  /* Check if current file have been accessed */
  if (res->accessedCounter == 2) {

    /* Check if there is more hits result on the current file. */
    if (list_hasnext(res->hitsArray)) {
      hitsData = list_next(res->hitsArray);
    } else {

      /* if there is no more result, marked as accessed and check the next
       * existing file */
      res->accessedCounter = 3;
      if (res->next != NULL) {
        hitsData = result_next(res->next);
      }
    }

    // return the next existing data, when the current file have been used.
  } else {
    hitsData = result_next(res->next);
  }

  return hitsData;
}

/**
 * @param str: String to be convert to lower case.
 */
static void toLowerCase(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = tolower((unsigned char)str[i]);
  }
}

/**
 * @brief Create a search hits result for the multi-string search.
 * @param idx - The index_t structure that contains documents data.
 * @param tokens - The list of words that are used to search.
 * @param query - The query string that are used to search.
 * @return search_result_t* A pointer to the result .
 */
static search_result_t *multi_find(index_t *idx, list_t *tokens,
                                   const char *query) {

  /* Allocates memory block for multi-string */
  search_result_t *mainResult = index_find(idx, list_popfirst(tokens));
  search_result_t *tmp;

  int str_len = list_size(tokens);
  int subWordPos = 0;

  /* Find main-word locations that's contains the sub-word in the right position
   */
  int size = str_len;
  while (size > 0) {
    subWordPos++;

    /* Stores the mainResult position if the sub word exist next to it*/
    search_result_t *subWord = index_find(idx, list_popfirst(tokens));
    tmp = cmpSearchResult(mainResult, subWord, subWordPos, str_len, idx);

    /* Update the mainResult search hits position, to compare with the next word
     */
    mainResult = tmp;
    size = list_size(tokens);
  }

  /* Search other files */
  if (idx->next) {

    if (mainResult == NULL) {
      mainResult = index_find(idx->next, query);
    } else {
      mainResult->next = index_find(idx->next, query);
    }
  }
  return mainResult;
}

/**
 *
 * @brief Return a new search result that are created by comparing the main
 * search result locations with the sub search result location, where the sub
 * (location - word position) must be equal to the main location.
 *
 * @param main - The main search result that contains search hits on the first
 * word.
 * @param sub - The sub search result that contains search hits on the word in
 * position +( wordIndex ).
 * @param subWordPos - The current position of the word on the multi words
 * string.
 * @param str_len - The number of words in the multi words string.
 * @param idx - The index_t structure that contains documents data.
 * @return search_result_t* A pointer to the New search result.
 *
 * **/
static search_result_t *cmpSearchResult(search_result_t *main,
                                        search_result_t *sub, int subWordPos,
                                        int str_len, index_t *idx) {

  /* Return Null if there is nothing to compare */
  if (main == NULL || sub == NULL || main->hitsArray == NULL ||
      sub->hitsArray == NULL) {
    return NULL;
  }

  /* Create iter to avoid removing the original hits result */
  list_iter_t *main_iter = main->hitsArray;
  list_iter_t *sub_iter = sub->hitsArray;

  /* Create a new search result */
  search_result_t *NewResult = create_search_result_t(idx);
  list_t *hitsResult = list_create(NULL);

  bool hold_SubWord_Pos = false;
  while (list_hasnext(main_iter)) {

    /* Compare the sub-word index while main-word index is not empty */
    search_hit_t *current_hits = list_next(main_iter);
    while (list_hasnext(sub_iter)) {

      if (hold_SubWord_Pos) {
        goto next_mainWord_Pos;
      }
      search_hit_t *current_hits_sub = list_next(sub_iter);

    next_mainWord_Pos:
      hold_SubWord_Pos = false; /* Reset the jump */

      /* If main location > sub location, check the next location in sub */
      if (current_hits_sub->location - subWordPos < current_hits->location) {
        continue;

        /* If main location < sub location, hold the sub-word index to compare
         * with the next position in main */
      } else if (current_hits_sub->location - subWordPos >
                 current_hits->location) {
        hold_SubWord_Pos = true;
        break;

        /* If main location == sub location, store the location in the
         * hitsResult */
      } else if (current_hits->location ==
                 current_hits_sub->location - subWordPos) {

        search_hit_t *matchedPosition = malloc(sizeof(search_hit_t));
        matchedPosition->location = current_hits->location;
        matchedPosition->len = str_len;

        list_addlast(hitsResult, matchedPosition);
        break; /* Compare the next location in main with */
      }
    }
  }

  /* Store the hits result in the new search result */
  list_iter_t *res_it = list_createiter(hitsResult);
  NewResult->hitsArray = res_it;
  return NewResult;
}

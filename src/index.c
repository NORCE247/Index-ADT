#include <string.h>
#include <stdbool.h>
#include "index.h"
#include "printing.h"
#include "trie.h"
#include "map.h"
#include <ctype.h>
#include "common.h"


/**
 * @brief This structure represents an index entry for a document.
 * It contains information about the document along with related data structures for indexing.
 *
 * @struct index_t
 * @var documentName - A string containing the name of the document
 * @var stringArray - Represent document contents.
 * @var size - The length of the stringArray
 * @var trieTree - A trie structure used for autocompletion
 * @var next - A pointer to the next index_t structure
 * @var map - Hashmap
 */
typedef struct index
{
    char *documentName;
    char **stringArray;
    trie_t *trieTree;
    index_t *next;
    int size;
    map_t *map;

} index_t;

/**
 * @brief This structure represent a iteration for the search results,
 * and holds the information that's describe the length and found word location.
 * These information is stored in search_hit_t and search_hit_t is stored in a linked-list.
 *
 * @struct search_hit_t
 * @var hitsArray - A linked list containing search_hit_t
 * @var index - A pointer to index_t, is used to store the words in a document where search results is founded
 * @var next - Points to the next structure, and works like a linked list
 * @var accessedCounter - Represent the number of times a file have been visited.
 * [0,1,2,3] = [create, content, length, next content]
 *
 */
typedef struct search_result
{
    list_iter_t *hitsArray;
    index_t *index;
    search_result_t *next;
    int accessedCounter;

} search_result_t ;

/**
 * @brief Creates a new search result structure.
 * @param[in] idx - index_t structure
 * @return  A pointer to the newly allocated search result structure
 */
static search_result_t *create_search_result_t(index_t *idx) {

    search_result_t *new = malloc(sizeof(search_result_t));
    if ( new == NULL ) {
        return NULL;
    }

    new->index = idx;
    new->hitsArray = NULL;
    new->accessedCounter = 0;
    return new;
}

index_t *index_create()
{
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

void index_destroy(index_t *index)
{
    if (index != NULL) {

        free(index->stringArray);
        trie_destroy(index->trieTree);
        index_destroy(index->next);
        free(index);

    } else { return; } // Avoid to deallocate a non-allocated memory block.
}

/**
 * @param str: String to be convert tolower case.
 * */
void toLowerCase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

void index_add_document(index_t *idx, char *document_name, list_t *words)
{

    /* Check if the current index_t is empty and can be use & store data */
    if (idx->documentName == NULL) {

        // Mark as non-empty
        idx->documentName = document_name;
        int len = list_size(words);

        // Allocate memory to store content.
        idx->stringArray = malloc(sizeof(char*) * len + 1);
        idx->trieTree = trie_create();

        // Allocate Hashmap for the usage of search hits
        idx->map= map_create(compare_strings, djb2);

        // Insert words in the String array, and Trie Tree.
        for (int i = 0; i < len; ++i) {

            // Pack out the word from linked list and defined the len.
            char *word = list_popfirst(words);

            // Create a null terminated word.
            char *nullTerminated = strdup(word);

            // Insert a word into the array.
            idx->stringArray[i] = word;

            /** @VERSION 2 index.c line:143-152 **/
            //Define word location and length.
            search_hit_t *hit = malloc(sizeof(search_hit_t));
            hit->location = i;
            hit->len = 0;

            // Convert string to lower case
            char *cpyWord = strdup(word);
            toLowerCase(cpyWord);

            // Store the word's location and length in the hashmap.
            map_put(idx->map, (char*)cpyWord, hit);

            //insert null terminated word into the Trie-Tree.
            trie_insert(idx->trieTree, nullTerminated, NULL);
            idx->size++;
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

/** @VERSION 1 *//*
search_result_t *index_find(index_t *idx, const char *query)
{
    // Return NULL if there is no document to access.
    if (idx == NULL || idx->documentName == NULL ) { return NULL; }

    search_result_t *searchResult = create_search_result_t(idx);
    bool found = false;

    // Iterate through the words in the document.
    for (int i = 0; i < idx->size; ++i) {

        // Find a match word.
        char *currentWord = idx->stringArray[i];
        if (strcasecmp(currentWord, query) == 0) {

            found = true;
            search_hit_t *hit = malloc(sizeof(search_hit_t));
            if (hit != NULL) {

                // Define the matched word location and length.
                hit->location = i;
                hit->len = strlen(currentWord);

                // Store the search hit data in the linked list.
                list_addfirst(searchResult->hitsArray, hit);

            } else { return NULL; }
        }
    }

    if (found) {

        // Store the document content.
        searchResult->index = idx;

        // If there is more documents, use recursion to store the next search results
        if (idx->next != NULL){
            searchResult->next = index_find(idx->next, query);
        }

    } else {

        // Find query on the nest documents, if current Search Result is none,
        // store data on the current, else stores on the next Search Result
        if (searchResult != NULL ){
            searchResult->next = index_find(idx->next, query);
        } else  { searchResult = index_find(idx->next, query); }
    }

    return searchResult;
}*/

/** @VERSION 2 **/
search_result_t *index_find(index_t *idx, const char *query)
{
    // Return NULL if there is no document to access.
    if (idx == NULL || idx->documentName == NULL || query == NULL) { return NULL; }

    search_result_t *searchResult = create_search_result_t(idx);
    bool found = false;


    /* Check the number of string*/
    list_t *tokens = list_create(NULL);
    parse_word((char*)query, tokens);

    if (list_size(tokens) > 1) {
        search_result_t *main_word = index_find(idx, list_popfirst(tokens));
        search_result_t *array;
        int str_len = list_size(tokens);
        int i = 0;

        int size = list_size(tokens);
        while (size > 0){
            i++;
            char *word = list_popfirst(tokens);
            search_result_t *current_word = index_find(idx, word);
            array = diff_checker(main_word, current_word, i, str_len, idx);
            main_word = array;
            size = list_size(tokens);
        }

        //Create a new search_result_t
        search_result_t *searchResult_new = main_word;

        if (idx->next) {
            searchResult_new->next = index_find(idx->next, query);
        }
        return searchResult_new;
    }

    // Check if there is any search hits
    list_t *hits = map_get(idx->map, (char*)query);
    if (hits != NULL) {
        found = true;
        searchResult->hitsArray = list_createiter(hits);
    }

    if (found) {

        // Store the document content.
        searchResult->index = idx;

        /* If there is more documents, use recursion to store the next search results */
        if (idx->next != NULL){
            searchResult->next = index_find(idx->next, query);
        }

    } else {
        /* Find query on the nest documents, if current Search Result is none,
         * store data on the current, else stores on the next Search Result */
        if (searchResult != NULL ){
            searchResult->next = index_find(idx->next, query);
        } else  { searchResult = index_find(idx->next, query); }
    }

    return searchResult;
}

/** @VERSION 2 **/
search_result_t *diff_checker(search_result_t *main, search_result_t *sub, int i, int str_len, index_t*idx){

    list_iter_t *main_iter = (main->hitsArray);
    list_iter_t *sub_iter = (sub->hitsArray);
    if (main_iter == NULL || sub_iter == NULL)
        return NULL;

    search_result_t *array = create_search_result_t(idx);
    list_t *res = list_create(NULL);

    bool jump = false;
    while (list_hasnext(main_iter)){

        search_hit_t *current_hits = list_next(main_iter);
        while (list_hasnext(sub_iter)){

            if (jump){
                goto X;
            }
            search_hit_t *current_hits_sub = list_next(sub_iter);
            X:
            jump = false;
            if (current_hits_sub->location-i < current_hits->location){
                continue;

            } else if (current_hits_sub->location-i > current_hits->location){
                jump = true;
                break;

            } else if (current_hits->location == current_hits_sub->location-i){

                search_hit_t *new = malloc(sizeof (search_hit_t));
                new->location = current_hits->location;
                new->len = str_len;
                list_addlast(res, new);
                break;
            }

        }
    }
    list_iter_t *res_it = list_createiter(res);
    array->hitsArray = res_it;
    return array;
}

char *autocomplete(index_t *idx, char *input, size_t size)
{

    //Define the suggestion word that are found in the Trie-Tree.
    char*suggestion = trie_find(idx->trieTree, input);

    /* If there is no suggestion word, check the next document. */
    if (suggestion == NULL && idx->next != NULL) {
        suggestion = autocomplete(idx->next, input, size);
    }

    return suggestion;
}

char **result_get_content(search_result_t *res)
{
    if (res == NULL) { return NULL; }
    char **content = NULL;
    
    /* Check if the current file's content have been accessed. */
    if (res->accessedCounter == 0) {

        content = res->index->stringArray;
        res->accessedCounter = 1 ; // Mark as accessed

            // if its accessed, then check the next file.
    } else { content = result_get_content(res->next); }

    return content;
}

int result_get_content_length(search_result_t *res)
{
    if (res == NULL) { return 0; }
    int length;
    
    /* Check if the content length on the current file have been accessed. */
    if (res->accessedCounter == 1){
        res->accessedCounter = 2;  // Mark as accessed
        length = res->index->size;

        // if its accessed, then check the next file.
    } else { length = result_get_content_length(res->next); }

    return length;
}

/** @VERSION 1 *//*
search_hit_t *result_next(search_result_t *res)
{
    if (res == NULL) { return NULL; }
    search_hit_t *hitsData = NULL;

    // Check if there is more hit result on the current file.
    if (res->accessedCounter == 2){

        hitsData = list_poplast(res->hitsArray);

        // if there is no more result, marked as accessed and check the next existing file
        if (hitsData == NULL){
            res->accessedCounter = 3;

            if (res->next != NULL) {
                hitsData = result_next(res->next);
            }
        }

        // return the next existing data, when the current file have been used.
        } else { hitsData = result_next(res->next); }

    return hitsData;
}*/

/** @VERSION 2 **/
search_hit_t *result_next(search_result_t *res)
{
    if (res == NULL || res->hitsArray == NULL) { return NULL; }
    search_hit_t *hitsData = NULL;

    /* Check if there is more hits result on the current file. */
    if (res->accessedCounter == 2){

        if (list_hasnext(res->hitsArray)) {
            hitsData = list_next(res->hitsArray);
        } else {

            /* if there is no more result, marked as accessed and check the next existing file */
            res->accessedCounter = 3;
            if (res->next != NULL) {
                hitsData = result_next(res->next);
            }

        }

    // return the next existing data, when the current file have been used.
    } else { hitsData = result_next(res->next); }

    return hitsData;
}

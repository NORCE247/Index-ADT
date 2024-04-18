/* 
 * Authors: 
 * Steffen Viken Valvaag <steffenv@cs.uit.no> 
 * Magnus Stenhaug <magnus.stenhaug@uit.no> 
 * Erlend Helland Graff <erlend.h.graff@uit.no> 
 */

#include "map.h"
#include "printing.h"
#include "list.h"
#include <stdlib.h>
#include <ctype.h>
#include "index.h"


struct mapentry
{
    void *key;
    list_t *list; /** @MODIFIED include linked list */
    struct mapentry *next;
};

typedef struct mapentry mapentry_t;

struct map
{
    cmpfunc_t cmpfunc;
    hashfunc_t hashfunc;
    int size;
    mapentry_t **buckets;
    int numbuckets;
};


static mapentry_t *newentry(void *key, search_hit_t *hits, mapentry_t *next)
{
    mapentry_t *e;

    e = malloc(sizeof(mapentry_t));
    if (e == NULL)
    {
        ERROR_PRINT("out of memory");
        goto end;
    }

    e->key = key;

    /** @MODIFIED : Create and insert the first elem to the Linked list */
    e->list = list_create(NULL);
    list_addfirst(e->list,hits);
    e->next = next;

    end:
    return e;
}


map_t *map_create(cmpfunc_t cmpfunc, hashfunc_t hashfunc)
{
    map_t *map;

    map = malloc(sizeof(map_t));
    if (map == NULL)
    {
        ERROR_PRINT("out of memory");
        goto map_error;
    }

    map->cmpfunc = cmpfunc;
    map->hashfunc = hashfunc;
    map->size = 0;

    /** @MODIFIED SIZE: TO AVOID EXTENSION OF MAP */
    map->numbuckets = 10000;
    map->buckets = calloc(map->numbuckets, sizeof(mapentry_t *));
    if (map->buckets == NULL)
    {
        ERROR_PRINT("out of memory");
        goto buckets_error;
    }

    return map;

    buckets_error:
    free(map);
    map_error:
    return NULL;
}


static void freebuckets(int numbuckets, mapentry_t **buckets, void (*destroy_key)(void *), void (*destroy_val)(void *))
{
    int b;
    mapentry_t *e, *tmp;

    for (b = 0; b < numbuckets; b++)
    {
        e = buckets[b];
        while (e != NULL)
        {
            tmp = e;
            e = e->next;

            if (destroy_key && tmp->key)
                destroy_key (tmp->key);

            if (destroy_val && tmp->list)
                list_destroy(tmp->list);

            free(tmp);
        }
    }
    free(buckets);
}


void map_destroy(map_t *map, void (*destroy_key)(void *), void (*destroy_val)(void *))
{
    freebuckets(map->numbuckets, map->buckets, destroy_key, destroy_val);
    free(map);
}


/** @param hits : represent search hits, where it will be store in linked list*/
void map_put(map_t *map, void *key, search_hit_t *hits)
{
    unsigned long hash = map->hashfunc(key);
    int b = hash % map->numbuckets;
    mapentry_t *e = map->buckets[b];

    while (e != NULL && map->cmpfunc(key, e->key) != 0)
    {
        e = e->next;
    }
    if (e == NULL)
    {
        map->buckets[b] = newentry(key, hits, map->buckets[b]);
        map->size++;

    }
    else
    {   /**@MODIFIED Insert the hits result into the list */
        list_addlast(e->list, hits);
    }
}


int map_haskey(map_t *map, void *key)
{
    unsigned long hash = map->hashfunc(key);
    int b = hash % map->numbuckets;
    mapentry_t *e = map->buckets[b];

    while (e != NULL && map->cmpfunc(key, e->key) != 0)
    {
        e = e->next;
    }
    if (e == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


void *map_get(map_t *map, void *key)
{
    unsigned long hash = map->hashfunc(key);
    int b = hash % map->numbuckets;
    mapentry_t *e = map->buckets[b];

    while (e != NULL && map->cmpfunc(key, e->key) != 0)
    {
        e = e->next;
    }
    if (e == NULL)
    {
        return NULL;
    }
    else
    {
        /** @MODIFIED Return the linked list where it contains search hits */
        return e->list;
    }
}


unsigned long djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */

    return hash;
}

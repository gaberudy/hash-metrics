/*
 * cuckoo.c
 *
 * Dictionary code for cuckoo hashing.
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that due acknoweledgement to the authors are provided and
 * this permission notice appears in all copies of the software.
 * The software is provided "as is". There is no warranty of any kind.
 *
 *
 * Authors:
 *     Rasmus Pagh and Flemming Friche Rodler
 *     BRICS (Basic Research In Computer Science}
 *     Department of Computer Science
 *     University of Aarhus, Denmark
 *     {pagh,ffr}@brics.dk
 * 
 * Date: June 27, 2001.  
*/


#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<time.h>
#include<math.h>

typedef struct cell {       /* hash table cell type */ 
  int key; 
} cell;

typedef struct chain {
  int size;
  cell *c;
} chain;

struct dict {            /* dictionary type */ 
  int size;                 /* current size */
  int tablesize;            /* size of hash tables */
  int minsize,meansize;     /* rehash trigger sizes */
  int chain_size;
  chain *t;
  int kmask;
  hash_data h;
};

dict *alloc_dict(int tablesize) {
  dict *d;
  int i;
  int default_chain_size;
  
  d = calloc(1,sizeof(dict));
  d->size = 0;
  d->tablesize = tablesize;

  d->meansize = 5*(2*tablesize)/12;
  d->minsize =  (2*tablesize)/5;  

  if(!(d->t = calloc(tablesize, sizeof(chain)))) {
    fprintf(stderr,"Error while allocating mem for t\n");
    exit(0);
  }
  for(i=0; i<tablesize; i++) {
    d->t[i].size = default_chain_size;
    if(!(d->t[i].c = calloc(default_chain_size, sizeof(cell)))) {
      fprintf(stderr,"Error while allocating mem for chain\n");
      exit(0);
    }
  }
    
  hash_init(d->h);

  return d;
}

void rehash(dict* d, int new_size) {
  dict *dnew;
  int k;
  int i;

  dnew = alloc_dict(new_size);

  for(k = 0; k < d->tablesize; k++)
    for(i=0; i<d->t[k].size; i++)
      insert(dnew, d->t[k].c[i].key);

  free(d->t);
  dnew->size = d->size;
  *d = *dnew;
  free(dnew);
}

dict *construct_dict(int min_size) {
  return alloc_dict(min_size);
}

int insert(dict *d, int key) { 
  int hkey;
  int i;
  hkey = hash(d->h, key, d->kmask);
  chain *ch = &d->t[hkey];

  for(i=0; i<ch->size && ch->c[i].key; i++)
    if(ch->c[i].key == key) return 0;

  if(ch->size == i) {
    ch->c = realloc(ch->c, ch->size*2);
    ch->size *= 2;
  } else if (!i) {
    d->chain_size++;
  }
  ch->c[i].key = key;
  ch->size++;
  d->size++;

  if(d->tablesize < d->chain_size) rehash(d, 2*d->tablesize);
  return 1;
}

int lookup(dict* d, int key) {
  int hkey;
  int i;
  hkey = hash(d->h, key, d->kmask);
  chain *ch = &d->t[hkey];
  for(i=0; i<ch->size && ch->c[i].key; i++)
    if(ch->c[i].key == key) return 1;
  return 0;
}

int delete(dict *d, int key) {
  int hkey;
  int i;
  hkey = hash(d->h, key, d->kmask);
  chain *ch = &d->t[hkey];
  for(i=0; i<ch->size && ch->c[i].key; i++) {
    if(ch->c[i].key == key) {
      d->size--;
      if(!i) d->chain_size--;
//      if(d->size < d->minsize) ; /*rehash(d, d->tablesize/2);*/
      if(d->chain_size < d->minsize) rehash(d, d->tablesize/2);
      for(; i<ch->size-1 && ch->c[i].key; i++)
        ch->c[i] = ch->c[i+1];
      ch->c[i].key = 0;
      return 1;
    }
  }
  return 0;
}

int size(dict *d) {
  return d->size;
}

void clear(dict *d, int min_size) {
  dict *dnew;
  dnew = construct_dict(min_size);
  free(d->t);
  *d = *dnew;
  free(dnew);
}

dict *destruct_dict(dict *d) {
  free(d->t);
  free(d);
  return NULL;
}
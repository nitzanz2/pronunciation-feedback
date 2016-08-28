/*
 * dict.h -- dictionary
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Changed dict_init to return dict_t *.
 * 
 * 01-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University.
 * 		Created
 */


/*
 * NOTE: The system can only have ONE dictionary, maintained by this module.
 */


#ifndef _LIBFBS_DICT_H_
#define _LIBFBS_DICT_H_


#include "s3types.h"
#include <../libutil/libutil.h>

typedef struct {
    char *word;		/* Ascii word string */
    s3cipid_t *ciphone;	/* Pronunciation */
    int32 pronlen;	/* Pronunciation length */
    s3wid_t alt;	/* Next alternative pronunciation id, NO_WID if none */
    s3wid_t basewid;	/* Base pronunciation id */
} dictword_t;

typedef struct {
    dictword_t *word;	/* Array of entries in dictionary */
    hash_t ht;		/* Hash table for mapping word strings to word ids */
    int32 max_words;	/* #Entries allocated in dict, including empty slots */
    int32 n_word;	/* #Occupied entries in dict; ie, excluding empty slots */
    int32 filler_start;	/* First filler word id (read from filler dict) */
    int32 filler_end;	/* Last filler word id (read from filler dict) */
} dict_t;


/* -------------------------------- INTERFACE -------------------------------- */


/*
 * Initialize with given main and filler dictionary files.  fillerfile can be NULL.
 * Return ptr to dict_t if successful, NULL otherwise.
 */
dict_t *dict_init (char *dictfile, char *fillerfile);

/* Return base word id for given word id w (which may be itself).  w must be valid */
s3wid_t dict_basewid (s3wid_t w);

/* Return word id for given word string if present.  Otherwise return BAD_WID */
s3wid_t dict_wordid (char *word);

/* Return TRUE if w is a filler word, FALSE otherwise */
int32 dict_filler_word (s3wid_t w);

/* Return word string for given word id, which must be valid */
char *dict_wordstr (s3wid_t wid);

/* Return a pointer to the dictionary.  It MUST NOT be modified outside this module */
dict_t *dict_getdict ( void );

/* Return the no. of active words in the dictionary */
int32 dict_size ( void );

/*
 * Add a word with the given ciphone pronunciation list to the dictionary.
 * Return value: Result word id if successful, BAD_WID otherwise
 */
s3wid_t dict_add_word (char *word, s3cipid_t *p, int32 np);


#endif

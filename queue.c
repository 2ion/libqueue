/*
 * libqueue - provides persistent, named data storage queues
 * Copyright (C) 2014-2016 Jens Oliver John <dev@2ion.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

#include "queue.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <leveldb/c.h>
#include <limits.h>
#include <fcntl.h>

#define IFFN(X) {if (NULL != X) {free(X); X =NULL;}}
#define IFFNF(X,FUNC) {if (NULL != X) {FUNC(X); X =NULL;}}

struct Queue {
	leveldb_t * db;
	leveldb_iterator_t* readItr;
	leveldb_iterator_t* writeItr;
	leveldb_readoptions_t* rop;
	leveldb_writeoptions_t* wop;
	leveldb_options_t* options;
	leveldb_comparator_t * cmp;
	char * error_strp;
};
static void CmpDestroy(void* arg) {  }

static u_int64_t convertToKey(const char* a, size_t alen) {
	if (alen != sizeof(u_int64_t))
		return 0;
	return *((u_int64_t *)a);
}


static int CmpCompare(void* arg, const char* a, size_t alen, const char* b, size_t blen) {
	u_int64_t av = convertToKey(a, alen);
	u_int64_t bv = convertToKey(b, alen);
	if (av < bv) return -1;
	else if (av > bv) return +1;
	return 0;
}

const char * queue_get_last_error(const struct Queue * const q) {
	return q->error_strp;
}

static const char* CmpName(void* arg) {
	return "foo";

}

int queue_is_opened (const struct Queue * const q) {
	return NULL != q->db;
}

struct Queue * queue_open_with_options(const char * const path,... ) {
	struct Queue * q = malloc(sizeof (struct Queue));
	memset(q, 0, sizeof(struct Queue));

	q->options = leveldb_options_create();
	q->cmp = leveldb_comparator_create(NULL, CmpDestroy, CmpCompare, CmpName);
	leveldb_options_set_comparator(q->options, q->cmp);
	leveldb_options_set_create_if_missing(q->options, 1);

	q->rop = leveldb_readoptions_create();
	q->wop = leveldb_writeoptions_create();
	va_list argp;
	va_start(argp, path);
	const char * p;
	for (p = va_arg(argp, char *); p != NULL; p = va_arg(argp,char *)) {
		if (0 == strcmp(p, "failIfMissing")) {
			leveldb_options_set_create_if_missing(q->options,0);
		}
		if (0 == strcmp(p,"paranoidChecks")) {
			leveldb_options_set_paranoid_checks(q->options,1);
		}
		if (0 == strcmp(p,"writeBufferSize")) {
			size_t bufferSize= va_arg(argp, size_t);
			leveldb_options_set_write_buffer_size(q->options,bufferSize);
		}
		if (0 == strcmp(p,"blockSize")) {
			size_t blockSize= va_arg(argp, size_t);
			leveldb_options_set_block_size(q->options,blockSize);
		}
		if (0 == strcmp(p,"blockRestartInterval")) {
			size_t blockRestartInterval= va_arg(argp, size_t);
			leveldb_options_set_block_restart_interval(q->options,blockRestartInterval);
		}
		if (0 == strcmp(p,"maxOpenFiles")) {
			int maxOpenFiles= va_arg(argp, int );
			leveldb_options_set_max_open_files(q->options,maxOpenFiles);
		}
		if (0 == strcmp(p,"noCompress")) {
			leveldb_options_set_compression(q->options,0);
		}
		if (0 == strcmp(p,"syncWrte")) {
			leveldb_writeoptions_set_sync(q->wop , 1);
		}
	}
	va_end(argp);
	q->db = leveldb_open(q->options, path, &q->error_strp);
	return q;
}

struct Queue * queue_open(const char * const path) {
	return queue_open_with_options(path,NULL);
}
static void freeItrs(struct Queue * const q) {
	IFFNF(q->readItr, leveldb_iter_destroy);
	IFFNF(q->writeItr, leveldb_iter_destroy);
}

int queue_close(struct Queue *q) {
	assert(q != NULL);
	freeItrs(q);
	IFFNF(q->db, leveldb_close);
	IFFNF(q->options,leveldb_options_destroy);
	IFFNF(q->cmp,leveldb_comparator_destroy);
	IFFNF(q->wop,leveldb_writeoptions_destroy);
	IFFNF(q->rop, leveldb_readoptions_destroy);
	IFFN(q->error_strp);
	IFFN(q);
	return LIBQUEUE_SUCCESS;
}
static u_int64_t getKeyFromIter(leveldb_iterator_t * const itr) {
	char * lkey= NULL;
	size_t klen;
	lkey = (char *)leveldb_iter_key(itr, &klen);
	return convertToKey(lkey, klen);
}

int queue_push(struct Queue * const q, struct QueueData * const d) {
	assert(q != NULL);
	assert(d != NULL);
	assert(d->v != NULL);
	if (NULL == q->writeItr)
		q->writeItr= leveldb_create_iterator(q->db,q->rop);
	leveldb_iter_seek_to_last(q->writeItr);
	u_int64_t key;
	if (0 == leveldb_iter_valid(q->writeItr)) {
		key = 0;
	} else  {
		key = 1+ getKeyFromIter(q->writeItr);
	}
	if (q->error_strp) {
		free(q->error_strp);
		q->error_strp =NULL;
	}
	leveldb_put(q->db, q->wop,(const char *)&key, sizeof(u_int64_t),d->v, d->vlen, &q->error_strp);
	freeItrs(q);
	return LIBQUEUE_SUCCESS;
}

int queue_pop(struct Queue * const q, struct QueueData * const d) {
	assert(q != NULL);
	assert(d != NULL);
	if (NULL == q->readItr )
		q->readItr= leveldb_create_iterator(q->db,q->rop);
	leveldb_iter_seek_to_first(q->readItr);
	if (0 == leveldb_iter_valid(q->readItr)) {
		return LIBQUEUE_FAILURE;
	}
	d->v = (char *)leveldb_iter_value(q->readItr, &d->vlen);
	if (d->v) {
		char * tmp = malloc (d->vlen);
		memcpy(tmp, d->v, d->vlen);
		d->v = tmp;
	}

	u_int64_t key = getKeyFromIter(q->readItr);
	leveldb_iter_next(q->readItr);
	if (q->error_strp) {
		free(q->error_strp);
		q->error_strp =NULL;
	}
	leveldb_delete(q->db,  q->wop,(const char *) &key, sizeof(u_int64_t), &q->error_strp);
	freeItrs(q);
	return LIBQUEUE_SUCCESS;
}

int queue_len(struct Queue * const q, int64_t * const lenbuf) {
	assert(lenbuf != NULL);
	if (NULL == q->readItr) {
		q->readItr= leveldb_create_iterator(q->db,q->rop);
	}
	leveldb_iter_seek_to_first(q->readItr);
	if (0 == leveldb_iter_valid(q->readItr)) {
		*lenbuf = 0;
		return LIBQUEUE_SUCCESS;
	}
	size_t sizes[1]  = { 0 };
	u_int64_t starti = 0;
	u_int64_t limiti = ULLONG_MAX;

	const char * start[1] = {(const char *)&starti };
	size_t start_len[1] = { sizeof(u_int64_t)  };
	const char * limit[1] = {(const char *)&limiti };
	leveldb_approximate_sizes(q->db, 1, start,start_len, limit, start_len, sizes);
	// TODO, figure out fast way to get size
	*lenbuf= sizes[0];
	return LIBQUEUE_SUCCESS;
}

int queue_peek(struct Queue * const q, int64_t idx, struct QueueData * const d) {
	assert(q != NULL);
	assert(d != NULL);
	if (NULL == q->readItr )
		q->readItr= leveldb_create_iterator(q->db,q->rop);
	else {
		leveldb_iter_seek_to_first(q->readItr);
	}
	while (idx)  {
		if (0 == leveldb_iter_valid(q->readItr)) {
			return LIBQUEUE_FAILURE;
		}
		leveldb_iter_next(q->readItr);
		idx--;
	}
	if (0 == leveldb_iter_valid(q->readItr)) {
		return LIBQUEUE_FAILURE;
	}
	d->v = (char *)leveldb_iter_value(q->readItr, &d->vlen);
	return LIBQUEUE_SUCCESS;
}
int queue_poke(struct Queue *q, int64_t idx, struct QueueData *d){
	assert(q != NULL);
	assert(d != NULL);
	assert(d->v != NULL);
	if (NULL == q->readItr )
		q->readItr= leveldb_create_iterator(q->db,q->rop);
	else {
		leveldb_iter_seek_to_first(q->readItr);
	}
	while (idx)  {
		if (0 == leveldb_iter_valid(q->readItr)) {
			return LIBQUEUE_FAILURE;
		}
		leveldb_iter_next(q->readItr);
		idx--;
	}
	if (0 == leveldb_iter_valid(q->readItr)) {
		return LIBQUEUE_FAILURE;
	}
	int key = getKeyFromIter(q->readItr);
	leveldb_put(q->db, q->wop,(const char *)&key, sizeof(u_int64_t),d->v, d->vlen, &q->error_strp);
	return LIBQUEUE_SUCCESS;
}


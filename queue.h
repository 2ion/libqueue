/*
 * libqueue - provides persistent, named data storage queues
 * Copyright (C) 2014 Jens Oliver John <dev@2ion.de>
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


#ifndef QUEUE_H
#define QUEUE_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define QUEUE_DATADIR ("libqueue")
#define QUEUE_TUNINGSUFFIX "#type=kct#zcomp=gz#opts=c"

enum {
  LIBQUEUE_FAILURE = -1,
  LIBQUEUE_SUCCESS = 0,
  LIBQUEUE_MEM_ERROR = -2
};

struct Queue;

struct QueueData {
  void *v;
  u_int64_t vlen ;
};
struct Queue * queue_open_with_options(const char *path,...);
struct Queue * queue_open(const char * path);
void  queue_repair_with_options(const char * path,...);
void  queue_repair(const char * path);
int queue_is_opened (const struct Queue * const q);
int queue_push(struct Queue *q, struct QueueData *d);
int queue_pop(struct Queue *q, struct QueueData *d);
int queue_len(struct Queue *q, int64_t *len);
int queue_count(struct Queue *q, int64_t *count);
int queue_compact(struct Queue *q);
int queue_peek(struct Queue *q, int64_t s, struct QueueData *d);
int queue_poke(struct Queue *q, int64_t s, struct QueueData *d);
int queue_close(struct Queue *q);
int queue_opened(struct Queue *q);
const char * queue_get_last_error(const struct Queue * const q);
#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

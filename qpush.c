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

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include "queue.h"
#include <signal.h>
#include "queueutils.h"

/* global so that we can close the database properly in case of an
 * interrupt while reading from stdin */
struct Queue *q;

static inline void push_buf(struct Queue *q,
    struct QueueData *d, char*linebuf, size_t*linebuflen) {
  d->v = linebuf;
  d->vlen = *linebuflen-(2*sizeof(char));//remove trailing newline
  if(queue_push(q, d) != LIBQUEUE_SUCCESS){
    fputs("Failed to push value onto the queue: ", stdout);
    puts(linebuf);
  }
  memset(linebuf, 0, *linebuflen);
}

void sighandler(int s) {
  puts("Cought SIGINT and SIGTERM while reading from stdin.\n"
      "I'll try to exit gracefully...");
  exit(EXIT_FAILURE);
}

void do_at_exit(void) {
  if(queue_close(q) != LIBQUEUE_SUCCESS)
    puts("Failed to close the queue");
}

int main(int argc, char **argv) {
  struct QueueData d;
  int i = 0;
  char *cq = NULL;
  int opt = 0;
  int use_stdin = 0;
  int use_null = 0;
  char *linebuf = NULL;
  size_t linebuflen = 0;

  while((opt = getopt(argc, argv, "0rhq:")) != -1)
    switch(opt) {
      case 'q':
        cq = strdup(optarg);
        break;
      case 'r':
        use_stdin = 1;
        break;
      case '0':
        use_null = 1;
        break;
      default:
      case 'h':
        puts("Usage: qpush [-h] [-q queue-name] [--] <args>");
        return EXIT_FAILURE;
    }

  i = optind-1;
  q = queue_open(SELECTQUEUE(cq));
  if(0 == queue_is_opened(q)) {
    fprintf(stderr,"Failed to open the queue:%s\n", queue_get_last_error(q));
    queue_close(q);
    return EXIT_FAILURE;
  }

  atexit(do_at_exit);
  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  switch(use_stdin) {
    default:
    case 0:
      while(argv[++i]) {
        d.v = argv[i];
        d.vlen = sizeof(char)*(strlen(argv[i])+1);
        if(queue_push(q, &d) != LIBQUEUE_SUCCESS) {
          fputs("Failed to push value onto the queue: ", stdout);
          puts(argv[i]);
        }
      }
      break;
    case 1:
      linebuf = calloc(256, sizeof(char));
      linebuflen = 256*sizeof(char);
      switch(use_null) {
        default:
        case 0:
          while(getline(&linebuf, &linebuflen, stdin) != -1)
            push_buf(q, &d, linebuf, &linebuflen);
          break; // case 0:use_null
        case 1:
          while(getdelim(&linebuf, &linebuflen, '\0', stdin) != -1)
            push_buf(q, &d, linebuf, &linebuflen);
          break;
      }
      break; // case 1:use_stdin
      free(linebuf);
  }
  if(cq != NULL)
    free(cq);
  return 0;
}

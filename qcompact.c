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

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "queue.h"
#include "queueutils.h"

int main(int argc, char **argv) {
  struct Queue *q;
  int opt = 0;
  char *cq = NULL;

  while((opt = getopt(argc, argv, "hq:")) != -1)
    switch(opt) {
      case 'q':
        cq = strdup(optarg);
        break;
      default:
        puts("Usage: qlen [-h] [-q queue-name] [--]");
        return EXIT_FAILURE;
    }
  q = queue_open(SELECTQUEUE(cq));
  if(0 == queue_is_opened(q)) {
    fprintf(stderr,"Failed to open the queue:%s", queue_get_last_error(q));
    closequeue(q);
    return EXIT_FAILURE;
  }
  if(queue_compact(q) != LIBQUEUE_SUCCESS) {
    closequeue(q);
    return EXIT_FAILURE;
  }
  if(cq != NULL)
    free(cq);
  return closequeue(q);
}

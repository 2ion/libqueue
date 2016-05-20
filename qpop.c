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
#include <stdlib.h>
#include <getopt.h>
#include "queue.h"

#include "queueutils.h"

int main(int argc, char **argv) {
  struct Queue * q;
  struct QueueData d;
  int64_t l = 0;
  int opt = 0;
  char *cq = NULL;

  while((opt = getopt(argc, argv, "hq:")) != -1)
    switch(opt){
      case 'q':
        cq = strdup(optarg);
        break;
      default:
      case 'h':
        puts("Usage: qpop [-h] [-q queue-name]");
        return EXIT_FAILURE;
    }

  if((q= queue_open( SELECTQUEUE(cq))) == NULL) {
    puts("Failed to open the queue.");
    return EXIT_FAILURE;
  }
  if(queue_len(q, &l) != LIBQUEUE_SUCCESS) {
    puts("Failed to retrieve the queue length.");
    closequeue(q);
    return EXIT_FAILURE;
  }
  if(l == 0) {
    closequeue(q);
    return EXIT_FAILURE;
  }
  if(queue_pop(q, &d) != LIBQUEUE_SUCCESS){
    puts("Failed to retrieve the value from the queue.");
    closequeue(q);
    return EXIT_FAILURE;
  }
  printf("%s\n", (const char*)d.v);
  free(d.v);
  if(cq != NULL)
    free(cq);
  return closequeue(q);
}

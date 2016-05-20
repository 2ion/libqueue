#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include <libgen.h>
#include <string.h>

int main(int argc, char **argv) {
  struct Queue *q;
  int32_t ft = 42;
  struct QueueData qd = { "HENRY" , sizeof("HENRY") };
  struct QueueData qd2;
  puts(argv[0]);

  char template[] = "/tmp/XXXXXX";
  mkdtemp(template);

  if(NULL == (q=queue_open(template))) {
    puts("there was an error opening the queue!");
    return 1;
  }
  puts("queue successfully opened!");

  if(queue_push(q, &qd) != 0)
    puts("pushing HENRY failed!");

  if(queue_pop(q, &qd2) != 0) {
    puts("popping HENRY failed!");
  } else {
    puts("here's what popped!");
    puts((const char*)qd2.v);
  }

  qd.v = &ft;
  qd.vlen = sizeof(int32_t);
  queue_push(q, &qd);
  queue_pop(q, &qd2);
  printf("popping forty-two: %d\n", *(int32_t*)qd2.v);


  if(queue_close(q) != 0)
    puts("there was an error closing the queue!");
  puts("queue successfully closed!");
  return 0;
}

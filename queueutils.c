#include <stdlib.h>
#include "queueutils.h"

int closequeue(struct Queue *q) {
  if(queue_close(q) == LIBQUEUE_SUCCESS)
    return EXIT_SUCCESS;
  else {
    puts("Failed to close the queue properly.");
    return EXIT_FAILURE;
  }
}

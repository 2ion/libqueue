#ifndef QUEUEUTILS_H
#define QUEUEUTILS_H

#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define QUEUEUTILS_QUEUE (".")
#define SELECTQUEUE(pvar) ((pvar)!=NULL?(pvar):(QUEUEUTILS_QUEUE))

int closequeue(struct Queue *q);

#endif /* QUEUEUTILS_H */

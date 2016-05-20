#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "queue.h"
#include "queueutils.h"

int main(int argc, char **argv) {
    struct Queue *q;
    struct QueueData d;
    int64_t l = 0;
    int64_t i = 0;
    char *cq = NULL;
    int opt = 0;

    while((opt = getopt(argc, argv, "hq:")) != -1)
        switch(opt) {
            case 'q':
                cq = strdup(optarg);
                break;
            default:
            case 'h':
                puts("Usage: qpoke [-h] [-q queue-name] [--] index value");
                return EXIT_FAILURE;
        }

    if(argc-optind != 2){
        puts("Too few arguments.");
        return EXIT_FAILURE;
    }
    q = queue_open( SELECTQUEUE(cq));
    if(0 == queue_is_opened(q)) {
        fprintf(stderr,"Failed to open the queue:%s", queue_get_last_error(q));
        closequeue(q);
        return EXIT_FAILURE;
    }
    if(queue_len(q, &l) != LIBQUEUE_SUCCESS) {
        puts("Failed to read the queue length.");
        closequeue(q);
        return EXIT_FAILURE;
    }
    if((i = (int64_t)atoi((const char*)argv[optind])) < 0
            || (i+1)>l) {
        puts("Index value is out of bounds.");
        closequeue(q);
        return EXIT_FAILURE;
    }
    d.v = argv[optind+1];
    d.vlen = sizeof(char)*(strlen(argv[optind+1])+1);
    if(queue_poke(q, i-1, &d) != LIBQUEUE_SUCCESS) {
        puts("Failed to poke.");
        closequeue(q);
        return EXIT_FAILURE;
    }
    if(cq != NULL)
        free(cq);
    return closequeue(q);
}

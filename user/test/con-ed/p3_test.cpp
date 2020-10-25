//
// Created by joe on 10/25/20.
//

/**
 * reference from hot_potato
 */
#include "con_ed.h"
#define usage(arg0) fprintf(stderr, "usage: %s [<nthreads> <niters>]\n", arg0)
void *cc_thread(void *ignore)
{
    int i;
    for (i = 0; i < 10; i++) {
        int operation = random() % 4;
        char *banana = "banana";
        uint32_t key = rand() % 64;
        if (operation == 0)
            kkv_init(0);
        else if (operation == 1)
            kkv_destroy(0);
        else if (operation == 2)
            kkv_put(key, banana, strlen(banana) + 1, 0);
        else
            kkv_get(key, banana, MAX_VAL_SIZE, KKV_NONBLOCK);
    }
    return NULL;
}
void test_part2(int nthreads)
{
    pthread_t *threads;
    int i;
    threads = malloc(nthreads * sizeof(*threads));
    for (i = 0; i < nthreads; i++) {
        int ret = pthread_create((threads + i), NULL, cc_thread, NULL);
        if (ret)
            die("pthread_create() failed");
    }
    for (i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);
    kkv_destroy(0);
    free(threads);
}
int main(int argc, char **argv)
{
    int nthreads = 15;
    RUN_TEST(test_part2, nthreads);
    return 0;
}
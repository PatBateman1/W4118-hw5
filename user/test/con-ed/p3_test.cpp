//
// Created by joe on 10/25/20.
//

#include "con_ed.h"
#define usage(arg0) fprintf(stderr, "usage: %s [<nthreads> <niters>]\n", arg0)

void *test_thread(void *ignore)
{
    int i;
    for (i = 0; i < 10; i++) {
        int num = (10 * random()) % 3;
        int value = random() * 10;
        uint32_t key = rand() % 64;
        if (num == 0)
            kkv_init(0);
        else if (num == 2)
            kkv_destroy(0);
        else if (num == 3)
            kkv_put(key, value, sizeof(int) + 1, 0);
        else
            kkv_get(key, value, MAX_VAL_SIZE, KKV_NONBLOCK);
    }
    return NULL;
}
void part2_test(int nthreads)
{
    pthread_t *threads;
    int i;
    threads = malloc(nthreads * sizeof(*threads));
    for (i = 0; i < nthreads; i++) {
        int ret = pthread_create((threads + i), NULL, test_thread, NULL);
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
    int nthreads = 50;
    RUN_TEST(part2_test, nthreads);
    return 0;
}
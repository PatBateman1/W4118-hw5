/*
 * test part 1

 */

#include "con_ed.h"

#define usage(arg0) fprintf(stderr, "usage: %s [<nthreads>]\n", arg0)

#define KEY 0xbae
#define NOT 0xbbb
#define POTATO "hot potato"

pid_t gettid(void)
{
	return syscall(__NR_gettid);
}

void *init_test_thread(void *ignore)
{
	DEBUG("Thread [%u] init_hotpotato initialized\n", gettid());

	/* let loose the hot potato! */
	assert(kkv_put(KEY, POTATO, strlen(POTATO) + 1, 0) == 0);

	pthread_exit(NULL);
}

void *test_thread(void *ignore)
{
	char res[MAX_VAL_SIZE];

	/* spin wait for the hot potato */
	while (kkv_get(KEY, res, MAX_VAL_SIZE, KKV_NONBLOCK) != 0)
		DEBUG("[%u] Cold potato!\n", gettid());

	/* check that we still have the same potato */
	assert(strcmp(res, POTATO) == 0);

	/* pass on the potato */
	assert(kkv_put(KEY, res, strlen(res) + 1, 0) == 0);

    /* test key not in ht */
    kkv_get(NOT, res, MAX_VAL_SIZE, KKV_NONBLOCK);
    assert(errno == ENOENT);

	pthread_exit(NULL);
}

void test(int nthreads)
{
	pthread_t threads[nthreads];
	int i, ret;

	kkv_init(0);

	ret = pthread_create((pthread_t *) threads, NULL,
			init_test_thread, NULL);
	if (ret)
		die("pthread_create() failed");

	for (i = 1; i < nthreads; i++) {
		ret = pthread_create((threads + i), NULL,
				test_thread, NULL);
		if (ret)
			die("pthread_create() failed");
	}

	for (i = 0; i < nthreads; i++)
		pthread_join(threads[i], NULL);

	kkv_destroy(0);
}

int main(int argc, char **argv)
{
	int nthreads = argc > 1 ? atoi(argv[1]) : 64;

	if (!nthreads) {
		usage(argv[0]);
		return 2;
	}

	RUN_TEST(test, nthreads);
	return 0;
}

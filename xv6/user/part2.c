#include "types.h"
#include "stat.h"
#include "user.h"

/*
 * Takes two command-line arguments: the number of threads and the number of
 * nodes each thread should add to a shared linked list.
 *
 * Should exit cleanly with no output.
 */

static int numthreads;
static int* pids = NULL;

struct node {
	int value;
	struct node* next;
};

static void bail(void)
{
	int i, count = 0;

	for (i = 0; i < numthreads; i++) {
		if (pids[i] != -1) {
			thread_join();
			count += 1;
		}
	}

	printf(1, "[cleaned up %d threads before exiting]\n", count);

	exit();
}

static struct node* new_node(int val)
{
	struct node* n = malloc(sizeof(*n));
	if (!n) {
		printf(1, "oops, malloc failed\n");
		exit();
	}
	n->value = val;
	n->next = NULL;
	return n;
}

struct node* head = NULL;
struct spinlock lock;

static void threadfunc(void* arg)
{
	int i;
	struct node* n;

	for (i = 0; i < (int)arg; i++) {
		n = new_node(i);

		spin_lock(&lock);
		n->next = head;
		head = n;
		spin_unlock(&lock);
	}

	exit();
}

int
main(int argc, char *argv[])
{
	int i, j, pid, count;
	int sum, numnodes;
	struct node* n;
	struct node* tmp;

	if (argc != 3) {
		printf(1, "Usage: part2 NUMTHREADS COUNT\n");
		exit();
	}

	numthreads = atoi(argv[1]);
	count = atoi(argv[2]);

	pids = malloc(numthreads * sizeof(*pids));
	if (!pids) {
		printf(1, "oops, malloc failed\n");
		exit();
	}

	for (i = 0; i < numthreads; i++)
		pids[i] = -1;

	spin_init(&lock);

	for (i = 0; i < numthreads; i++) {
		pids[i] = thread_create(threadfunc, (void*)count);
		if (pids[i] < 1) {
			printf(1, "oops, thread_create() failed\n");
			bail();
		}
	}

	for (i = 0; i < numthreads; i++) {
		pid = thread_join();
		if (pid < 0) {
			printf(1, "oops, thread_join() returned -1 unexpectedly\n");
			bail();
		}
		for (j = 0; j < numthreads; j++) {
			if (pids[j] == pid) {
				pids[j] = -1;
				break;
			}
		}
		if (j == numthreads) {
			printf(1, "oops, thread_join() returned unexpected pid\n");
			bail();
		}
	}

	free(pids);

	numnodes = sum = 0;
	for (n = head; n; n = tmp) {
		tmp = n->next;
		sum += n->value;
		numnodes += 1;
		free(n);
	}

	if (numnodes != count * numthreads) {
		printf(1, "oops, list isn't the right length\n");
		exit();
	}

	if (sum != numthreads * ((count * (count - 1)) / 2)) {
		printf(1, "oops, list didn't add up to right value\n");
		exit();
	}

	exit();
}

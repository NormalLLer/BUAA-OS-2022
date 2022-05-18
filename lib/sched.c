#include <env.h>
#include <pmap.h>
#include <printf.h>

/* Overview:
 *  Implement simple round-robin scheduling.
 *
 *
 * Hints:
 *  1. The variable which is for counting should be defined as 'static'.
 *  2. Use variable 'env_sched_list', which is a pointer array.
 *  3. CANNOT use `return` statement!
 */
/*** exercise 3.15 ***/
void sched_yield(void)
{
    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
    static struct Env* cur = NULL;
    /*  hint:
     *  1. if (count==0), insert `e` into `env_sched_list[1-point]`
     *     using LIST_REMOVE and LIST_INSERT_TAIL.
     *  2. if (env_sched_list[point] is empty), point = 1 - point;
     *     then search through `env_sched_list[point]` for a runnable env `e`, 
     *     and set count = e->env_pri
     *  3. count--
     *  4. env_run()
     *
     *  functions or macros below may be used (not all):
     *  LIST_INSERT_TAIL, LIST_REMOVE, LIST_FIRST, LIST_EMPTY
     */
// ****** self annotations ******
	// in env.c/env_alloc():  set e->env_status = ENV_RUNNABLE;
	// in env.c/env_create_priority(): LIST_INSERT_HEAD(env_sched_list, e, env_sched_link);
	// in env.c/env_free(): LIST_REMOVE(e, env_sched_link);
	if (count <= 0 || cur->env_status != ENV_RUNNABLE) {
		do {
			if (LIST_EMPTY(&env_sched_list[point])) {
				point = 1 - point;
			}	
			cur = LIST_FIRST(&env_sched_list[point]);
			if (cur != NULL) {
				LIST_REMOVE(cur, env_sched_link);
				LIST_INSERT_TAIL(&env_sched_list[1 - point], cur, env_sched_link);
				count = cur->env_pri;
			}
		} while (cur == NULL || cur->env_status != ENV_RUNNABLE);
	}
	--count;
	env_run(cur);
}

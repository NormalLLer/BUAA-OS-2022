#include "../drivers/gxconsole/dev_cons.h"
#include <mmu.h>
#include <env.h>
#include <printf.h>
#include <pmap.h>
#include <sched.h>

extern char *KERNEL_SP;
extern struct Env *curenv;

struct msg {
	LIST_ENTRY(msg) q_link;
	int s_id;
	int r_id;
	int value;
	int srcva;
	int perm;
};
LIST_HEAD(msg_list, msg);
struct msg_list msgs;
int init = 0;
/* Overview:
 * 	This function is used to print a character on screen.
 *
 * Pre-Condition:
 * 	`c` is the character you want to print.
 */
void sys_putchar(int sysno, int c, int a2, int a3, int a4, int a5)
{
	printcharc((char) c);
	return ;
}

/* Overview:
 * 	This function enables you to copy content of `srcaddr` to `destaddr`.
 *
 * Pre-Condition:
 * 	`destaddr` and `srcaddr` can't be NULL. Also, the `srcaddr` area
 * 	shouldn't overlap the `destaddr`, otherwise the behavior of this
 * 	function is undefined.
 *
 * Post-Condition:
 * 	the content of `destaddr` area(from `destaddr` to `destaddr`+`len`) will
 * be same as that of `srcaddr` area.
 */
void *memcpy(void *destaddr, void const *srcaddr, u_int len)
{
	char *dest = destaddr;
	char const *src = srcaddr;

	while (len-- > 0) {
		*dest++ = *src++;
	}

	return destaddr;
}

/* Overview:
 *	This function provides the environment id of current process.
 *
 * Post-Condition:
 * 	return the current environment id
 */
u_int sys_getenvid(void)
{
	return curenv->env_id;
}

/* Overview:
 *	This function enables the current process to give up CPU.
 *
 * Post-Condition:
 * 	Deschedule current process. This function will never return.
 *
 * Note:
 *  For convenience, you can just give up the current time slice.
 */
/*** exercise 4.6 ***/
void sys_yield(void)
{
	bcopy((void*) (KERNEL_SP - sizeof(struct Trapframe)), 
		  (void*) (TIMESTACK - sizeof(struct Trapframe)), sizeof(struct Trapframe));
	sched_yield();
}

/* Overview:
 * 	This function is used to destroy the current environment.
 *
 * Pre-Condition:
 * 	The parameter `envid` must be the environment id of a
 * process, which is either a child of the caller of this function
 * or the caller itself.
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 when error occurs.
 */
int sys_env_destroy(int sysno, u_int envid)
{
	/*
		printf("[%08x] exiting gracefully\n", curenv->env_id);
		env_destroy(curenv);
	*/
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0) {
		return r;
	}

	printf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

/* Overview:
 * 	Set envid's pagefault handler entry point and exception stack.
 *
 * Pre-Condition:
 * 	xstacktop points to one byte past exception stack.
 *
 * Post-Condition:
 * 	The envid's pagefault handler will be set to `func` and its
 * 	exception stack will be set to `xstacktop`.
 * 	Returns 0 on success, < 0 on error.
 */
/*** exercise 4.12 ***/
int sys_set_pgfault_handler(int sysno, u_int envid, u_int func, u_int xstacktop)
{
	// Your code here.
	struct Env *env;
	int ret;


	return 0;
	//	panic("sys_set_pgfault_handler not implemented");
}

/* Overview:
 * 	Allocate a page of memory and map it at 'va' with permission
 * 'perm' in the address space of 'envid'.
 *
 * 	If a page is already mapped at 'va', that page is unmapped as a
 * side-effect.
 *
 * Pre-Condition:
 * perm -- PTE_V is required,
 *         PTE_COW is not allowed(return -E_INVAL),
 *         other bits are optional.
 *
 * Post-Condition:
 * Return 0 on success, < 0 on error
 *	- va must be < UTOP
 *	- env may modify its own address space or the address space of its children
 */
/*** exercise 4.3 ***/
int sys_mem_alloc(int sysno, u_int envid, u_int va, u_int perm)
{
	// Your code here.
	struct Env *env;
	struct Page *ppage;
	int ret = 0;
	if ((!(perm & PTE_V)) || (perm & PTE_COW) || va >= UTOP) return -E_INVAL;
	if ((ret = envid2env(envid, &env, 1)) < 0) return ret;  // 3rd argument: checkperm = 1
	if ((ret = page_alloc(&ppage)) < 0) return ret;
	if ((ret = page_insert(env->env_pgdir, ppage, va, perm)) < 0) return ret;
	return 0;
}

/* Overview:
 * 	Map the page of memory at 'srcva' in srcid's address space
 * at 'dstva' in dstid's address space with permission 'perm'.
 * Perm must have PTE_V to be valid.
 * (Probably we should add a restriction that you can't go from
 * non-writable to writable?)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Note:
 * 	Cannot access pages above UTOP.
 */
/*** exercise 4.4 ***/
int sys_mem_map(int sysno, u_int srcid, u_int srcva, u_int dstid, u_int dstva, u_int perm)
{
	int ret;
	u_int round_srcva, round_dstva;
	struct Env *srcenv;
	struct Env *dstenv;
	struct Page *ppage;
	Pte *ppte;

	ppage = NULL;
	ret = 0;
	round_srcva = ROUNDDOWN(srcva, BY2PG);
	round_dstva = ROUNDDOWN(dstva, BY2PG);

    //your code here
	if ((!(perm & PTE_V)) || (srcva >= UTOP) || (dstva >= UTOP)) return -E_INVAL;
	if ((ret = envid2env(srcid, &srcenv, 0)) < 0) return ret;
	if ((ret = envid2env(dstid, &dstenv, 0)) < 0) return ret;
	if ((ppage = page_lookup(srcenv->env_pgdir, round_srcva, &ppte)) == NULL) return -E_INVAL;
	ret = page_insert(dstenv->env_pgdir, ppage, round_dstva, perm);
	return ret;
}

/* Overview:
 * 	Unmap the page of memory at 'va' in the address space of 'envid'
 * (if no page is mapped, the function silently succeeds)
 *
 * Post-Condition:
 * 	Return 0 on success, < 0 on error.
 *
 * Cannot unmap pages above UTOP.
 */
/*** exercise 4.5 ***/
int sys_mem_unmap(int sysno, u_int envid, u_int va)
{
	// Your code here.
	int ret;
	struct Env *env;
	if (va >= UTOP) return -E_INVAL;
	if ((ret = envid2env(envid, &env, 0)) < 0) return ret;
	page_remove(env->env_pgdir, va);  // ret type of page_remove: void

	return ret;
	//	panic("sys_mem_unmap not implemented");
}

/* Overview:
 * 	Allocate a new environment.
 *
 * Pre-Condition:
 * The new child is left as env_alloc created it, except that
 * status is set to ENV_NOT_RUNNABLE and the register set is copied
 * from the current environment.
 *
 * Post-Condition:
 * 	In the child, the register set is tweaked so sys_env_alloc returns 0.
 * 	Returns envid of new environment, or < 0 on error.
 */
/*** exercise 4.8 ***/
int sys_env_alloc(void)
{
	// Your code here.
	int r;
	struct Env *e;


	return e->env_id;
	//	panic("sys_env_alloc not implemented");
}

/* Overview:
 * 	Set envid's env_status to status.
 *
 * Pre-Condition:
 * 	status should be one of `ENV_RUNNABLE`, `ENV_NOT_RUNNABLE` and
 * `ENV_FREE`. Otherwise return -E_INVAL.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if status is not a valid status for an environment.
 * 	The status of environment will be set to `status` on success.
 */
/*** exercise 4.14 ***/
int sys_set_env_status(int sysno, u_int envid, u_int status)
{
	// Your code here.
	struct Env *env;
	int ret;

	return 0;
	//	panic("sys_env_set_status not implemented");
}

/* Overview:
 * 	Set envid's trap frame to tf.
 *
 * Pre-Condition:
 * 	`tf` should be valid.
 *
 * Post-Condition:
 * 	Returns 0 on success, < 0 on error.
 * 	Return -E_INVAL if the environment cannot be manipulated.
 *
 * Note: This hasn't be used now?
 */
int sys_set_trapframe(int sysno, u_int envid, struct Trapframe *tf)
{

	return 0;
}

/* Overview:
 * 	Kernel panic with message `msg`.
 *
 * Pre-Condition:
 * 	msg can't be NULL
 *
 * Post-Condition:
 * 	This function will make the whole system stop.
 */
void sys_panic(int sysno, char *msg)
{
	// no page_fault_mode -- we are trying to panic!
	panic("%s", TRUP(msg));
}

/* Overview:
 * 	This function enables caller to receive message from
 * other process. To be more specific, it will flag
 * the current process so that other process could send
 * message to it.
 *
 * Pre-Condition:
 * 	`dstva` is valid (Note: 0 is also a valid value for `dstva`).
 *
 * Post-Condition:
 * 	This syscall will set the current process's status to
 * ENV_NOT_RUNNABLE, giving up cpu.
 */
/*** exercise 4.7 ***/

void sys_ipc_recv(int sysno, u_int dstva)
{
    if (dstva >= UTOP) return;
	struct msg *m;
    LIST_FOREACH(m, &msgs, q_link) {
		if (m->r_id == curenv->env_id) {
			struct Env* s;
			envid2env(m->s_id, &s, 0);
			s->env_status = ENV_RUNNABLE;
			curenv->env_ipc_recving = 1;
		    curenv->env_ipc_dstva = dstva;
	    	curenv->env_status = ENV_NOT_RUNNABLE;	
			LIST_REMOVE(m, q_link);
			sys_yield();
		}
	}

    curenv->env_ipc_recving = 1;
    curenv->env_ipc_dstva = dstva;
    curenv->env_status = ENV_NOT_RUNNABLE;
    sys_yield();
}

/*** exercise 4.7 ***/
int sys_ipc_can_send(int sysno, u_int envid, u_int value, u_int srcva, u_int perm)
{  // envid: target env's envid
// srcva == 0: just send value, not set ppage   
    if (!init) {
        init = 1;
        LIST_INIT(&msgs);
    }
    int r;
    struct Env *e;  // targer env
    struct Page *p;
    struct msg message;
    if (srcva >= UTOP) return -E_INVAL;
    if ((r = envid2env(envid, &e, 0)) < 0) return r;
    if (e->env_ipc_recving == 0) {
        curenv->env_status = ENV_NOT_RUNNABLE;
        message.s_id = curenv->env_id;
        message.r_id = envid;
        message.value = value;
        message.srcva = srcva;
        message.perm = perm;
        LIST_INSERT_TAIL(&msgs, &message, q_link);
        do {
            sys_yield();
        } while (e->env_ipc_recving == 0);
        struct msg* m;
        LIST_FOREACH(m, &msgs, q_link) {
            if (m->s_id == curenv->env_id) {
                envid2env(envid, &e, 0);
                e->env_ipc_value = m->value;
                e->env_ipc_recving = 0;
                e->env_ipc_perm = m->perm;
                e->env_status = ENV_RUNNABLE;
                if (srcva) {
                     if ((p = page_lookup(curenv->env_pgdir, srcva, NULL)) == NULL) return -E_INVAL;
                     if ((r = page_insert(e->env_pgdir, p, e->env_ipc_dstva, perm)) < 0) return r;
                }
                break;
            }
        }
    } else {
        e->env_ipc_value = value;
        e->env_ipc_recving = 0;
        e->env_ipc_from = curenv->env_id;
        e->env_ipc_perm = perm;
        e->env_status = ENV_RUNNABLE;
        if (srcva) {
            if ((p = page_lookup(curenv->env_pgdir, srcva, NULL)) == NULL) return -E_INVAL;
            if ((r = page_insert(e->env_pgdir, p, e->env_ipc_dstva, perm)) < 0) return r;
        }
    }
    //return -E_IPC_NOT_RECV;
    return 0;
}

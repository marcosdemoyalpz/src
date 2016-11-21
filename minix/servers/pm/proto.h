/* Function prototypes. */

struct mproc;

#include <minix/timers.h>


/**
SEM_MAX is the max number of semphores that the system can use
at any given time
*/
#define SEM_MAX 30

/* alarm.c */
int do_itimer(void);
void set_alarm(struct mproc *rmp, clock_t ticks);
void check_vtimer(int proc_nr, int sig);

/* exec.c */
int do_exec(void);
int do_newexec(void);
int do_execrestart(void);
void exec_restart(struct mproc *rmp, int result, vir_bytes pc, vir_bytes sp,
	vir_bytes ps_str);

/* forkexit.c */
int do_fork(void);
int do_srv_fork(void);
int do_exit(void);
void exit_proc(struct mproc *rmp, int exit_status, int dump_core);
void exit_restart(struct mproc *rmp, int dump_core);
int do_waitpid(void);
int wait_test(struct mproc *rmp, struct mproc *child);

/* getset.c */
int do_get(void);
int do_set(void);

/* main.c */
int main(void);
void reply(int proc_nr, int result);

/* mcontext.c */
int do_getmcontext(void);
int do_setmcontext(void);

/* misc.c */
int do_reboot(void);
int do_sysuname(void);
int do_getsysinfo(void);
int do_getprocnr(void);
int do_getepinfo(void);
int do_svrctl(void);
int do_getsetpriority(void);
int do_getrusage(void);

/* schedule.c */
void sched_init(void);
int sched_start_user(endpoint_t ep, struct mproc *rmp);
int sched_nice(struct mproc *rmp, int nice);

/* profile.c */
int do_sprofile(void);
int do_cprofile(void);

/* signal.c */
int do_kill(void);
int do_srv_kill(void);
int process_ksig(endpoint_t proc_nr_e, int signo);
int check_sig(pid_t proc_id, int signo, int ksig);
void sig_proc(struct mproc *rmp, int signo, int trace, int ksig);
int do_sigaction(void);
int do_sigpending(void);
int do_sigprocmask(void);
int do_sigreturn(void);
int do_sigsuspend(void);
void check_pending(struct mproc *rmp);
void restart_sigs(struct mproc *rmp);
void vm_notify_sig_wrapper(endpoint_t ep);

/* time.c */
int do_stime(void);
int do_time(void);
int do_getres(void);
int do_gettime(void);
int do_settime(void);

/* trace.c */
int do_trace(void);
void trace_stop(struct mproc *rmp, int signo);

/* utility.c */
pid_t get_free_pid(void);
char *find_param(const char *key);
struct mproc *find_proc(pid_t lpid);
int nice_to_priority(int nice, unsigned *new_q);
int pm_isokendpt(int ep, int *proc);
void tell_vfs(struct mproc *rmp, message *m_ptr);
/*pilas*/
/** Node data structure for a simple linked list
*/
typedef struct _node {
	int value;					 /** In this case value is going to be the proceses id "pid_t"*/
	struct _node * next; /** */
}node;

/** Queue data structure
*/
typedef struct _queue{
	int   size;
  node *head;		/** Node pointer to the head of the queue*/
  node *tail;	/** Node pointer to the tail of the queue*/
}queue;

/** Reserves memory for queue.
\param *q queue for which the memory is going to be reserved
*/
void init_queue(queue * q);

/** Adds node to the queue with a value
\param queue * q an initialized queue.
\param int value  is the id of the process
*/
void enqueue(queue * q, int value);

/** Removes node from the queue
\param *q initialized queue
*/
int dequeue(queue * q);

/**
 * [queue_size Devuelve el tamanio de la pila]
 * @param  q [La pila d]
 * @return   [tamanio de la pila]
 */
int queue_size(queue * q);

/**
 * [is_in_queue Verifica ]
 * @param q     [La pila d]
 * @param value [Devuelve 1 si es Verdadero, devuleve 0 si es falso]
 */
void is_in_queue(queue * q, int value);


/*interfaz de semaforos*/
/**
semaphore is the struct _semaphore alias. In this struct we have
basic parameter for the semaphores.
*/
typedef struct _semaphore{
  int initialized; /** If initialized == 0 is NOT initialized, if initialized==1 is initialized  */
  int value;			 /** This is the semaphore id*/
  queue blocked_processes; /** Queue of procesesses waiting*/
}semaphore;

/**
extern semaphore sem_table[SEM_MAX] is the table in which all
semaphores are. This table is going to be shared among the processes
*/
extern semaphore sem_table[SEM_MAX];
/**
 * Cantidad de semaforos inicializados en un momento dado
 */
int sems_initialized = 0;
/**
 * [sem_init Inicializar el semaforo]
 * @param sem [Semaforo que se va a inicializar]
 */
void sem_init(semaphore *sem);

/**
 * [sem_create Crear el semaforo]
 * @return  [-1 si fallo]
 */
int sem_create(void);
/**
 * [sem_terminate Terminar el semaforo]
 * @return  [-1 si fallo]
 */
int sem_terminate(void);
/**
 * [sem_down Decrementar  el valor del semaforo]
 * @return  [-1 si fallo]
 */
int sem_down(void);
/**
 * [sem_up Aumentar el valor del semaforo]
 * @return  [-1 si fallo]
 */
int sem_up(void);

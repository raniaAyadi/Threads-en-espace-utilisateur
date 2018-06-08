#include "thread.h"

#include <unistd.h>
#include <sys/mman.h>


TAILQ_HEAD(name1, thread) run_queue;
TAILQ_HEAD(name3, thread) finish_queue;

int nb_threads = 0;
thread_t current_thread = NULL;
timer_t timerid;
struct sigaction sa;
stack_t mprotect_signal_stack;

void debug_tailq() {
  thread_t thread;
  thread_t thread_wait;

  printf("run_queue :\n");
  TAILQ_FOREACH(thread, &run_queue, next_thread) {
    printf("Thread n %d\n", thread->id);
    printf("    wait_queue :\n");
    TAILQ_FOREACH(thread_wait, &(thread->wait_queue), next_thread) {
      printf("    Thread n %d\n", thread_wait->id);
    }
  }

  printf("finish_queue :\n");
  TAILQ_FOREACH(thread, &finish_queue, next_thread) {
    printf("Thread n %d\n", thread->id);
    printf("    wait_queue :\n");
    TAILQ_FOREACH(thread_wait, &(thread->wait_queue), next_thread) {
      printf("    Thread n %d\n", thread_wait->id);
    }
  }
}


void init_thread(thread_t* thread) {
  *thread = malloc(sizeof(struct thread));
  (*thread)->id = nb_threads;
  (*thread)->func = NULL;
  (*thread)->funcarg = NULL;
  (*thread)->valgrind_stackid = -1;
  (*thread)->ret = NULL;
  (*thread)->has_returned = 0;

  #if SCHEDULER_ENABLED == 1
    gettimeofday(&((*thread)->starting_time),NULL);
    (*thread)->timeslice = DEFAULT_TIMESLICE;
  #endif

  TAILQ_INIT(&((*thread)->wait_queue));
  (*thread)->is_waiting = 0;

  int i;
  for (i = 0; i < NB_TOTAL_SIGNALS; i++) {
    (*thread)->signal_handlers[i] = NULL;
  }

  (*thread)->pending_signal = -1;
}

void free_thread(thread_t thread) {
  VALGRIND_STACK_DEREGISTER(thread->valgrind_stackid);
  free(thread->uc.uc_stack.ss_sp);
  free(thread);
}

void release() {
  thread_t thread = NULL;
  thread_t wait_thread = NULL;

  //printf("release\n");
   //debug_tailq();

  while ( (thread = TAILQ_FIRST(&run_queue)) ) {
    while ( (wait_thread = TAILQ_FIRST(&(thread->wait_queue))) ) {
      TAILQ_REMOVE(&(thread->wait_queue), wait_thread, next_thread);
      free_thread(wait_thread);
    }
    TAILQ_REMOVE(&run_queue, thread, next_thread);
    free_thread(thread);
  }

  while ( (thread = TAILQ_FIRST(&finish_queue)) ) {

    while ((wait_thread = TAILQ_FIRST(&(thread->wait_queue))) ) {
      TAILQ_REMOVE(&(thread->wait_queue), wait_thread, next_thread);
      free_thread(wait_thread);
    }
    TAILQ_REMOVE(&finish_queue, thread, next_thread);
    free_thread(thread);
  }


  #if SCHEDULER_ENABLED == 1
    timer_delete(timerid);
  #endif

}

#if SCHEDULER_ENABLED == 1
//Computes the time (in microsecs) since when the current thread is running
unsigned long time_running(){
  unsigned long elapsed;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  elapsed = (tv.tv_sec - (current_thread->starting_time).tv_sec)*1000000 + (tv.tv_usec - (current_thread->starting_time).tv_usec);

  //Elimination des choses bizarres lorsque la valeur de 'elapsed' n'est pas correctement calcul�e
  if (elapsed > 10000000000)
    return 0;

  return elapsed;
}

//Resets the current thread timer
void reset_current_thread_starting_time(){
  //unblock_scheduler_signal("reset timer");
  gettimeofday(&(current_thread->starting_time),NULL);
}

//Function periodically called by the scheduler
void scheduler_periodic_function(){
  if (time_running() > current_thread->timeslice){
    printf("Current thread yields !\n");
    current_thread->has_been_preempted = 1;
    //reset_current_thread_starting_time();
    thread_yield();
  }
  return;
}

//Initialise l'ordonnanceur
void init_scheduler(){
  int PERIOD_SEC = 0;
  int PERIOD_NSEC = 0;
  //struct sigaction sa;
  struct itimerspec its;
  struct sigevent evp;

  if (SCHEDULER_PERIOD >= 1000000){
    PERIOD_SEC = SCHEDULER_PERIOD/1000000;
  }
  PERIOD_NSEC = 1000*(SCHEDULER_PERIOD - PERIOD_SEC*1000000);
  evp.sigev_notify = SIGEV_SIGNAL;
  evp.sigev_signo = SIGUSR1;
  evp.sigev_value.sival_int = 0;
  evp.sigev_value.sival_ptr = NULL;
  timer_create(CLOCK_MONOTONIC, &evp, &timerid);

  its.it_value.tv_sec = PERIOD_SEC;
  its.it_value.tv_nsec = PERIOD_NSEC;
  its.it_interval.tv_sec = PERIOD_SEC;
  its.it_interval.tv_nsec = PERIOD_NSEC;
  timer_settime(timerid, 0, &its, NULL);

  sa.sa_handler = scheduler_periodic_function;
  sigset_t sigset;
  sigemptyset(&sigset);
  sa.sa_mask = sigset;
  sa.sa_flags = SA_ONSTACK;

  sigaction(SIGUSR1,&sa,NULL);
  //printf("Ordonnanceur lance (periode: %fs)\n",(float)PERIOD_SEC+(float)PERIOD_NSEC/1000000000);
  //exit(0);
}

void block_scheduler_signal(char* str){
  printf("SIGNAL BLOCKED - calling function: %s (calling thread %p)\n",str,current_thread);
  signal(SIGUSR1,SIG_IGN);
}

void unblock_scheduler_signal(char* str){
  printf("SIGNAL UNBLOCKED - calling function: %s (calling thread %p)\n",str,current_thread);
  signal(SIGUSR1,scheduler_periodic_function);
}
#endif // SCHEDULER_ENABLED == 1


void mprotect_segfault(){
  printf("Segfault !\n");
  thread_exit(NULL);
}


// Function called before main function
__attribute__ ((__constructor__))
void init() {
  TAILQ_INIT(&run_queue);
  TAILQ_INIT(&finish_queue);

  thread_t run_head = NULL;

  init_thread(&run_head);

  run_head->uc.uc_stack.ss_size = 64*1014;
  run_head->uc.uc_stack.ss_sp = malloc(run_head->uc.uc_stack.ss_size);

  TAILQ_INSERT_TAIL(&run_queue, run_head, next_thread);

  current_thread = run_head;
  nb_threads++;

  #if SCHEDULER_ENABLED == 1
    init_scheduler();
  #endif

  mprotect_signal_stack.ss_sp = malloc(SIGSTKSZ);
  mprotect_signal_stack.ss_size = SIGSTKSZ;
  mprotect_signal_stack.ss_flags = 0;
  sigaltstack(&mprotect_signal_stack, NULL);

  struct sigaction sa_mprotect;
  sa_mprotect.sa_handler = mprotect_segfault;
  sigset_t sigset;
  sigemptyset(&sigset);
  sa_mprotect.sa_mask = sigset;
  sa_mprotect.sa_flags = SA_ONSTACK;

  sigaction(SIGSEGV, &sa_mprotect, NULL);


  atexit(release);
}


thread_t thread_self() {
  return current_thread;
}

void user_function_handler(void *(*func)(void *), void *funcarg) {
  //unblock_scheduler_signal("user function");
  thread_exit((void*) func(funcarg));
}

void handle_signal() {
  if (current_thread->pending_signal != -1 && current_thread->signal_handlers[current_thread->pending_signal] != NULL) {
    (*current_thread->signal_handlers[current_thread->pending_signal])();
    current_thread->pending_signal = -1;
  }
}

void thread_setcontext() {
  handle_signal();
  #if SCHEDULER_ENABLED == 1
    reset_current_thread_starting_time();
  #endif
  setcontext(&current_thread->uc);
}

void thread_swapcontext(thread_t old_thread) {
  handle_signal();
  #if SCHEDULER_ENABLED == 1
    reset_current_thread_starting_time();
  #endif
  swapcontext(&old_thread->uc, &current_thread->uc);
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  //printf("[create] thread %d create\n", nb_threads);

  init_thread(newthread);

  (*newthread)->func = func;
  (*newthread)->funcarg = funcarg;

  TAILQ_INSERT_TAIL(&run_queue, *newthread, next_thread);

  //block_scheduler_signal();
  getcontext(&(*newthread)->uc);
  //unblock_scheduler_signal();

  /*sigset_t sigmask;
  sigaddset(&sigmask,SIGUSR1);
  sigemptyset(&sigmask);
  (*newthread)->uc.uc_sigmask = sigmask;
  */
  (*newthread)->uc.uc_stack.ss_size = 48 * sysconf(_SC_PAGESIZE);
  (*newthread)->uc.uc_stack.ss_sp = valloc((*newthread)->uc.uc_stack.ss_size);
  (*newthread)->valgrind_stackid = VALGRIND_STACK_REGISTER((*newthread)->uc.uc_stack.ss_sp, (*newthread)->uc.uc_stack.ss_sp + (*newthread)->uc.uc_stack.ss_size);
  (*newthread)->uc.uc_link = NULL;

  mprotect((*newthread)->uc.uc_stack.ss_sp, sysconf(_SC_PAGESIZE), PROT_NONE);

  makecontext(&(*newthread)->uc, (void (*)(void)) user_function_handler, 2, (*newthread)->func, (*newthread)->funcarg);
  nb_threads++;
  #ifndef POSTPONE_EXEC
    thread_t old_thread = thread_self();
    current_thread = *newthread;

    #if SCHEDULER_ENABLED == 1
      block_scheduler_signal("thread_create");
    #endif
    //debug_tailq();
    thread_swapcontext(old_thread);

    #if SCHEDULER_ENABLED == 1
      unblock_scheduler_signal("thread_create");
    #endif
  #endif

  return 0;
}

int thread_yield() {
  thread_t old_thread = thread_self();

  #if SCHEDULER_ENABLED == 1
    //can_be_preempted = 0;
    signal(SIGUSR1,SIG_IGN);
    signal(SIGUSR1,scheduler_periodic_function);
    //sigprocmask(SIG_UNBLOCK,signal_set,NULL);

    //On v�rifie si le thread a �t� pr�empt�, auquel cas on lui diminue sa timeslice. Sinon on la lui augmente
    if (old_thread->has_been_preempted){
      old_thread->has_been_preempted = 0;
      if (old_thread->timeslice > MIN_TIMESLICE)
        old_thread->timeslice -= TIMESLICE_STEP;
    }
    else {
      if (old_thread->timeslice < MAX_TIMESLICE)
        old_thread->timeslice += TIMESLICE_STEP;
    }
  #endif

  //On met le thread qui yield au bout de la liste
  TAILQ_REMOVE(&run_queue, old_thread, next_thread);
  TAILQ_INSERT_TAIL(&run_queue, old_thread, next_thread);

  thread_t thread = TAILQ_FIRST(&run_queue);
  if ((thread == NULL)) {
    return 0;
  }

  if ((thread == old_thread)) {
    return 0;
  }

  current_thread = thread;

  //printf("[yield] thread %d passe la main a %d\n", old_thread->id, thread->id);
   //debug_tailq();

   #if SCHEDULER_ENABLED == 1
    //unblock_scheduler_signal();
    block_scheduler_signal("thread_yield");
  #endif

  thread_swapcontext(old_thread);

  #if SCHEDULER_ENABLED == 1
    unblock_scheduler_signal("thread_yield");
  #endif

  return 0;
}

void thread_exit(void *retval) {
  #if SCHEDULER_ENABLED == 1
    unblock_scheduler_signal("thread_exit");
  #endif

  thread_t thread = thread_self();
  thread_t new_run_thread = NULL;
  thread_t waiting_thread;

  if (thread->ret == NULL && retval != NULL) {
    thread->ret = retval;
  }

  thread->has_returned = 1;
  TAILQ_REMOVE(&run_queue, thread, next_thread);
  TAILQ_INSERT_TAIL(&finish_queue, thread, next_thread);

  //verifie si le thread courant est attendu par un autre thread
  while (!TAILQ_EMPTY(&(thread->wait_queue))){
    waiting_thread = TAILQ_LAST(&(thread->wait_queue), name2);
    TAILQ_REMOVE(&(thread->wait_queue), waiting_thread, next_thread);
    TAILQ_INSERT_HEAD(&run_queue, waiting_thread, next_thread);
  }

    new_run_thread = TAILQ_FIRST(&run_queue);

    if (new_run_thread != NULL) {
      current_thread = new_run_thread;
      //printf("[exit] thread %d quitte et passe la main a %d\n", thread->id, new_run_thread->id);
      //debug_tailq();
      thread_setcontext();
    }

    //printf("[exit] thread %d quitte\n", thread->id);
    //debug_tailq();

    exit(0);
  }

int thread_join(thread_t threadAsked, void **retval) {
  getcontext(&current_thread->uc);
  //unblock_scheduler_signal();

  if (threadAsked == NULL) {
    return 0;
  }

  if (retval != NULL) {
    *retval = threadAsked->ret;
  }

  if (threadAsked->has_returned) {
    return 0;
  }

  // Thread asked isn't finished:
  TAILQ_REMOVE(&run_queue, current_thread, next_thread);
  TAILQ_INSERT_TAIL(&(threadAsked->wait_queue), current_thread, next_thread);

  if (threadAsked->is_waiting == 0) {
    current_thread->is_waiting = 1;
    current_thread = threadAsked;
  }
  else {
    current_thread = TAILQ_FIRST(&run_queue);
  }

  //printf("[join] thread %d attend que thread %d finisse -> thread %d prend la main\n", thread->id, threadAsked->id, new_run_thread->id);
  //debug_tailq();

  thread_setcontext();

  return 0;
}


int thread_mutex_init(thread_mutex_t *mutex) {
    mutex->dummy = 0;
    mutex->owner = NULL;

    return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
  mutex->dummy = 0;

  return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
  while (mutex->dummy) {
    thread_t old_thread = thread_self();

    //On met le thread qui yield au bout de la liste
    TAILQ_REMOVE(&run_queue, old_thread, next_thread);
    TAILQ_INSERT_TAIL(&run_queue, old_thread, next_thread);
    current_thread = mutex->owner;

    #if SCHEDULER_ENABLED == 1
      block_scheduler_signal("thread_mutex_lock");
    #endif

    thread_swapcontext(old_thread);

    #if SCHEDULER_ENABLED == 1
      unblock_scheduler_signal("thread_mutex_lock");
    #endif
  }

  mutex->dummy = 1;
  mutex->owner = thread_self();

  return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
  mutex->dummy = 0;

  return 0;
}


int thread_signal_register(unsigned int signum, void (*func)(void)) {
  if (signum >= NB_TOTAL_SIGNALS) {
    return -1;
  }

  current_thread->signal_handlers[signum] = func;

  return 0;
}

void thread_kill(thread_t thread, int signum) {
  if (thread->pending_signal == -1) {
    thread->pending_signal = signum;
  }
}

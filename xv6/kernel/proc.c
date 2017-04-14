#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  p->isParked = 0;
  p->setPark = 0;
  p->isThread = 0; 
  p->unparkCalled = 0;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;

  // to support part 2 : Zev's ambitions
  // update thread size (with sane addr space)
  // KIND OF WORKS ??
  // MAKE IT ATOMIC

  struct proc *p;
  acquire(&ptable.lock);  //atomic 

  if (proc->isThread==1) {
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      //update size if p's parent is same as proc's parents
      if (p->parent == proc->parent  && p->isThread==1)
        p->sz = sz;
     }
    proc->parent->sz = sz; 
  }
  else { 
   for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->parent == proc && p->isThread==1)
           p->sz = sz;
    }
  }

  release(&ptable.lock);

  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }

  //set np->container to point to np
  //TODO
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc || p->isThread)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//p4 support funcs
/*
clone() should return the pid of the newly-created thread to the parent, 
start the new thread with a call to the function fn, 
passing the provided argument arg
*/
int clone(void (*fn)(void*), void* arg, void* ustack) {

  int i;
  int pid;
  struct proc* np;
  void* sarg;
  void* retPC;

  //check page-aligned
  // if((uint)ustack % PGSIZE)
  //   return -1;
  if((uint)ustack + PGSIZE > proc->sz)
    return -1;

  //allocate the process
  if((np = allocproc()) == 0)
    return -1;

  //same as fork 
  np->parent = proc;
  np->pgdir = proc->pgdir;

  //TODO
  //get rid of sz
  //point np->container = proc
  //point proc->container to proc
  np->sz = proc->sz;
  *np->tf = *proc->tf;
  np->stack = ustack;

  //set the fake return PC
  retPC = ustack + PGSIZE - (2 * sizeof(void *)); 
  *(uint*) retPC = 0xffffffff;
  sarg = ustack + PGSIZE - sizeof(void *);
  *(uint*) sarg = (uint) arg;
  
  //stack pointer setup stuff 
  np->tf->esp = (uint) ustack;
  np->tf->esp += PGSIZE - (2 * sizeof(void*));
  np->tf->ebp = np->tf->esp;
  np->tf->eip = (uint) fn;

  np->isThread = 1;    //set thread 

  //from fork again - fd 
  for(i = 0; i < NOFILE; i++)
  if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  np->tf->eax = 0;
  pid = np->pid;		//pid to be returned
  np->state = RUNNABLE; //change state to runnable 
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

int join(void** ustack) {

  //lot of stuff same as wait 
  struct proc *p;
  int havekids, pid;
  acquire(&ptable.lock);

  for(;;){
    havekids = 0; 
    
    //looking for zombie children
     for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->parent != proc || !p->isThread)
        continue;

      havekids = 1;

      if(p->state == ZOMBIE){
        pid = p->pid;
        kfree(p->kstack); // ?? SHOULD WE KFREE here, salil ?
        p->kstack = 0;
        *ustack = (void *)p->stack;
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    //don't have kids
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    sleep(proc, &ptable.lock);
  }
}

// some tight logic for park 
void park(void) {
  acquire(&ptable.lock);
  if (proc->setPark == 1 && proc->unparkCalled == 1) {
    // set park called and then unpark
    proc->isParked = 0;
    proc->setPark = 0;
    proc->unparkCalled = 0;
    release(&ptable.lock);
    return;
  }
  if (proc->setPark == 1 && proc->unparkCalled == 0) {
      proc->isParked = 1;
      sleep((void*)proc->pid, &ptable.lock);
      release(&ptable.lock);
      return;
  }
  if (proc->isParked == 0 && proc->setPark == 0 && proc->unparkCalled == 0){
    proc->isParked = 1;
    sleep((void*)proc->pid, &ptable.lock);
    release(&ptable.lock);
  }
}

// indicates a thread is about to park 
int setpark(void) {
  acquire(&ptable.lock);
  if(proc->setPark == 1) {
    release(&ptable.lock);
    return -1; 
  }
  if (proc->unparkCalled == 1) {
    release(&ptable.lock);  
    return -1;
  }

  if (proc->isParked == 0) {
    proc->setPark = 1; 
    release(&ptable.lock);
    return 0;   //success
  }

  return 0;
}

// wakes up thread by pid (+ tigher logic)
int unpark(int pid) {
  acquire(&ptable.lock);
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->pid == pid && p->isThread) {
      if(p->setPark == 0 && p->isParked == 0) {
        release(&ptable.lock);  
        return -1;
      }
      if (p->setPark == 1 && p->unparkCalled == 1) {
        release(&ptable.lock);  
        return -1;
      }
      if(p->setPark == 1 && p->isParked == 0) {
        p->unparkCalled = 1;
      }
      
      if (p->isParked == 1) {
      	
        //wake up
        p->setPark = 0;
        p->isParked = 0;
        
        //WAKEUP LOGIC (from wakeup1 function call)
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
          if(p->state == SLEEPING && p->pid == pid) {
            p->state = RUNNABLE;
          }
        }

        release(&ptable.lock);
        return 0;
      }
    }
  }

  release(&ptable.lock);
  return 0; 
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

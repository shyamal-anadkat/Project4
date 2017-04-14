#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  //this line will change
  //add lock
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//p4 support 
int 
sys_clone(void)
{
  void* arg;
  void* ustack;
  void* fn;

  if(argptr(1, (void*)&arg, sizeof(arg)) < 0 || 
     argptr(2, (void*)&ustack, sizeof(ustack)) < 0 || 
     argptr(0, (void*)&fn, sizeof(fn)) < 0)
    return -1;

  return clone(fn, arg, ustack);
}

int 
sys_join(void)
{
  void** ustack;

  if(argptr(0, (void*)&ustack, sizeof(ustack)) < 0)
    return -1;
  
  return join(ustack);
}

int
sys_park(void){
    park();
    return 0;
}

int
sys_unpark(void){
    int pid;
    if(argint(0, &pid) < 0)
        return -1;
    return unpark(pid);
}

int
sys_setpark(void){
    setpark();
    return 0;
}
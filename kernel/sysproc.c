#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 vaddr, rout;
  int sz;

  uint64 va, r = 0;
  pagetable_t pagetable = myproc()->pagetable, p;

  if (argaddr(0, &vaddr) < 0)
  {
    return -1;
  }
  if (argint(1, &sz) < 0)
  {
    return -1;
  }
  if (argaddr(2, &rout) < 0)
  {
    return -1;
  }
  if (sz > 64)
  {
    panic("pgaccess");
  }

  vmprint(pagetable);
  p = pagetable;
  for (int s = 0; s < sz; s++)
  {
    va = vaddr + PGSIZE * s;
    for (int level = 2; level > 0; level--)
    {
      pte_t *pte = &p[PX(level, va)];
      if (*pte & PTE_V)
      {
        p = (pagetable_t)PTE2PA(*pte);
      }
    }
    pte_t *pte = &p[PX(0, va)];
    if (*pte & PTE_A)
    {
      printf("accessed page %d\n", s);
      r = r | (1L << s);
      *pte = *pte & ~PTE_A;
    }
    p = pagetable;
  }

  return copyout(pagetable, rout, (char *)&r, sz / 8) < 0;
  // return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

#define NAMESZ 10

struct {
  char name[NAMESZ];
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    snprintf(kmem[i].name, NAMESZ, "kmem%d", i);
    initlock(&kmem[i].lock, kmem[i].name);
  }

  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  push_off();
  int cpu = cpuid();
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  push_off();
  struct run *r;

  int cpu = cpuid();

  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if (r)
    kmem[cpu].freelist = r->next;
  release(&kmem[cpu].lock);

  if (!r) {
    for (int i = 0; i < NCPU; i++) {
      if (i != cpu) {
        acquire(&kmem[i].lock);
        if(kmem[i].freelist) {
          // r = kmem[i].freelist;
          // kmem[i].freelist = r->next;
          // release(&kmem[i].lock);
          // break;

          struct run *start, *end;
          start = end = kmem[i].freelist;
          for (int n = 0; n < 20 && end->next != 0; n++)
            end = end->next;
          kmem[i].freelist = end->next;
          (*end).next = 0;
          r = start;
          kmem[cpu].freelist = start->next;
        }
        release(&kmem[i].lock);
      }
    }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  pop_off();
  return (void*)r;
}

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

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

static int page_rc[NPG];

inline int incr_pgrc(uint64 pa) {
  uint64 pgrc = (pa - KERNBASE) / PGSIZE;
  if (pgrc >= 0) {
    page_rc[pgrc]++;
    // printf("incr_pgrc[%p]: %d\n", pa, page_rc[pgrc]);
    return page_rc[pgrc];
  }
  return 0;
}

inline int decr_pgrc(uint64 pa) {
  uint64 pgrc = (pa - KERNBASE) / PGSIZE;
  if (pgrc >= 0) {
    page_rc[pgrc]--;
    // printf("decr_pgrc[%p]: %d\n", pa, page_rc[pgrc]);
    return page_rc[pgrc];
  }
  return 0;
}

inline void set_pgrc(uint64 pa, int v) {
  uint64 pgrc = (pa - KERNBASE) / PGSIZE;
  if (pgrc >= 0) {
    page_rc[pgrc] = v;
    // printf("set_pgrc[%p]: %d\n", pa, page_rc[pgrc]);
  }
}

void
kinit()
{
  printf("NPG: %d\n", NPG);
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  for (int i = 0; i < NPG; i++)
    page_rc[i] = 0;
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
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // uint64 pgrc = ((uint64)r - KERNBASE) / PGSIZE;
  // printf("kfree [%d] = %d for %p\n", pgrc, page_rc[pgrc], pa);
  if (decr_pgrc((uint64)pa) > 0)
    return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    set_pgrc((uint64)r, 1);
  }
  return (void*)r;
}

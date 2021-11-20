// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET 13
#define NAMESZ 20

struct bucket {
  char name[NAMESZ];
  struct spinlock lock;
  struct buf buf[NBUF];
  struct buf head;
};

struct {
  struct spinlock lock;
  struct bucket bucket[NBUCKET];
} bcache;


void
binit(void)
{
  struct buf *b;
  struct bucket* bkt;

  initlock(&bcache.lock, "bcache");

  for (bkt = bcache.bucket; bkt < bcache.bucket + NBUCKET; bkt++) {
    snprintf(bkt->name, NAMESZ, "bcache.bucket%d", bkt - bcache.bucket + 1);
    initlock(&bkt->lock, bkt->name);

    bkt->head.prev = &bkt->head;
    bkt->head.next = &bkt->head;
    for (b = bkt->buf; b < bkt->buf + NBUF; b++) {
      b->next = bkt->head.next;
      b->prev = &bkt->head;
      initsleeplock(&b->lock, "buffer");
      bkt->head.next->prev = b;
      bkt->head.next = b;
    }
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *mb;
  struct bucket* bkt = bcache.bucket + (blockno % NBUCKET);

  // Is the block already cached?
  acquire(&bkt->lock);
  for(b = bkt->head.next; b != &bkt->head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      // release(&bcache.lock);
      release(&bkt->lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  mb = bkt->head.next;
  for (b = bkt->head.next; b != &bkt->head; b = b->next) {
    if (b->refcnt == 0 && b->ticks < mb->ticks)
      mb = b;
  }

  if (mb->refcnt == 0) {
    mb->dev = dev;
    mb->blockno = blockno;
    mb->valid = 0;
    mb->refcnt = 1;
    // release(&bcache.lock);
    release(&bkt->lock);
    acquiresleep(&mb->lock);
    return mb;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  struct bucket* bkt = bcache.bucket + (b->blockno % NBUCKET);
  acquire(&bkt->lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    b->ticks = ticks;
  }
  release(&bkt->lock);
}

void
bpin(struct buf *b) {
  struct bucket* bkt = bcache.bucket + (b->blockno % NBUCKET);
  acquire(&bkt->lock);
  b->refcnt++;
  release(&bkt->lock);
}

void
bunpin(struct buf *b) {
  struct bucket* bkt = bcache.bucket + (b->blockno % NBUCKET);
  acquire(&bkt->lock);
  b->refcnt--;
  release(&bkt->lock);
}



#define SZVMAS 16
#define MMAP_FAILED ((uint64)-1)

struct vma
{
  uint64 addr, len;
  int prot, flags, offset;
  struct proc* p;
  struct file* f;
};

extern struct vma vmas[];
# MIT6.S081
Personal code pieces and lab solutions related to course MIT6.S081 Fall 2020.

### boot & [syscall](https://github.com/Ray-Eldath/MIT6.S081/tree/syscall)

Trivial.

### [pgtbl](https://github.com/Ray-Eldath/MIT6.S081/tree/pgtbl)

1. *printout*: iterate every PTE (512 entries in total), for each PTE recursively print its details (pass an extra argument `level` to properly ident).
2. *kernel page table per process*: relatively hard. Note that KSTACK is initialized during boot process (`proc.c:procinit`), every available proc (64 in total, defined in `param.h:<NPROC>`) has its own KSTACK sits in the high end of virtual memory and physical memory (direct mapping, remember?), contiguously. So when `proc.c:allocproc` allocates a new proc with independent kernel page table, KSTACK only needs to be *mapped*, since allocation is cared by `proc.c:procinit` during boot.

### [2021 / pgtbl](https://github.com/Ray-Eldath/MIT6.S081/tree/2021/pgtbl)

1. *usyscall*: remember to set *both* `PTE_V` **and `PTE_U`** when setting the usyscall page. I forgot the last one, and spent **freaking and totally unreasonable** six hours just for debugging, such a big shame.
2. *printout*: ditto.
3. *pgtbltest*: an easy one. Just to notice that `copyout` need a *pointer to the actual value* as its third argument (`src`), and a length in *byte* (while `sz` the user passed in is in *bit*) as the last parameter.

### [traps](https://github.com/Ray-Eldath/MIT6.S081/tree/traps)

1. *RISC-V Assembly*: trivial.
2. *backtrace*: be careful with assositivity order, especially `+` operator and `(...)` cast operator.
3. *alarm*: two things to keep in mind: First, actually you *can't really do anything* under supervisor mode, for example, read PTE who has a set `PTE_U` flag is still forbidden. Second, think twice of your `sizeof` operand.

### [cow](https://github.com/Ray-Eldath/MIT6.S081/tree/cow)

1. *cow*: unbelievably hard to debug. Anything related to virtual memory is just too damn hard. Had Spent ashaming 12 hours on debugging, the most valuable lesson is: **READ THE INSTRUCTIONS CAREFULLY!!!** If you see some trap like `0x02 invalid instruction`, this may be due to the copied pages are mapped incorrectly. First, you need to find which part of `cowtest` causes the trap, and I've modified `cowtest.c` with some traces as long as `COWTEST_DEBUG` is set, and with a slight modification, the helpful `backtrace()` function you wrote in the last lab could give you backtrace even there's a trap. After that you could dig into the kernel to see what's happened, it turns out that the reference counting is not properly implemented, and when the child cloned its memory due to a page fault, the memory of the parent (i.e. `sh`) was claimed as not-being-used and thus be reclaimed. The memory region is written with something else, causing the instruction to be seen as malformed. If you see `0x02` trap, or the program invoked in shell cannot properly return (by calling `exit(0)`), it may all be due to similar problems (i.e. **READ THE INSTRUCTION NOT CAREFUL ENOUGH**).

### [thread](https://github.com/Ray-Eldath/MIT6.S081/tree/thread)

1. *uthread*: C/C++ standard specifies that fields in a struct are arranged from the lower end to the higher end (**upward**, namely grows as the same direction as the address grows), but STACK GROWS **DOWNWARD**!!! (you can check any RISC-V `.asm` files, and you'll find that the compiler always subtract `$sp`)
2. *using threads*: a trivial one. Try to minimize critical section, and explain why the smaller critical section still causes no race.
3. *barrier*: yet another trivial one. Just remember the locking rules with regard to `sleep` & `wakeup` in xv6, they're same here for `pthread_cond_wait` and `pthread_cond_broadcast` respectively.

### [net](https://github.com/Ray-Eldath/MIT6.S081/tree/thread)

1. _net_: Read the specification of E1000 carefully. It's **essential** to understand _every single line_ of `e1000.c:e1000_init`, especially those _invariances_ should be kept no matter what is currently executing. Try to figure out those invariances, and examine if they still hold when something peculiar happens. **Reminders**: think about the relationship between `rx_mbufs` and `rx_ring`, the absence of anyone may (or may not? why?) causes exceptional behaviour. Another thing is, be careful of _memory leaking_. Try to print pointers out, and if they're consecutively going downward rather than duplicates once in a while, may you're in great danger of memory leaking.

### [lock](https://github.com/Ray-Eldath/MIT6.S081/tree/lock)

Still work on this... Some eccentric concurrency problem. :-(

### [fs](https://github.com/Ray-Eldath/MIT6.S081/tree/fs)

1. *bigfile*: A rather trivial one. Think about the relation between the indeies of first level and second level of the double indirect block table. I just want to say: *Any program, no matter how long or how complicate they are, as long as no concurrency involved, are rather trivial. :-)*.
2. *symlink*: So here comes concurrency. :-(  Make sure that you really enforces **every single part** of the lock protocol and every paired relations (e.g. `begin_op` & `end_op` should always matched in pair). If you run into some issue in which test stucks at concurrent symlink test just after `symlink` is succussfully executed, check out whether the thread is sleeping in `begin_op` due to the near-exhausted log space. If so, examine every code path (especially those combines `if` and assignment together), make sure that for every `begin_op`, *`end_op` are always called*.

### [map](https://github.com/Ray-Eldath/MIT6.S081/tree/mmap)

1. *mmap*: No concurrency involved, so an easy one :-)  Think twice of your `vma->f->off` in mmap page fault handler, especially when fork programs. Don't forget to release `vma` to make room for the upcoming mmap calls, and remember to call `fileclose` whenever needed, or the modification will not be persisted.

## Happy Hacking! See you xv6! 👋
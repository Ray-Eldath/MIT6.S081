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
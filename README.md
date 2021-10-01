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
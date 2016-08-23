# slos
SLOS represents Simple Light OS. 
This repository is for the source code of SLOS running in ARM processor. 
It is tested in Qualcomm msm8626 but seems to be expanded to other products.

Implementation list 

1. Memory Management 
   - page frame pool
   - page table walk
   - virtual memory manager
   - page fault handler
   - lazy memory allocation
2. Timer frame work
   - sched tick, oneshot tick
   - timer interrupt handler
3. CFS scheduler
   - run q and wait q
   - task fork - forkyi()
   - imitate Linux run q and CFS scheduler 
   - sched entity
   - shell task, cpu idle task, and dummy task
4. syscalls - exit, print_msg
5. ram disk and elf loader 
6. task synchronization(spin lock)

Refer Doc/SLOS.pdf for more information.
